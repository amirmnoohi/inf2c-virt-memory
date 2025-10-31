/**
 * @file tlb.h
 * @brief Translation Lookaside Buffer (TLB) interface
 * 
 * TLB caches virtual-to-physical page number translations.
 * Similar structure to cache but operates on page numbers (4KB pages).
 */

#ifndef TLB_H
#define TLB_H

#include "types.h"

/**
 * @brief Initialize TLB with given configuration
 * 
 * Creates TLB with specified number of entries and associativity.
 * Similar to cache_init() but for page translations.
 * 
 * @param config TLB configuration (num_entries, associativity)
 * @return Pointer to initialized TLB, or NULL on error
 */
tlb_t* tlb_init(tlb_config_t config);

/**
 * @brief Look up virtual page number in TLB
 * 
 * Searches TLB for the given VPN. If found (hit), returns the PPN
 * and updates LRU. If not found (miss), returns -1.
 * 
 * @param tlb TLB instance
 * @param vpn Virtual page number to look up
 * @param[out] ppn Physical page number (if hit)
 * @param[out] dirty Dirty bit value (if hit)
 * @return TLB_HIT or TLB_MISS
 */
tlb_result_t tlb_lookup(tlb_t *tlb, uint32_t vpn, uint32_t *ppn, bool *dirty);

/**
 * @brief Insert or update TLB entry
 * 
 * Inserts a VPN->PPN mapping into the TLB. If TLB is full,
 * evicts an entry using LRU policy.
 * 
 * @param tlb TLB instance
 * @param vpn Virtual page number
 * @param ppn Physical page number
 */
void tlb_insert(tlb_t *tlb, uint32_t vpn, uint32_t ppn);

/**
 * @brief Set dirty bit for a TLB entry
 * 
 * Marks the TLB entry for the given VPN as dirty.
 * Also updates the corresponding page table entry.
 * 
 * @param tlb TLB instance
 * @param vpn Virtual page number
 */
void tlb_set_dirty(tlb_t *tlb, uint32_t vpn);

/**
 * @brief Print TLB statistics
 * 
 * Format:
 *   * TLB Statistics *
 *   total accesses: X
 *   hits: X
 *   misses: X
 * 
 * @param tlb TLB instance
 */
void tlb_print_stats(const tlb_t *tlb);

/**
 * @brief Print TLB entries (verbose mode)
 * 
 * Format:
 *   TLB Entries (Valid-Bit Dirty-Bit VPN PPN)
 *   1 0 0x00001 0x00000
 *   0 0 - -
 *   ...
 * 
 * @param tlb TLB instance
 */
void tlb_print_entries(const tlb_t *tlb);

/**
 * @brief Destroy TLB and free resources
 * 
 * @param tlb TLB to destroy
 */
void tlb_destroy(tlb_t *tlb);

#endif /* TLB_H */

