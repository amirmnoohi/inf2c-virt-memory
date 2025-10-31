/**
 * @file cache.c (STUDENT VERSION)
 * @brief Unified cache implementation - TO BE COMPLETED BY STUDENTS
 * 
 * Complete the functions below to implement a cache that supports:
 * - Task 1: Fully associative cache
 * - Task 2: Variable block sizes
 * - Task 3: All associativity modes (direct-mapped, 2-way, 4-way, fully-assoc)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "types.h"

/* ============================================================================
 * Helper Functions - PROVIDED
 * ============================================================================ */

uint32_t log2_uint32(uint32_t n) {
    uint32_t log = 0;
    while (n > 1) {
        n >>= 1;
        log++;
    }
    return log;
}

/* ============================================================================
 * Address Parsing Functions - TO BE IMPLEMENTED
 * ============================================================================ */

uint32_t cache_get_index(const cache_t *cache, uint32_t addr) {
    // TODO: Task 1 - For fully associative, return 0
    // TODO: Task 3 - For set-associative/direct-mapped, extract index bits
    // Hint: offset_bits tells you where index starts
    // Hint: index_bits tells you how many bits to extract
    
    (void)cache;
    (void)addr;
    return 0;  // STUDENT: Replace with actual implementation
}

uint32_t cache_get_tag(const cache_t *cache, uint32_t addr) {
    // TODO: Extract tag bits from address
    // Hint: Tag starts after offset and index bits
    
    (void)cache;
    (void)addr;
    return 0;  // STUDENT: Replace with actual implementation
}

uint32_t cache_get_offset(const cache_t *cache, uint32_t addr) {
    // TODO: Extract offset bits (position within cache line)
    
    (void)cache;
    (void)addr;
    return 0;  // STUDENT: Replace with actual implementation
}

/* ============================================================================
 * Cache Initialization - TO BE IMPLEMENTED
 * ============================================================================ */

cache_t* cache_init(cache_config_t config) {
    // TODO: Task 1 - Use malloc/calloc to allocate cache_t structure
    //       REQUIRED: cache = calloc(1, sizeof(cache_t));
    
    // TODO: Task 1 - Calculate cache geometry (num_sets, ways_per_set)
    //       Hints:
    //       - For fully-assoc: num_sets=1, ways_per_set=size/block_size
    //       - For direct-mapped: num_sets=size/block_size, ways_per_set=1
    //       - For 2-way: num_sets=size/(block_size*2), ways_per_set=2
    //       - For 4-way: num_sets=size/(block_size*4), ways_per_set=4
    
    // TODO: Task 1 - Calculate bit field sizes (offset_bits, index_bits, tag_bits)
    //       offset_bits = log2(block_size)
    //       index_bits = log2(num_sets) for set-assoc, 0 for fully-assoc
    //       tag_bits = 32 - offset_bits - index_bits
    
    // TODO: Task 1 - Use malloc/calloc to allocate cache sets array
    //       REQUIRED: cache->sets = calloc(num_sets, sizeof(cache_set_t));
    
    // TODO: Task 1 - For each set, use malloc/calloc to allocate lines array
    //       REQUIRED: sets[i].lines = calloc(ways_per_set, sizeof(cache_line_t));
    
    // TODO: Task 1 - For each line, use malloc/calloc to allocate data block
    //       REQUIRED: lines[j].data = calloc(block_size, sizeof(uint8_t));
    
    // TODO: Task 1 - Initialize LRU tracking for set-associative/fully-assoc
    //       NOTE: Direct-mapped does NOT need LRU (only 1 way per set)
    //       For SA/FA: Create doubly-linked list (head <-> line[0] <-> ... <-> tail)
    
    // TODO: Task 2 - Handle variable block sizes
    //       (If Task 1 is correct, this should already work!)
    
    // TODO: Task 3 - Support all associativity modes
    //       Use switch statement on config.associativity
    //       NOTE: Set-associative and fully-associative REQUIRE LRU replacement policy
    //             Direct-mapped does NOT need LRU (no replacement choice)
    
    (void)config;
    return NULL;  // STUDENT: Replace with actual implementation
}

/* ============================================================================
 * Cache Access - TO BE IMPLEMENTED
 * ============================================================================ */

cache_result_t cache_access(cache_t *cache, uint32_t addr, bool is_write) {
    // TODO: Task 1 - Update access statistics
    // TODO: Task 1 - Parse address (extract index, tag, offset)
    // TODO: Task 1 - Search cache set for matching tag
    // TODO: Task 1 - If HIT:
    //        - Update hit statistics
    //        - Update LRU (move to head)
    //        - Set dirty bit if write
    //        - Return CACHE_HIT
    // TODO: Task 1 - If MISS:
    //        - Update miss statistics
    //        - Select victim (invalid line or LRU)
    //        - Evict victim if dirty (write-back)
    //        - Install new line
    //        - Return CACHE_MISS
    
    (void)cache;
    (void)addr;
    (void)is_write;
    return CACHE_MISS;  // STUDENT: Replace with actual logic
}

/* ============================================================================
 * Statistics and Cleanup - PROVIDED
 * ============================================================================ */

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
    // TODO: Free all allocated memory in REVERSE order of allocation
    //       Must free every malloc/calloc you did in cache_init!
    
    // Order matters to avoid memory leaks:
    // 1. For each set:
    //    a. For each line in set: free(line->data)
    //    b. free(set->lines)
    // 2. free(cache->sets)
    // 3. free(cache)
    
    if (!cache) return;
    
    // STUDENT: Implement cleanup
    // Hint: Nested loops matching your allocation in cache_init
    
    free(cache);
}

