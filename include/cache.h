/**
 * @file cache.h
 * @brief Unified cache interface (Tasks 1-3)
 * @author Amir Noohi
 * @copyright Copyright (c) 2025 Amir Noohi. All rights reserved.
 * 
 * Single cache implementation that handles all associativity modes:
 * - Task 1: Fully associative, fixed block size
 * - Task 2: Fully associative, variable block size
 * - Task 3: All associativity modes (direct-mapped, 2-way, 4-way, fully-assoc)
 * 
 * Also used as building block for Task 4 multi-level cache (L1 and L2).
 */

#ifndef CACHE_H
#define CACHE_H

#include "types.h"

/**
 * @brief Initialize cache with given configuration
 * 
 * Creates and initializes a cache with the specified size, block size,
 * and associativity. Allocates all necessary data structures including
 * cache sets, cache lines, and LRU tracking structures.
 * 
 * Algorithm:
 * 1. Calculate number of sets based on associativity
 * 2. Calculate ways per set
 * 3. Calculate bit field sizes (offset, index, tag)
 * 4. Allocate cache sets and lines
 * 5. Initialize LRU lists for set-associative caches
 * 
 * @param config Cache configuration (size, block_size, associativity)
 * @return Pointer to initialized cache, or NULL on error
 */
cache_t* cache_init(cache_config_t config);

/**
 * @brief Access cache with given physical address
 * 
 * Performs a cache access (read or write) at the specified physical address.
 * Handles all associativity modes internally through configuration-driven logic.
 * 
 * Algorithm:
 * 1. Update access statistics
 * 2. Extract index, tag, offset from address
 * 3. Search cache set for matching tag
 * 4. If HIT:
 *    a. Update hit statistics
 *    b. Update LRU (move to head if set-associative)
 *    c. Set dirty bit if write
 * 5. If MISS:
 *    a. Update miss statistics
 *    b. Select victim line (LRU or invalid)
 *    c. Evict victim if necessary
 *    d. Insert new line
 * 
 * @param cache Cache instance
 * @param addr Physical address to access
 * @param is_write true for write access, false for read
 * @return CACHE_HIT or CACHE_MISS
 */
cache_result_t cache_access(cache_t *cache, uint32_t addr, bool is_write);

/**
 * @brief Print cache statistics
 * 
 * Outputs cache statistics in the format specified by the assignment.
 * 
 * Format:
 *   * Cache Statistics *
 *   total accesses: X
 *   hits: X
 *   misses: X
 *   total reads: X
 *   read hits: X
 *   total writes: X
 *   write hits: X
 * 
 * @param cache Cache instance
 * @param label Optional label (e.g., "L1 Cache", "L2 Cache", or NULL)
 */
void cache_print_stats(const cache_t *cache, const char *label);

/**
 * @brief Destroy cache and free all resources
 * 
 * Frees all allocated memory including cache sets, lines, and data blocks.
 * 
 * @param cache Cache to destroy
 */
void cache_destroy(cache_t *cache);

/* ============================================================================
 * Internal helper functions (used by cache.c, exposed for testing)
 * ============================================================================ */

/**
 * @brief Calculate set index from address
 * 
 * Extracts the index bits from the physical address based on cache configuration.
 * For fully-associative: always returns 0
 * For direct-mapped/set-associative: extracts appropriate index bits
 * 
 * @param cache Cache instance
 * @param addr Physical address
 * @return Set index
 */
uint32_t cache_get_index(const cache_t *cache, uint32_t addr);

/**
 * @brief Calculate tag from address
 * 
 * Extracts the tag bits from the physical address.
 * 
 * @param cache Cache instance
 * @param addr Physical address
 * @return Tag value
 */
uint32_t cache_get_tag(const cache_t *cache, uint32_t addr);

/**
 * @brief Calculate offset from address
 * 
 * Extracts the offset bits (position within cache line).
 * 
 * @param cache Cache instance
 * @param addr Physical address
 * @return Offset within cache line
 */
uint32_t cache_get_offset(const cache_t *cache, uint32_t addr);

/**
 * @brief Calculate log base 2 of a number
 * 
 * Utility function for calculating bit field sizes.
 * Assumes n is a power of 2.
 * 
 * @param n Number (must be power of 2)
 * @return log2(n)
 */
uint32_t log2_uint32(uint32_t n);

#endif /* CACHE_H */

