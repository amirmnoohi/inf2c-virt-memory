/**
 * @file multilevel_cache.h
 * @brief Multi-level cache hierarchy (Task 4)
 * 
 * 2-level cache design (L1, L2) for Task 4.
 * Reuses the unified cache implementation for each level.
 */

#ifndef MULTILEVEL_CACHE_H
#define MULTILEVEL_CACHE_H

#include "types.h"
#include "cache.h"

/**
 * @brief Initialize multi-level cache hierarchy
 * 
 * Creates a 2-level cache hierarchy (L1 and L2) for Task 4.
 * Each level is implemented using the unified cache from cache.c.
 * 
 * Algorithm:
 * 1. Validate num_levels (must be exactly 2)
 * 2. For each level, initialize cache using cache_init()
 * 3. Validate hierarchy (L2 >= L1, compatible block sizes, etc.)
 * 
 * @param configs Array of cache configurations [L1, L2]
 * @param num_levels Number of cache levels (must be 2)
 * @return Pointer to multi-level cache structure, or NULL on error
 */
multilevel_cache_t* multilevel_cache_init(cache_config_t *configs, uint32_t num_levels);

/**
 * @brief Access multi-level cache hierarchy
 * 
 * Tries each cache level sequentially until hit or all levels miss.
 * Automatically handles data promotion from lower to upper levels.
 * 
 * Access flow (2-level example):
 * 1. Try L1
 *    - If hit: return CACHE_HIT_L1
 *    - If miss: continue to L2
 * 2. Try L2
 *    - If hit: promote to L1, return CACHE_MISS_L1_HIT_L2
 *    - If miss: fetch from memory, insert into L2 and L1, return CACHE_MISS_ALL_LEVELS
 * 
 * For 3+ levels: Loop continues through all levels
 * 
 * @param mlc Multi-level cache instance
 * @param addr Physical address to access
 * @param is_write true for write, false for read
 * @return Result code indicating which level hit (or all miss)
 */
cache_result_t multilevel_cache_access(multilevel_cache_t *mlc, 
                                       uint32_t addr, 
                                       bool is_write);

/**
 * @brief Print statistics for all cache levels
 * 
 * Prints separate statistics for each level:
 * - L1 Cache Statistics
 * - L2 Cache Statistics
 * - (L3, L4, ... if more levels exist)
 * 
 * @param mlc Multi-level cache instance
 */
void multilevel_cache_print_stats(const multilevel_cache_t *mlc);

/**
 * @brief Destroy multi-level cache and free resources
 * 
 * Destroys all cache levels and frees the multi-level structure.
 * 
 * @param mlc Multi-level cache to destroy
 */
void multilevel_cache_destroy(multilevel_cache_t *mlc);

/**
 * @brief Get cache at specific level (for testing/debugging)
 * 
 * @param mlc Multi-level cache instance
 * @param level Level number (0 = L1, 1 = L2, ...)
 * @return Pointer to cache at specified level, or NULL if invalid level
 */
cache_t* multilevel_cache_get_level(multilevel_cache_t *mlc, uint32_t level);

#endif /* MULTILEVEL_CACHE_H */

