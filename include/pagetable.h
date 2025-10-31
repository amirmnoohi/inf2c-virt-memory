/**
 * @file pagetable.h
 * @brief Page table interface
 * 
 * Linear page table with 2^14 entries (26-bit virtual address space).
 * Manages 256 physical pages (1MB physical memory) with LRU eviction.
 * 
 * STUDENTS IMPLEMENT THIS MODULE
 */

#ifndef PAGETABLE_H
#define PAGETABLE_H

#include "types.h"

/**
 * @brief Initialize page table system
 * 
 * Creates:
 * - Linear page table with PAGE_TABLE_ENTRIES entries
 * - Free page list with NUM_PHYSICAL_PAGES pages
 * - Used page list for LRU tracking
 * 
 * Initially all pages are in the free list.
 */
void pagetable_init(void);

/**
 * @brief Look up VPN in page table
 * 
 * Checks if the page is present in memory. If present (hit),
 * returns the PPN and updates LRU. If not present (page fault),
 * returns -1.
 * 
 * @param vpn Virtual page number
 * @param[out] ppn Physical page number (if present)
 * @param[out] dirty Dirty bit value (if present)
 * @return PT_HIT or PT_MISS
 */
pt_result_t pagetable_lookup(uint32_t vpn, uint32_t *ppn, bool *dirty);

/**
 * @brief Handle page fault
 * 
 * Allocates a physical page for the given VPN. If free pages
 * are available, uses one. Otherwise, evicts a page using LRU.
 * 
 * Algorithm:
 * 1. Try to get a free page
 * 2. If no free pages, evict LRU page:
 *    a. If dirty, write to disk (dummy function)
 *    b. Mark PTE as not present
 * 3. Read new page from disk (dummy function)
 * 4. Update PTE: present=true, PPN=allocated frame
 * 5. Add page to used list (head = MRU)
 * 6. Return allocated PPN
 * 
 * @param vpn Virtual page number
 * @return Physical page number allocated
 */
uint32_t pagetable_handle_fault(uint32_t vpn);

/**
 * @brief Set dirty bit for a page
 * 
 * Marks the page as modified (needs writeback on eviction).
 * 
 * @param vpn Virtual page number
 */
void pagetable_set_dirty(uint32_t vpn);

/**
 * @brief Print page table statistics
 * 
 * Format:
 *   * Page Table Statistics *
 *   total accesses: X
 *   page faults: X
 *   page faults with dirty bit: X
 */
void pagetable_print_stats(void);

/**
 * @brief Print page table entries (verbose mode)
 * 
 * Format:
 *   Page Table Entries (Present-Bit Dirty-Bit VPN PPN)
 *   1 0 0x00001 0x00000
 *   1 1 0x00002 0x00001
 *   ...
 * 
 * Only prints entries where present bit is set.
 */
void pagetable_print_entries(void);

/**
 * @brief Destroy page table and free resources
 */
void pagetable_destroy(void);

/* ============================================================================
 * Dummy I/O functions (PROVIDED - do not modify)
 * ============================================================================ */

/**
 * @brief Dummy function to simulate reading a page from disk
 * 
 * In a real system, this would read 4KB from disk into memory.
 * In the simulator, this is a no-op.
 * 
 * @param page_data Buffer to read into
 * @param disk_block Disk block number
 */
void read_page_from_disk(uint8_t *page_data, uint32_t disk_block);

/**
 * @brief Dummy function to simulate writing a page to disk
 * 
 * In a real system, this would write 4KB from memory to disk.
 * In the simulator, this is a no-op.
 * 
 * @param page_data Buffer to write from
 */
void write_page_to_disk(const uint8_t *page_data);

#endif /* PAGETABLE_H */

