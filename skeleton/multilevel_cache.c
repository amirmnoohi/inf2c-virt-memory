/**
 * @file multilevel_cache.c (STUDENT VERSION)
 * @brief Multi-level cache implementation - TO BE COMPLETED BY STUDENTS
 * 
 * Task 4: Implement a 2-level cache hierarchy (L1 + L2)
 * 
 * Design goal: Make this extensible to 3+ levels in the future
 * by using arrays and loops instead of hardcoding L1 and L2.
 */

#include <stdio.h>
#include <stdlib.h>
#include "multilevel_cache.h"
#include "cache.h"
#include "types.h"

/* ============================================================================
 * Initialization - TO BE IMPLEMENTED
 * ============================================================================ */

multilevel_cache_t* multilevel_cache_init(cache_config_t *configs, uint32_t num_levels) {
    // TODO: Task 4 - Allocate multilevel_cache_t structure
    // TODO: Task 4 - Validate num_levels (must be >= 2 and <= MAX_CACHE_LEVELS)
    // TODO: Task 4 - Validate hierarchy (L2 size >= L1 size, etc.)
    // TODO: Task 4 - For each level:
    //        - Call cache_init() to create the cache
    //        - Store in levels[] array
    // TODO: Task 4 - Initialize statistics arrays
    
    // Hints:
    // - Use a loop to initialize each level (extensible design!)
    // - Check that configs is not NULL
    // - Validate that L2 size >= L1 size
    // - Validate that block sizes are compatible
    
    (void)configs;
    (void)num_levels;
    return NULL;  // STUDENT: Replace with actual implementation
}

/* ============================================================================
 * Cache Access - TO BE IMPLEMENTED
 * ============================================================================ */

cache_result_t multilevel_cache_access(multilevel_cache_t *mlc, 
                                       uint32_t addr, 
                                       bool is_write) {
    // TODO: Task 4 - Loop through each cache level sequentially
    // TODO: Task 4 - Try L1 first (level 0)
    //        - If hit: return CACHE_HIT_L1
    // TODO: Task 4 - If L1 miss, try L2 (level 1)
    //        - If hit: return CACHE_HIT_L2 (or CACHE_MISS_L1_HIT_L2)
    //        - Track L2 accesses separately
    // TODO: Task 4 - If all levels miss, return CACHE_MISS_ALL_LEVELS
    
    // Hints:
    // - Use a for loop: for (level = 0; level < num_levels; level++)
    // - This makes it extensible to L3, L4, etc.!
    // - Call cache_access() for each level
    // - Only track accesses for level > 0 (L2, L3, ...)
    
    if (!mlc) {
        return CACHE_MISS_ALL_LEVELS;
    }
    
    // STUDENT: Implement the access logic
    
    return CACHE_MISS_ALL_LEVELS;  // STUDENT: Replace
}

/* ============================================================================
 * Statistics - PROVIDED
 * ============================================================================ */

void multilevel_cache_print_stats(const multilevel_cache_t *mlc) {
    if (!mlc) return;
    
    /* Print statistics for each level */
    for (uint32_t i = 0; i < mlc->num_levels; i++) {
        char label[32];
        snprintf(label, sizeof(label), "L%u Cache", i + 1);
        cache_print_stats(mlc->levels[i], label);
    }
}

/* ============================================================================
 * Cleanup - TO BE IMPLEMENTED
 * ============================================================================ */

void multilevel_cache_destroy(multilevel_cache_t *mlc) {
    // TODO: Task 4 - Destroy all cache levels
    // TODO: Task 4 - Free multilevel_cache_t structure
    
    // Hint: Loop through levels and call cache_destroy() for each
    
    if (!mlc) return;
    
    // STUDENT: Implement cleanup
    
    free(mlc);
}

cache_t* multilevel_cache_get_level(multilevel_cache_t *mlc, uint32_t level) {
    if (!mlc || level >= mlc->num_levels) {
        return NULL;
    }
    return mlc->levels[level];
}

