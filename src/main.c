/**
 * @file main.c
 * @brief Main program - unified for all tasks
 * @author Amir Noohi
 * @copyright Copyright (c) 2025 Amir Noohi. All rights reserved.
 * 
 * Single main program that handles Tasks 1-4 based on configuration.
 * Orchestrates VM translation (TLB + Page Table) and cache access.
 * 
 * PROVIDED TO STUDENTS
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "config.h"
#include "ll.h"
#include "cache.h"
#include "multilevel_cache.h"
#include "tlb.h"
#include "pagetable.h"

/* ============================================================================
 * Global State
 * ============================================================================ */

static tlb_t *tlb = NULL;
static cache_t *single_cache = NULL;
static multilevel_cache_t *multi_cache = NULL;
static sim_config_t *config = NULL;

/* ============================================================================
 * Address Translation
 * ============================================================================ */

/**
 * @brief Extract VPN from virtual address
 */
static inline uint32_t get_vpn(uint32_t vaddr) {
    return vaddr >> 12;  /* Bits 31-12 */
}

/**
 * @brief Extract offset from virtual address
 */
static inline uint32_t get_offset(uint32_t vaddr) {
    return vaddr & 0xFFF;  /* Bits 11-0 */
}

/**
 * @brief Construct physical address from PPN and offset
 */
static inline uint32_t make_paddr(uint32_t ppn, uint32_t offset) {
    return (ppn << 12) | offset;
}

/**
 * @brief Translate virtual address to physical address
 * 
 * Flow:
 * 1. Extract VPN from virtual address
 * 2. Check TLB for VPN -> PPN translation
 * 3. If TLB miss, check page table
 * 4. If page fault, handle fault (allocate page)
 * 5. Update TLB with translation
 * 6. Construct physical address
 * 7. On write, set dirty bits
 * 
 * @param vaddr Virtual address
 * @param is_write Whether this is a write access
 * @param[out] tlb_result TLB lookup result
 * @param[out] pt_result Page table lookup result
 * @return Physical address
 */
static uint32_t translate_address(uint32_t vaddr, bool is_write,
                                  tlb_result_t *tlb_result,
                                  pt_result_t *pt_result) {
    uint32_t vpn = get_vpn(vaddr);
    uint32_t offset = get_offset(vaddr);
    uint32_t ppn;
    bool dirty;
    
    /* Try TLB first */
    *tlb_result = tlb_lookup(tlb, vpn, &ppn, &dirty);
    
    if (*tlb_result == TLB_HIT) {
        /* TLB hit - we have the translation */
        *pt_result = PT_HIT;  /* Not actually accessed, but indicate success */
        
        /* Update page table LRU (even on TLB hit) */
        uint32_t dummy_ppn;
        bool dummy_dirty;
        pagetable_lookup(vpn, &dummy_ppn, &dummy_dirty);
    } else {
        /* TLB miss - consult page table */
        *pt_result = pagetable_lookup(vpn, &ppn, &dirty);
        
        if (*pt_result == PT_MISS) {
            /* Page fault - allocate page */
            ppn = pagetable_handle_fault(vpn);
        }
        
        /* Update TLB with translation */
        tlb_insert(tlb, vpn, ppn);
    }
    
    /* Set dirty bits on write */
    if (is_write) {
        tlb_set_dirty(tlb, vpn);
        pagetable_set_dirty(vpn);
    }
    
    return make_paddr(ppn, offset);
}

/* ============================================================================
 * Verbose Output
 * ============================================================================ */

/**
 * @brief Print access result in verbose mode
 */
static void print_verbose(char mode, uint32_t vaddr, uint32_t paddr,
                         tlb_result_t tlb_res, pt_result_t pt_res,
                         cache_result_t cache_res) {
    printf("%c 0x%08x 0x%08x ", mode, vaddr, paddr);
    
    /* TLB status */
    if (tlb_res == TLB_HIT) {
        printf("TLB-HIT ");
    } else {
        printf("TLB-MISS ");
    }
    
    /* Page table status */
    if (tlb_res == TLB_HIT) {
        printf("- ");  /* PT not consulted */
    } else if (pt_res == PT_HIT) {
        printf("PAGE-HIT ");
    } else {
        printf("PAGE-FAULT ");
    }
    
    /* Cache status */
    if (config->task == 4) {
        /* Multi-level cache */
        switch (cache_res) {
            case CACHE_HIT_L1:
                printf("L1-HIT L2-HIT");
                break;
            case CACHE_HIT_L2:
                printf("L1-MISS L2-HIT");
                break;
            case CACHE_MISS_ALL_LEVELS:
                printf("L1-MISS L2-MISS");
                break;
            default:
                printf("CACHE-ERROR");
                break;
        }
    } else {
        /* Single-level cache */
        if (cache_res == CACHE_HIT) {
            printf("CACHE-HIT");
        } else {
            printf("CACHE-MISS");
        }
    }
    
    printf("\n");
}

/* ============================================================================
 * Main Program
 * ============================================================================ */

int main(int argc, char *argv[]) {
    /* Parse and validate configuration */
    config = parse_arguments(argc, argv);
    if (!config) {
        fprintf(stderr, "Invalid configuration\n");
        return 1;
    }
    
    if (!validate_config(config)) {
        free_config(config);
        return 1;
    }
    
    /* Optional: Print configuration for debugging */
    // print_config(config);
    
    /* Initialize TLB */
    tlb = tlb_init(config->tlb);
    if (!tlb) {
        fprintf(stderr, "Failed to initialize TLB\n");
        free_config(config);
        return 1;
    }
    
    /* Initialize page table */
    pagetable_init();
    
    /* Initialize cache based on task */
    if (config->task == 4) {
        /* Multi-level cache (Task 4) */
        multi_cache = multilevel_cache_init(config->levels, config->num_levels);
        if (!multi_cache) {
            fprintf(stderr, "Failed to initialize multi-level cache\n");
            tlb_destroy(tlb);
            pagetable_destroy();
            free_config(config);
            return 1;
        }
    } else {
        /* Single-level cache (Tasks 1-3) */
        single_cache = cache_init(config->cache);
        if (!single_cache) {
            fprintf(stderr, "Failed to initialize cache\n");
            tlb_destroy(tlb);
            pagetable_destroy();
            free_config(config);
            return 1;
        }
    }
    
    /* Process trace file */
    FILE *trace = fopen(config->trace_file, "r");
    if (!trace) {
        fprintf(stderr, "Invalid configuration\n");
        if (multi_cache) multilevel_cache_destroy(multi_cache);
        if (single_cache) cache_destroy(single_cache);
        tlb_destroy(tlb);
        pagetable_destroy();
        free_config(config);
        return 1;
    }
    
    char mode;
    uint32_t vaddr;
    
    while (fscanf(trace, " %c 0x%x", &mode, &vaddr) == 2) {
        bool is_write = (mode == 'W' || mode == 'w');
        
        /* Translate virtual to physical address */
        tlb_result_t tlb_res;
        pt_result_t pt_res;
        uint32_t paddr = translate_address(vaddr, is_write, &tlb_res, &pt_res);
        
        /* Access cache */
        cache_result_t cache_res;
        if (config->task == 4) {
            cache_res = multilevel_cache_access(multi_cache, paddr, is_write);
        } else {
            cache_res = cache_access(single_cache, paddr, is_write);
        }
        
        /* Verbose output */
        if (config->verbose) {
            print_verbose(mode, vaddr, paddr, tlb_res, pt_res, cache_res);
        }
    }
    
    fclose(trace);
    
    /* Print statistics */
    tlb_print_stats(tlb);
    pagetable_print_stats();
    
    if (config->task == 4) {
        multilevel_cache_print_stats(multi_cache);
    } else {
        cache_print_stats(single_cache, NULL);
    }
    
    /* Verbose mode: print entries */
    if (config->verbose) {
        tlb_print_entries(tlb);
        pagetable_print_entries();
    }
    
    /* Cleanup */
    if (multi_cache) multilevel_cache_destroy(multi_cache);
    if (single_cache) cache_destroy(single_cache);
    tlb_destroy(tlb);
    pagetable_destroy();
    free_config(config);
    
    return 0;
}

