/**
 * @file cache.c
 * @brief Unified cache implementation
 * @author Amir Noohi
 * @copyright Copyright (c) 2025 Amir Noohi. All rights reserved.
 * 
 * Expert-level implementation that handles all cache configurations
 * through parameterization, not code duplication. Supports:
 * - Direct-mapped, 2-way, 4-way, and fully-associative
 * - Variable block sizes
 * - Write-back, write-allocate policy
 * - LRU replacement
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "types.h"

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

uint32_t log2_uint32(uint32_t n) {
    uint32_t log = 0;
    while (n > 1) {
        n >>= 1;
        log++;
    }
    return log;
}

/**
 * @brief Initialize a cache line
 */
static void init_cache_line(cache_line_t *line, uint32_t block_size) {
    line->valid = false;
    line->dirty = false;
    line->tag = 0;
    line->data = calloc(block_size, sizeof(uint8_t));
    line->prev = NULL;
    line->next = NULL;
}

/**
 * @brief Initialize LRU list for a cache set
 * 
 * Creates a doubly-linked list of cache lines ordered by LRU.
 * Head = MRU (most recently used)
 * Tail = LRU (least recently used, victim candidate)
 */
static void init_lru_list(cache_set_t *set) {
    if (set->num_ways <= 1) return;  /* No LRU for direct-mapped */
    
    /* Link all lines in order */
    set->lru_head = &set->lines[0];
    set->lru_tail = &set->lines[set->num_ways - 1];
    
    for (uint32_t i = 0; i < set->num_ways; i++) {
        if (i > 0) {
            set->lines[i].prev = &set->lines[i - 1];
        }
        if (i < set->num_ways - 1) {
            set->lines[i].next = &set->lines[i + 1];
        }
    }
}

/**
 * @brief Move a cache line to the head of LRU list (mark as MRU)
 */
static void lru_move_to_head(cache_set_t *set, cache_line_t *line) {
    if (!line || set->num_ways <= 1 || line == set->lru_head) {
        return;  /* Already at head or no LRU tracking needed */
    }
    
    /* Remove from current position */
    if (line->prev) {
        line->prev->next = line->next;
    }
    if (line->next) {
        line->next->prev = line->prev;
    }
    
    /* Update tail if we're removing the tail */
    if (line == set->lru_tail) {
        set->lru_tail = line->prev;
    }
    
    /* Insert at head */
    line->prev = NULL;
    line->next = set->lru_head;
    if (set->lru_head) {
        set->lru_head->prev = line;
    }
    set->lru_head = line;
}

/**
 * @brief Find a cache line with matching tag in a set
 */
static cache_line_t* find_line(cache_set_t *set, uint32_t tag) {
    for (uint32_t i = 0; i < set->num_ways; i++) {
        if (set->lines[i].valid && set->lines[i].tag == tag) {
            return &set->lines[i];
        }
    }
    return NULL;
}

/**
 * @brief Find an invalid (empty) line in a set
 */
static cache_line_t* find_invalid_line(cache_set_t *set) {
    for (uint32_t i = 0; i < set->num_ways; i++) {
        if (!set->lines[i].valid) {
            return &set->lines[i];
        }
    }
    return NULL;
}

/**
 * @brief Select victim line for eviction (LRU)
 */
static cache_line_t* select_victim(cache_set_t *set) {
    /* First, try to find an invalid line */
    cache_line_t *invalid = find_invalid_line(set);
    if (invalid) {
        return invalid;
    }
    
    /* All lines valid, use LRU */
    if (set->num_ways == 1) {
        return &set->lines[0];  /* Direct-mapped: only one choice */
    }
    
    /* Return tail of LRU list (least recently used) */
    return set->lru_tail;
}

/**
 * @brief Simulate reading a block from memory
 * 
 * Dummy function - in real hardware, this would fetch data from memory.
 */
static void read_block_from_memory(cache_line_t *line, uint32_t addr) {
    /* In simulation, we don't actually read data */
    (void)line;
    (void)addr;
}

/**
 * @brief Simulate writing a dirty block to memory
 * 
 * Dummy function - in real hardware, this would write data to memory.
 */
static void write_block_to_memory(cache_line_t *line, uint32_t addr) {
    /* In simulation, we don't actually write data */
    (void)line;
    (void)addr;
}

/* ============================================================================
 * Address Parsing Functions
 * ============================================================================ */

uint32_t cache_get_index(const cache_t *cache, uint32_t addr) {
    if (cache->associativity == FULLY_ASSOC) {
        return 0;  /* Fully-assoc has only 1 set */
    }
    
    /* Extract index bits */
    uint32_t index_mask = (1 << cache->index_bits) - 1;
    return (addr >> cache->offset_bits) & index_mask;
}

uint32_t cache_get_tag(const cache_t *cache, uint32_t addr) {
    uint32_t shift = cache->offset_bits + cache->index_bits;
    return addr >> shift;
}

uint32_t cache_get_offset(const cache_t *cache, uint32_t addr) {
    uint32_t offset_mask = (1 << cache->offset_bits) - 1;
    return addr & offset_mask;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

cache_t* cache_init(cache_config_t config) {
    cache_t *cache = calloc(1, sizeof(cache_t));
    if (!cache) {
        return NULL;
    }
    
    /* Store configuration */
    cache->size = config.size;
    cache->block_size = config.block_size;
    cache->associativity = config.associativity;
    
    /* Calculate cache geometry */
    switch (config.associativity) {
        case DIRECT_MAPPED:
            cache->num_sets = config.size / config.block_size;
            cache->ways_per_set = 1;
            break;
            
        case FULLY_ASSOC:
            cache->num_sets = 1;
            cache->ways_per_set = config.size / config.block_size;
            break;
            
        case TWO_WAY:
            cache->ways_per_set = 2;
            cache->num_sets = config.size / (config.block_size * 2);
            break;
            
        case FOUR_WAY:
            cache->ways_per_set = 4;
            cache->num_sets = config.size / (config.block_size * 4);
            break;
            
        default:
            free(cache);
            return NULL;
    }
    
    /* Calculate bit field sizes */
    cache->offset_bits = log2_uint32(config.block_size);
    cache->index_bits = (config.associativity == FULLY_ASSOC) ? 
                        0 : log2_uint32(cache->num_sets);
    cache->tag_bits = 32 - cache->offset_bits - cache->index_bits;
    
    /* Allocate cache sets */
    cache->sets = calloc(cache->num_sets, sizeof(cache_set_t));
    if (!cache->sets) {
        free(cache);
        return NULL;
    }
    
    /* Initialize each set */
    for (uint32_t i = 0; i < cache->num_sets; i++) {
        cache->sets[i].num_ways = cache->ways_per_set;
        cache->sets[i].lines = calloc(cache->ways_per_set, sizeof(cache_line_t));
        
        if (!cache->sets[i].lines) {
            /* Cleanup on failure */
            for (uint32_t j = 0; j < i; j++) {
                for (uint32_t k = 0; k < cache->sets[j].num_ways; k++) {
                    free(cache->sets[j].lines[k].data);
                }
                free(cache->sets[j].lines);
            }
            free(cache->sets);
            free(cache);
            return NULL;
        }
        
        /* Initialize each line in the set */
        for (uint32_t j = 0; j < cache->ways_per_set; j++) {
            init_cache_line(&cache->sets[i].lines[j], config.block_size);
        }
        
        /* Initialize LRU list */
        init_lru_list(&cache->sets[i]);
    }
    
    /* Initialize statistics */
    cache->accesses = 0;
    cache->hits = 0;
    cache->misses = 0;
    cache->reads = 0;
    cache->read_hits = 0;
    cache->writes = 0;
    cache->write_hits = 0;
    
    return cache;
}

cache_result_t cache_access(cache_t *cache, uint32_t addr, bool is_write) {
    /* Update access statistics */
    cache->accesses++;
    if (is_write) {
        cache->writes++;
    } else {
        cache->reads++;
    }
    
    /* Parse address */
    uint32_t index = cache_get_index(cache, addr);
    uint32_t tag = cache_get_tag(cache, addr);
    
    /* Get the appropriate cache set */
    cache_set_t *set = &cache->sets[index];
    
    /* Search for matching tag (cache hit?) */
    cache_line_t *line = find_line(set, tag);
    
    if (line) {
        /* CACHE HIT */
        cache->hits++;
        if (is_write) {
            cache->write_hits++;
            line->dirty = true;  /* Mark as dirty for write-back */
        } else {
            cache->read_hits++;
        }
        
        /* Update LRU (mark as most recently used) */
        lru_move_to_head(set, line);
        
        return CACHE_HIT;
    }
    
    /* CACHE MISS */
    cache->misses++;
    
    /* Select victim for replacement */
    cache_line_t *victim = select_victim(set);
    
    /* Evict victim if necessary (write-back policy) */
    if (victim->valid && victim->dirty) {
        /* Write dirty block back to memory */
        uint32_t victim_addr = (victim->tag << (cache->offset_bits + cache->index_bits)) |
                               (index << cache->offset_bits);
        write_block_to_memory(victim, victim_addr);
    }
    
    /* Install new block */
    victim->valid = true;
    victim->dirty = is_write;  /* Dirty if write miss (write-allocate) */
    victim->tag = tag;
    
    /* Read block from memory */
    read_block_from_memory(victim, addr);
    
    /* Move to head of LRU (most recently used) */
    lru_move_to_head(set, victim);
    
    return CACHE_MISS;
}

void cache_print_stats(const cache_t *cache, const char *label) {
    if (label) {
        printf("\n* %s Statistics *\n", label);
    } else {
        printf("\n* Cache Statistics *\n");
    }
    
    printf("total accesses: %llu\n", (unsigned long long)cache->accesses);
    printf("hits: %llu\n", (unsigned long long)cache->hits);
    printf("misses: %llu\n", (unsigned long long)cache->misses);
    printf("total reads: %llu\n", (unsigned long long)cache->reads);
    printf("read hits: %llu\n", (unsigned long long)cache->read_hits);
    printf("total writes: %llu\n", (unsigned long long)cache->writes);
    printf("write hits: %llu\n", (unsigned long long)cache->write_hits);
}

void cache_destroy(cache_t *cache) {
    if (!cache) return;
    
    /* Free all cache lines and their data */
    for (uint32_t i = 0; i < cache->num_sets; i++) {
        for (uint32_t j = 0; j < cache->sets[i].num_ways; j++) {
            free(cache->sets[i].lines[j].data);
        }
        free(cache->sets[i].lines);
    }
    
    free(cache->sets);
    free(cache);
}

