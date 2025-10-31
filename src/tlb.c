/**
 * @file tlb.c
 * @brief TLB implementation
 * @author Amir Noohi
 * @copyright Copyright (c) 2025 Amir Noohi. All rights reserved.
 * 
 * Similar structure to cache.c but for page translations.
 * Provided as reference implementation (can be given to students or
 * made into skeleton based on course requirements).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tlb.h"
#include "types.h"

/* External function from cache.c */
extern uint32_t log2_uint32(uint32_t n);

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * @brief Initialize TLB entry list for LRU
 */
static void init_tlb_lru(tlb_set_t *set) {
    if (set->num_ways <= 1) return;
    
    set->lru_head = &set->entries[0];
    set->lru_tail = &set->entries[set->num_ways - 1];
    
    for (uint32_t i = 0; i < set->num_ways; i++) {
        set->entries[i].valid = false;
        set->entries[i].dirty = false;
        set->entries[i].vpn = 0;
        set->entries[i].ppn = 0;
        
        if (i > 0) {
            set->entries[i].prev = &set->entries[i - 1];
        }
        if (i < set->num_ways - 1) {
            set->entries[i].next = &set->entries[i + 1];
        }
    }
}

/**
 * @brief Move TLB entry to head of LRU list
 */
static void tlb_lru_move_to_head(tlb_set_t *set, tlb_entry_t *entry) {
    if (!entry || set->num_ways <= 1 || entry == set->lru_head) {
        return;
    }
    
    /* Remove from current position */
    if (entry->prev) {
        entry->prev->next = entry->next;
    }
    if (entry->next) {
        entry->next->prev = entry->prev;
    }
    
    /* Update tail if needed */
    if (entry == set->lru_tail) {
        set->lru_tail = entry->prev;
    }
    
    /* Insert at head */
    entry->prev = NULL;
    entry->next = set->lru_head;
    if (set->lru_head) {
        set->lru_head->prev = entry;
    }
    set->lru_head = entry;
}

/**
 * @brief Find TLB entry with matching VPN
 */
static tlb_entry_t* find_tlb_entry(tlb_set_t *set, uint32_t vpn) {
    for (uint32_t i = 0; i < set->num_ways; i++) {
        if (set->entries[i].valid && set->entries[i].vpn == vpn) {
            return &set->entries[i];
        }
    }
    return NULL;
}

/**
 * @brief Find invalid TLB entry
 */
static tlb_entry_t* find_invalid_tlb_entry(tlb_set_t *set) {
    for (uint32_t i = 0; i < set->num_ways; i++) {
        if (!set->entries[i].valid) {
            return &set->entries[i];
        }
    }
    return NULL;
}

/**
 * @brief Select victim TLB entry for eviction
 */
static tlb_entry_t* select_tlb_victim(tlb_set_t *set) {
    /* Try invalid entry first */
    tlb_entry_t *invalid = find_invalid_tlb_entry(set);
    if (invalid) {
        return invalid;
    }
    
    /* Use LRU */
    if (set->num_ways == 1) {
        return &set->entries[0];
    }
    
    return set->lru_tail;
}

/**
 * @brief Get set index for VPN
 */
static uint32_t get_tlb_index(const tlb_t *tlb, uint32_t vpn) {
    if (tlb->associativity == FULLY_ASSOC) {
        return 0;
    }
    
    uint32_t index_mask = (1 << tlb->index_bits) - 1;
    return vpn & index_mask;
}

/**
 * @brief Get tag for VPN
 */
static uint32_t get_tlb_tag(const tlb_t *tlb, uint32_t vpn) {
    return vpn >> tlb->index_bits;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

tlb_t* tlb_init(tlb_config_t config) {
    tlb_t *tlb = calloc(1, sizeof(tlb_t));
    if (!tlb) {
        return NULL;
    }
    
    tlb->num_entries = config.num_entries;
    tlb->associativity = config.associativity;
    
    /* Calculate TLB geometry */
    switch (config.associativity) {
        case DIRECT_MAPPED:
            tlb->num_sets = config.num_entries;
            tlb->ways_per_set = 1;
            break;
            
        case FULLY_ASSOC:
            tlb->num_sets = 1;
            tlb->ways_per_set = config.num_entries;
            break;
            
        case TWO_WAY:
            tlb->ways_per_set = 2;
            tlb->num_sets = config.num_entries / 2;
            break;
            
        case FOUR_WAY:
            tlb->ways_per_set = 4;
            tlb->num_sets = config.num_entries / 4;
            break;
            
        default:
            free(tlb);
            return NULL;
    }
    
    /* Calculate bit fields */
    tlb->offset_bits = 12;  /* 4KB pages */
    tlb->index_bits = (config.associativity == FULLY_ASSOC) ? 
                      0 : log2_uint32(tlb->num_sets);
    tlb->tag_bits = 20 - tlb->index_bits;  /* VPN is 20 bits (bits 31-12) */
    
    /* Allocate sets */
    tlb->sets = calloc(tlb->num_sets, sizeof(tlb_set_t));
    if (!tlb->sets) {
        free(tlb);
        return NULL;
    }
    
    /* Initialize each set */
    for (uint32_t i = 0; i < tlb->num_sets; i++) {
        tlb->sets[i].num_ways = tlb->ways_per_set;
        tlb->sets[i].entries = calloc(tlb->ways_per_set, sizeof(tlb_entry_t));
        
        if (!tlb->sets[i].entries) {
            for (uint32_t j = 0; j < i; j++) {
                free(tlb->sets[j].entries);
            }
            free(tlb->sets);
            free(tlb);
            return NULL;
        }
        
        init_tlb_lru(&tlb->sets[i]);
    }
    
    tlb->accesses = 0;
    tlb->hits = 0;
    tlb->misses = 0;
    
    return tlb;
}

tlb_result_t tlb_lookup(tlb_t *tlb, uint32_t vpn, uint32_t *ppn, bool *dirty) {
    tlb->accesses++;
    
    uint32_t index = get_tlb_index(tlb, vpn);
    uint32_t tag = get_tlb_tag(tlb, vpn);
    
    tlb_set_t *set = &tlb->sets[index];
    tlb_entry_t *entry = find_tlb_entry(set, tag);
    
    if (entry) {
        /* TLB HIT */
        tlb->hits++;
        *ppn = entry->ppn;
        *dirty = entry->dirty;
        
        /* Update LRU */
        tlb_lru_move_to_head(set, entry);
        
        return TLB_HIT;
    }
    
    /* TLB MISS */
    tlb->misses++;
    return TLB_MISS;
}

void tlb_insert(tlb_t *tlb, uint32_t vpn, uint32_t ppn) {
    uint32_t index = get_tlb_index(tlb, vpn);
    uint32_t tag = get_tlb_tag(tlb, vpn);
    
    tlb_set_t *set = &tlb->sets[index];
    
    /* Check if entry already exists */
    tlb_entry_t *entry = find_tlb_entry(set, tag);
    if (entry) {
        /* Update existing entry */
        entry->ppn = ppn;
        tlb_lru_move_to_head(set, entry);
        return;
    }
    
    /* Select victim */
    tlb_entry_t *victim = select_tlb_victim(set);
    
    /* Install new entry */
    victim->valid = true;
    victim->dirty = false;
    victim->vpn = tag;
    victim->ppn = ppn;
    
    /* Move to head */
    tlb_lru_move_to_head(set, victim);
}

void tlb_set_dirty(tlb_t *tlb, uint32_t vpn) {
    uint32_t index = get_tlb_index(tlb, vpn);
    uint32_t tag = get_tlb_tag(tlb, vpn);
    
    tlb_set_t *set = &tlb->sets[index];
    tlb_entry_t *entry = find_tlb_entry(set, tag);
    
    if (entry) {
        entry->dirty = true;
    }
}

void tlb_print_stats(const tlb_t *tlb) {
    printf("\n* TLB Statistics *\n");
    printf("total accesses: %llu\n", (unsigned long long)tlb->accesses);
    printf("hits: %llu\n", (unsigned long long)tlb->hits);
    printf("misses: %llu\n", (unsigned long long)tlb->misses);
}

void tlb_print_entries(const tlb_t *tlb) {
    printf("\nTLB Entries (Valid-Bit Dirty-Bit VPN PPN)\n");
    
    for (uint32_t i = 0; i < tlb->num_sets; i++) {
        for (uint32_t j = 0; j < tlb->sets[i].num_ways; j++) {
            tlb_entry_t *entry = &tlb->sets[i].entries[j];
            
            if (entry->valid) {
                /* Reconstruct full VPN */
                uint32_t full_vpn = (entry->vpn << tlb->index_bits) | i;
                printf("%d %d 0x%05x 0x%05x\n", 
                       1, entry->dirty ? 1 : 0, full_vpn, entry->ppn);
            } else {
                printf("%d %d - -\n", 0, 0);
            }
        }
    }
}

void tlb_destroy(tlb_t *tlb) {
    if (!tlb) return;
    
    for (uint32_t i = 0; i < tlb->num_sets; i++) {
        free(tlb->sets[i].entries);
    }
    
    free(tlb->sets);
    free(tlb);
}

