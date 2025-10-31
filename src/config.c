/**
 * @file config.c
 * @brief Configuration parsing and validation implementation
 * @author Amir Noohi
 * @copyright Copyright (c) 2025 Amir Noohi. All rights reserved.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "config.h"
#include "types.h"

/**
 * @brief Convert associativity code to assoc_type_t
 */
static assoc_type_t parse_assoc(int code) {
    switch (code) {
        case 1: return DIRECT_MAPPED;
        case 2: return FULLY_ASSOC;
        case 3: return TWO_WAY;
        case 4: return FOUR_WAY;
        default: return FULLY_ASSOC;  /* Default to fully-assoc */
    }
}

/**
 * @brief Check if a number is a power of 2
 */
static bool is_power_of_2(uint32_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

/**
 * @brief Check if file exists
 */
static bool file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

sim_config_t* parse_arguments(int argc, char *argv[]) {
    sim_config_t *config = calloc(1, sizeof(sim_config_t));
    if (!config) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    /* Set defaults for Task 1 */
    config->cache.associativity = DIRECT_MAPPED;
    config->cache.block_size = DEFAULT_BLOCK_SIZE;
    config->cache.size = 0;  /* Must be specified */
    
    config->tlb.num_entries = 0;  /* Must be specified */
    config->tlb.associativity = DIRECT_MAPPED;
    
    config->verbose = false;
    config->trace_file = NULL;
    config->num_levels = 0;
    
    /* Parse arguments manually to handle -S1, -S2, etc. */
    bool has_l1 = false, has_l2 = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-S") == 0 && i + 1 < argc) {
            config->cache.size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-S1") == 0 && i + 1 < argc) {
            config->levels[0].size = atoi(argv[++i]);
            has_l1 = true;
        } else if (strcmp(argv[i], "-S2") == 0 && i + 1 < argc) {
            config->levels[1].size = atoi(argv[++i]);
            has_l2 = true;
        } else if (strcmp(argv[i], "-B") == 0 && i + 1 < argc) {
            config->cache.block_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-B1") == 0 && i + 1 < argc) {
            config->levels[0].block_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-B2") == 0 && i + 1 < argc) {
            config->levels[1].block_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-A") == 0 && i + 1 < argc) {
            config->cache.associativity = parse_assoc(atoi(argv[++i]));
        } else if (strcmp(argv[i], "-A1") == 0 && i + 1 < argc) {
            config->levels[0].associativity = parse_assoc(atoi(argv[++i]));
        } else if (strcmp(argv[i], "-A2") == 0 && i + 1 < argc) {
            config->levels[1].associativity = parse_assoc(atoi(argv[++i]));
        } else if (strcmp(argv[i], "-T") == 0 && i + 1 < argc) {
            config->tlb.num_entries = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-L") == 0 && i + 1 < argc) {
            config->tlb.associativity = parse_assoc(atoi(argv[++i]));
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            config->trace_file = strdup(argv[++i]);
        } else if (strcmp(argv[i], "-v") == 0) {
            config->verbose = true;
        } else {
            fprintf(stderr, "Invalid configuration\n");
            free_config(config);
            return NULL;
        }
    }
    
    /* Determine if multi-level cache */
    if (has_l1 && has_l2) {
        config->num_levels = 2;
        /* Set defaults for unspecified parameters */
        if (config->levels[0].block_size == 0)
            config->levels[0].block_size = DEFAULT_BLOCK_SIZE;
        if (config->levels[1].block_size == 0)
            config->levels[1].block_size = DEFAULT_BLOCK_SIZE;
        if (config->levels[0].associativity == 0)
            config->levels[0].associativity = FULLY_ASSOC;
        if (config->levels[1].associativity == 0)
            config->levels[1].associativity = FULLY_ASSOC;
    }
    
    /* Detect task */
    config->task = detect_task(config);
    
    return config;
}

int detect_task(const sim_config_t *config) {
    if (config->num_levels >= 2) {
        return 4;  /* Multi-level cache */
    }
    if (config->cache.associativity != FULLY_ASSOC) {
        return 3;  /* Associativity specified */
    }
    if (config->cache.block_size != DEFAULT_BLOCK_SIZE) {
        return 2;  /* Block size specified */
    }
    return 1;  /* Defaults (Task 1) */
}

bool validate_config(const sim_config_t *config) {
    /* Validate single-level cache */
    if (config->num_levels == 0) {
        /* Cache size must be positive and multiple of 4 */
        if (config->cache.size <= 0 || config->cache.size % 4 != 0) {
            fprintf(stderr, "Invalid configuration\n");
            return false;
        }
        
        /* Block size must be >= 4 and multiple of 4 */
        if (config->cache.block_size < 4 || config->cache.block_size % 4 != 0) {
            fprintf(stderr, "Invalid configuration\n");
            return false;
        }
        
        /* Block size cannot exceed cache size */
        if (config->cache.block_size > config->cache.size) {
            fprintf(stderr, "Invalid configuration\n");
            return false;
        }
        
        /* Validate associativity constraints */
        switch (config->cache.associativity) {
            case TWO_WAY:
                if (config->cache.size % (config->cache.block_size * 2) != 0) {
                    fprintf(stderr, "Invalid configuration\n");
                    return false;
                }
                break;
            case FOUR_WAY:
                if (config->cache.size % (config->cache.block_size * 4) != 0) {
                    fprintf(stderr, "Invalid configuration\n");
                    return false;
                }
                break;
            default:
                break;
        }
    } else {
        /* Validate multi-level cache */
        for (uint32_t i = 0; i < config->num_levels; i++) {
            if (config->levels[i].size <= 0 || config->levels[i].size % 4 != 0) {
                fprintf(stderr, "Invalid configuration\n");
                return false;
            }
            if (config->levels[i].block_size < 4 || config->levels[i].block_size % 4 != 0) {
                fprintf(stderr, "Invalid configuration\n");
                return false;
            }
            
            /* Block size cannot exceed cache size */
            if (config->levels[i].block_size > config->levels[i].size) {
                fprintf(stderr, "Invalid configuration\n");
                return false;
            }
            
            /* Check associativity constraints */
            switch (config->levels[i].associativity) {
                case TWO_WAY:
                    if (config->levels[i].size % (config->levels[i].block_size * 2) != 0) {
                        fprintf(stderr, "Invalid configuration\n");
                        return false;
                    }
                    break;
                case FOUR_WAY:
                    if (config->levels[i].size % (config->levels[i].block_size * 4) != 0) {
                        fprintf(stderr, "Invalid configuration\n");
                        return false;
                    }
                    break;
                default:
                    break;
            }
        }
        
        /* L2 must be >= L1 */
        if (config->num_levels >= 2) {
            if (config->levels[1].size < config->levels[0].size) {
                fprintf(stderr, "Invalid configuration\n");
                return false;
            }
        }
    }
    
    /* Validate TLB */
    if (config->tlb.num_entries < 2) {
        fprintf(stderr, "Invalid configuration\n");
        return false;
    }
    if (!is_power_of_2(config->tlb.num_entries)) {
        fprintf(stderr, "Invalid configuration\n");
        return false;
    }
    
    /* Check TLB associativity constraints */
    switch (config->tlb.associativity) {
        case TWO_WAY:
            if (config->tlb.num_entries % 2 != 0) {
                fprintf(stderr, "Invalid configuration\n");
                return false;
            }
            break;
        case FOUR_WAY:
            if (config->tlb.num_entries % 4 != 0) {
                fprintf(stderr, "Invalid configuration\n");
                return false;
            }
            break;
        default:
            break;
    }
    
    /* Validate trace file */
    if (!config->trace_file) {
        fprintf(stderr, "Invalid configuration\n");
        return false;
    }
    if (!file_exists(config->trace_file)) {
        fprintf(stderr, "Invalid configuration\n");
        return false;
    }
    
    return true;
}

void free_config(sim_config_t *config) {
    if (config) {
        if (config->trace_file) {
            free(config->trace_file);
        }
        free(config);
    }
}

void print_config(const sim_config_t *config) {
    printf("=== Configuration ===\n");
    printf("Task: %d\n", config->task);
    
    if (config->num_levels == 0) {
        printf("Cache: Size=%u, Block=%u, Assoc=%d\n",
               config->cache.size, config->cache.block_size, config->cache.associativity);
    } else {
        for (uint32_t i = 0; i < config->num_levels; i++) {
            printf("L%u Cache: Size=%u, Block=%u, Assoc=%d\n",
                   i+1, config->levels[i].size, config->levels[i].block_size,
                   config->levels[i].associativity);
        }
    }
    
    printf("TLB: Entries=%u, Assoc=%d\n",
           config->tlb.num_entries, config->tlb.associativity);
    printf("Trace: %s\n", config->trace_file);
    printf("Verbose: %s\n", config->verbose ? "Yes" : "No");
    printf("====================\n\n");
}

