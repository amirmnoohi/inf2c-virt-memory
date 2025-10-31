/**
 * @file types.h
 * @brief Common type definitions and structures for the VM/Cache simulator
 * @author Amir Noohi
 * @copyright Copyright (c) 2025 Amir Noohi. All rights reserved.
 * 
 * This file contains all shared data structures, enums, and constants used
 * across the virtual memory and cache simulator. Designed for extensibility
 * to support future enhancements (e.g., L3 cache, different inclusion policies).
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Constants
 * ============================================================================ */

#define PAGE_SIZE 4096              /* 4KB pages */
#define NUM_PHYSICAL_PAGES 256      /* 1MB physical memory */
#define PAGE_TABLE_ENTRIES (1 << 14) /* 2^14 entries for 26-bit virtual addresses */
#define MAX_CACHE_LEVELS 2          /* Task 4: 2-level cache (L1+L2) */

/* Default values for Task 1 */
#define DEFAULT_BLOCK_SIZE 4
#define DEFAULT_ASSOC DIRECT_MAPPED

/* ============================================================================
 * Enumerations
 * ============================================================================ */

/**
 * @brief Cache associativity types
 */
typedef enum {
    DIRECT_MAPPED = 1,    /* 1-way set associative */
    FULLY_ASSOC = 2,      /* Fully associative */
    TWO_WAY = 3,          /* 2-way set associative */
    FOUR_WAY = 4          /* 4-way set associative */
} assoc_type_t;

/**
 * @brief Cache access result codes
 */
typedef enum {
    CACHE_HIT = 0,
    CACHE_MISS = 1,
    /* Multi-level cache results */
    CACHE_HIT_L1 = 10,
    CACHE_HIT_L2 = 11,
    CACHE_HIT_L3 = 12,
    CACHE_MISS_L1_HIT_L2 = 20,
    CACHE_MISS_L1_MISS_L2 = 21,
    CACHE_MISS_ALL_LEVELS = 30
} cache_result_t;

/**
 * @brief TLB access result codes
 */
typedef enum {
    TLB_HIT = 0,
    TLB_MISS = 1
} tlb_result_t;

/**
 * @brief Page table access result codes
 */
typedef enum {
    PT_HIT = 0,
    PT_MISS = 1  /* Page fault */
} pt_result_t;

/* ============================================================================
 * Forward Declarations
 * ============================================================================ */

typedef struct cache_line_s cache_line_t;
typedef struct cache_set_s cache_set_t;
typedef struct cache_s cache_t;
typedef struct cache_config_s cache_config_t;
typedef struct multilevel_cache_s multilevel_cache_t;

typedef struct tlb_entry_s tlb_entry_t;
typedef struct tlb_set_s tlb_set_t;
typedef struct tlb_s tlb_t;
typedef struct tlb_config_s tlb_config_t;

typedef struct page_table_entry_s pte_t;
typedef struct page_s page_t;

typedef struct sim_config_s sim_config_t;

/* ============================================================================
 * Cache Structures
 * ============================================================================ */

/**
 * @brief Cache line (cache block)
 * 
 * Represents a single cache line with metadata and data storage.
 */
struct cache_line_s {
    bool valid;                    /* Valid bit */
    bool dirty;                    /* Dirty bit (for write-back) */
    uint32_t tag;                  /* Tag bits */
    uint8_t *data;                 /* Pointer to data block */
    
    /* LRU tracking (for set-associative caches) */
    cache_line_t *prev;            /* Previous in LRU list */
    cache_line_t *next;            /* Next in LRU list */
};

/**
 * @brief Cache set
 * 
 * Contains multiple cache lines (ways) for set-associative caches.
 * For direct-mapped, num_ways = 1. For fully-assoc, there's only 1 set.
 */
struct cache_set_s {
    uint32_t num_ways;             /* Number of ways in this set */
    cache_line_t *lines;           /* Array of cache lines */
    
    /* LRU tracking */
    cache_line_t *lru_head;        /* Most recently used */
    cache_line_t *lru_tail;        /* Least recently used (victim) */
};

/**
 * @brief Unified cache structure (handles all associativities)
 * 
 * Single cache implementation that works for:
 * - Task 1: Direct-mapped, 4-byte blocks
 * - Task 2: Direct-mapped with variable block sizes
 * - Task 3: All associativity modes (direct-mapped, 2-way, 4-way, fully-assoc)
 * - Task 4: Used as L1 and L2 in 2-level cache
 */
struct cache_s {
    /* Configuration */
    uint32_t size;                 /* Total cache size in bytes */
    uint32_t block_size;           /* Block size in bytes */
    assoc_type_t associativity;    /* Associativity type */
    uint32_t num_sets;             /* Number of sets */
    uint32_t ways_per_set;         /* Ways per set */
    
    /* Bit field calculations */
    uint32_t offset_bits;          /* Bits for offset within block */
    uint32_t index_bits;           /* Bits for set index */
    uint32_t tag_bits;             /* Bits for tag */
    
    /* Storage */
    cache_set_t *sets;             /* Array of cache sets */
    
    /* Statistics */
    uint64_t accesses;             /* Total accesses */
    uint64_t hits;                 /* Total hits */
    uint64_t misses;               /* Total misses */
    uint64_t reads;                /* Total reads */
    uint64_t read_hits;            /* Read hits */
    uint64_t writes;               /* Total writes */
    uint64_t write_hits;           /* Write hits */
};

/**
 * @brief Cache configuration
 */
struct cache_config_s {
    uint32_t size;                 /* Cache size in bytes */
    uint32_t block_size;           /* Block size in bytes */
    assoc_type_t associativity;    /* Associativity type */
};

/**
 * @brief Multi-level cache for Task 4
 * 
 * Supports exactly 2 cache levels (L1, L2) for Task 4.
 */
struct multilevel_cache_s {
    uint32_t num_levels;                      /* Number of cache levels */
    cache_t *levels[MAX_CACHE_LEVELS];        /* Array of cache pointers */
    
    /* Per-level access tracking (beyond what cache_t tracks) */
    uint64_t level_accesses[MAX_CACHE_LEVELS]; /* Accesses to each level */
};

/* ============================================================================
 * TLB Structures
 * ============================================================================ */

/**
 * @brief TLB entry
 */
struct tlb_entry_s {
    bool valid;                    /* Valid bit */
    bool dirty;                    /* Dirty bit */
    uint32_t vpn;                  /* Virtual page number (tag) */
    uint32_t ppn;                  /* Physical page number */
    
    /* LRU tracking */
    tlb_entry_t *prev;
    tlb_entry_t *next;
};

/**
 * @brief TLB set (for set-associative TLB)
 */
struct tlb_set_s {
    uint32_t num_ways;
    tlb_entry_t *entries;
    
    /* LRU tracking */
    tlb_entry_t *lru_head;
    tlb_entry_t *lru_tail;
};

/**
 * @brief TLB structure
 */
struct tlb_s {
    uint32_t num_entries;          /* Total number of TLB entries */
    assoc_type_t associativity;    /* Associativity type */
    uint32_t num_sets;             /* Number of sets */
    uint32_t ways_per_set;         /* Ways per set */
    
    /* Bit field calculations */
    uint32_t offset_bits;          /* Always 12 (4KB pages) */
    uint32_t index_bits;           /* Bits for set index */
    uint32_t tag_bits;             /* Bits for VPN tag */
    
    /* Storage */
    tlb_set_t *sets;
    
    /* Statistics */
    uint64_t accesses;
    uint64_t hits;
    uint64_t misses;
};

/**
 * @brief TLB configuration
 */
struct tlb_config_s {
    uint32_t num_entries;          /* Number of TLB entries */
    assoc_type_t associativity;    /* Associativity type */
};

/* ============================================================================
 * Page Table Structures
 * ============================================================================ */

/**
 * @brief Page table entry
 */
struct page_table_entry_s {
    bool present;                  /* Present bit (in memory or not) */
    bool dirty;                    /* Dirty bit */
    uint32_t ppn;                  /* Physical page number */
};

/**
 * @brief Physical page (frame)
 * 
 * Used for free list and LRU tracking of allocated pages.
 */
struct page_s {
    uint32_t frame_id;             /* Physical frame number */
    pte_t *pte;                    /* Pointer to corresponding PTE */
    uint8_t data[PAGE_SIZE];       /* Page data */
    
    /* Linked list pointers */
    page_t *next;
    page_t *prev;
};

/* ============================================================================
 * Simulation Configuration
 * ============================================================================ */

/**
 * @brief Complete simulator configuration
 * 
 * Holds all configuration parameters parsed from command line.
 * Supports Tasks 1-4 with extensible multi-level cache design.
 */
struct sim_config_s {
    int task;                      /* Detected task number (1, 2, 3, or 4) */
    
    /* Single-level cache (Tasks 1-3) */
    cache_config_t cache;
    
    /* Multi-level cache (Task 4+) - Extensible design */
    cache_config_t levels[MAX_CACHE_LEVELS];  /* L1, L2, L3, ... */
    uint32_t num_levels;           /* Number of cache levels (0 for single, 2+ for multi) */
    
    /* TLB configuration */
    tlb_config_t tlb;
    
    /* File and options */
    char *trace_file;              /* Trace file path */
    bool verbose;                  /* Verbose output mode */
};

#endif /* TYPES_H */

