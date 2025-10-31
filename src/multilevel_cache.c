/**
 * @file multilevel_cache.c
 * @brief Multi-level cache implementation
 * @author Amir Noohi
 * @copyright Copyright (c) 2025 Amir Noohi. All rights reserved.
 * 
 * Extensible implementation supporting 2+ cache levels using
 * loop-based access pattern. Easy to extend to L3, L4, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include "multilevel_cache.h"
#include "cache.h"
#include "types.h"

/**
 * @brief Encode hit result based on level number
 */
static cache_result_t encode_hit_level(uint32_t level) {
    /* L1 hit = CACHE_HIT_L1 (10), L2 hit = CACHE_HIT_L2 (11), etc. */
    return CACHE_HIT_L1 + level;
}

/**
 * @brief Validate 2-level cache hierarchy
 * 
 * Ensures that L2 size >= L1 size and block sizes are compatible.
 */
static bool validate_hierarchy(cache_config_t *configs, uint32_t num_levels) {
    for (uint32_t i = 1; i < num_levels; i++) {
        /* Each level should be >= previous level */
        if (configs[i].size < configs[i-1].size) {
            fprintf(stderr, "Invalid configuration: L%u size must be >= L%u size\n", 
                    i+1, i);
            return false;
        }
        
        /* Block sizes should be compatible (typically same or increasing) */
        if (configs[i].block_size < configs[i-1].block_size) {
            fprintf(stderr, "Invalid configuration: L%u block size incompatible\n", i+1);
            return false;
        }
    }
    return true;
}

multilevel_cache_t* multilevel_cache_init(cache_config_t *configs, uint32_t num_levels) {
    /* Validate input - Task 4 requires exactly 2 levels */
    if (num_levels != 2) {
        fprintf(stderr, "Invalid number of cache levels: %u (Task 4 requires exactly 2 levels)\n",
                num_levels);
        return NULL;
    }
    
    if (!configs) {
        fprintf(stderr, "NULL configuration array\n");
        return NULL;
    }
    
    /* Validate hierarchy */
    if (!validate_hierarchy(configs, num_levels)) {
        return NULL;
    }
    
    /* Allocate multi-level cache structure */
    multilevel_cache_t *mlc = calloc(1, sizeof(multilevel_cache_t));
    if (!mlc) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    mlc->num_levels = num_levels;
    
    /* Initialize each cache level */
    for (uint32_t i = 0; i < num_levels; i++) {
        mlc->levels[i] = cache_init(configs[i]);
        
        if (!mlc->levels[i]) {
            /* Cleanup on failure */
            for (uint32_t j = 0; j < i; j++) {
                cache_destroy(mlc->levels[j]);
            }
            free(mlc);
            return NULL;
        }
        
        mlc->level_accesses[i] = 0;
    }
    
    return mlc;
}

cache_result_t multilevel_cache_access(multilevel_cache_t *mlc, 
                                       uint32_t addr, 
                                       bool is_write) {
    if (!mlc) {
        return CACHE_MISS_ALL_LEVELS;
    }
    
    /*
     * 2-level cache access pattern for Task 4
     * Check L1 first, then L2 on miss
     */
    for (uint32_t level = 0; level < mlc->num_levels; level++) {
        cache_t *cache = mlc->levels[level];
        
        /* Track accesses to each level (L1 gets all, L2+ only on L1 miss, etc.) */
        if (level > 0) {
            mlc->level_accesses[level]++;
        }
        
        /* Access this cache level */
        cache_result_t result = cache_access(cache, addr, is_write);
        
        if (result == CACHE_HIT) {
            /*
             * HIT at this level (L1 or L2)
             * Return encoded result indicating which level hit
             */
            return encode_hit_level(level);
        }
    }
    
    /*
     * Missed at both L1 and L2 - fetch from memory
     * The cache_access() calls already handle installation
     */
    return CACHE_MISS_ALL_LEVELS;
}

void multilevel_cache_print_stats(const multilevel_cache_t *mlc) {
    if (!mlc) return;
    
    /* Print statistics for each level */
    for (uint32_t i = 0; i < mlc->num_levels; i++) {
        char label[32];
        snprintf(label, sizeof(label), "L%u Cache", i + 1);
        cache_print_stats(mlc->levels[i], label);
    }
    
    /* Optional: Print hierarchy-specific statistics */
    if (mlc->num_levels >= 2) {
        printf("\n* Multi-Level Cache Summary *\n");
        for (uint32_t i = 0; i < mlc->num_levels; i++) {
            printf("L%u accesses: %llu\n", i + 1, 
                   (unsigned long long)(i == 0 ? mlc->levels[i]->accesses : 
                                                 mlc->level_accesses[i]));
        }
    }
}

void multilevel_cache_destroy(multilevel_cache_t *mlc) {
    if (!mlc) return;
    
    /* Destroy all cache levels */
    for (uint32_t i = 0; i < mlc->num_levels; i++) {
        cache_destroy(mlc->levels[i]);
    }
    
    free(mlc);
}

cache_t* multilevel_cache_get_level(multilevel_cache_t *mlc, uint32_t level) {
    if (!mlc || level >= mlc->num_levels) {
        return NULL;
    }
    return mlc->levels[level];
}

