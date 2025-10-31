/**
 * @file pagetable.c
 * @brief Page table implementation
 * @author Amir Noohi
 * @copyright Copyright (c) 2025 Amir Noohi. All rights reserved.
 * 
 * Linear page table with LRU page replacement.
 * Students implement this module for the assignment.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pagetable.h"
#include "ll.h"
#include "types.h"

/* ============================================================================
 * Global State
 * ============================================================================ */

/* Page table array */
static pte_t page_table[PAGE_TABLE_ENTRIES];

/* Free page list */
static page_t *free_page_list = NULL;

/* Used page list (for LRU tracking) */
static page_t *used_page_list = NULL;

/* Hash table for quick frame lookup */
static page_t *frame_table[NUM_PHYSICAL_PAGES];

/* Statistics */
static uint64_t pt_accesses = 0;
static uint64_t page_faults = 0;
static uint64_t page_faults_dirty = 0;

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * @brief Create a physical page frame
 */
static page_t* create_page(uint32_t frame_id) {
    page_t *page = calloc(1, sizeof(page_t));
    if (!page) return NULL;
    
    page->frame_id = frame_id;
    page->pte = NULL;
    page->next = NULL;
    page->prev = NULL;
    memset(page->data, 0, PAGE_SIZE);
    
    return page;
}

/**
 * @brief Get a free page from the free list
 */
static page_t* get_free_page(void) {
    if (!free_page_list) {
        return NULL;  /* No free pages */
    }
    
    /* Remove from free list */
    page_t *page = ll_remove_head(&free_page_list);
    return page;
}

/**
 * @brief Get LRU victim page for eviction
 */
static page_t* get_victim_page(void) {
    /* Get tail of used list (LRU page) */
    page_t *victim = ll_get_tail(used_page_list);
    
    if (victim) {
        /* Remove from used list */
        ll_remove_page(&used_page_list, victim);
        
        /* Mark PTE as not present */
        if (victim->pte) {
            victim->pte->present = false;
        }
    }
    
    return victim;
}

/* ============================================================================
 * Dummy I/O Functions (PROVIDED)
 * ============================================================================ */

void read_page_from_disk(uint8_t *page_data, uint32_t disk_block) {
    /* Simulated disk read - no actual I/O */
    (void)page_data;
    (void)disk_block;
}

void write_page_to_disk(const uint8_t *page_data) {
    /* Simulated disk write - no actual I/O */
    (void)page_data;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

void pagetable_init(void) {
    /* Initialize page table entries */
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        page_table[i].present = false;
        page_table[i].dirty = false;
        page_table[i].ppn = 0;
    }
    
    /* Initialize frame table */
    for (uint32_t i = 0; i < NUM_PHYSICAL_PAGES; i++) {
        frame_table[i] = NULL;
    }
    
    /* Create free page list (all physical pages initially free) */
    for (int i = NUM_PHYSICAL_PAGES - 1; i >= 0; i--) {
        page_t *page = create_page(i);
        if (page) {
            frame_table[i] = page;
            ll_insert_head(&free_page_list, page);
        }
    }
    
    used_page_list = NULL;
    
    /* Reset statistics */
    pt_accesses = 0;
    page_faults = 0;
    page_faults_dirty = 0;
}

pt_result_t pagetable_lookup(uint32_t vpn, uint32_t *ppn, bool *dirty) {
    pt_accesses++;
    
    /* Check if page is present */
    if (page_table[vpn].present) {
        /* Page hit */
        *ppn = page_table[vpn].ppn;
        *dirty = page_table[vpn].dirty;
        
        /* Update LRU - move page to head of used list */
        page_t *page = frame_table[page_table[vpn].ppn];
        if (page && page != used_page_list) {
            ll_move_to_head(&used_page_list, page);
        }
        
        return PT_HIT;
    }
    
    /* Page fault */
    return PT_MISS;
}

uint32_t pagetable_handle_fault(uint32_t vpn) {
    page_faults++;
    
    page_t *page = NULL;
    
    /* Try to get a free page */
    page = get_free_page();
    
    if (!page) {
        /* No free pages - must evict */
        page = get_victim_page();
        
        if (!page) {
            fprintf(stderr, "FATAL: No pages available for allocation\n");
            exit(1);
        }
        
        /* Write back if dirty */
        if (page->pte && page->pte->dirty) {
            page_faults_dirty++;
            write_page_to_disk(page->data);
            page->pte->dirty = false;
        }
    }
    
    /* Read new page from disk */
    read_page_from_disk(page->data, vpn);
    
    /* Update page table entry */
    page_table[vpn].present = true;
    page_table[vpn].dirty = false;
    page_table[vpn].ppn = page->frame_id;
    
    /* Link page to PTE */
    page->pte = &page_table[vpn];
    
    /* Add to head of used list (most recently used) */
    ll_insert_head(&used_page_list, page);
    
    return page->frame_id;
}

void pagetable_set_dirty(uint32_t vpn) {
    if (page_table[vpn].present) {
        page_table[vpn].dirty = true;
    }
}

void pagetable_print_stats(void) {
    printf("\n* Page Table Statistics *\n");
    printf("total accesses: %llu\n", (unsigned long long)pt_accesses);
    printf("page faults: %llu\n", (unsigned long long)page_faults);
    printf("page faults with a dirty bit: %llu\n", (unsigned long long)page_faults_dirty);
}

void pagetable_print_entries(void) {
    printf("\nPage Table Entries (Present-Bit Dirty-Bit VPN PPN)\n");
    
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        if (page_table[i].present) {
            printf("%d %d 0x%05x 0x%05x\n",
                   1,
                   page_table[i].dirty ? 1 : 0,
                   i,
                   page_table[i].ppn);
        }
    }
}

void pagetable_destroy(void) {
    /* Free all pages */
    for (uint32_t i = 0; i < NUM_PHYSICAL_PAGES; i++) {
        if (frame_table[i]) {
            free(frame_table[i]);
            frame_table[i] = NULL;
        }
    }
    
    free_page_list = NULL;
    used_page_list = NULL;
}

