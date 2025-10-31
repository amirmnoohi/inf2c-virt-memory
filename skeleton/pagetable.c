/**
 * @file pagetable.c (STUDENT VERSION)
 * @brief Page table implementation - TO BE COMPLETED BY STUDENTS
 * 
 * Implement a linear page table with:
 * - 2^14 entries (26-bit virtual address space)
 * - 256 physical pages (1MB physical memory)
 * - Free page list management
 * - LRU page replacement
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pagetable.h"
#include "ll.h"
#include "types.h"

/* ============================================================================
 * Global State - STUDENTS MUST USE THESE
 * ============================================================================ */

/* Page table array */
static pte_t page_table[PAGE_TABLE_ENTRIES];

/* Free page list */
static page_t *free_page_list = NULL;

/* Used page list (for LRU tracking) */
static page_t *used_page_list = NULL;

/* Frame table for quick lookup */
static page_t *frame_table[NUM_PHYSICAL_PAGES];

/* Statistics */
static uint64_t pt_accesses = 0;
static uint64_t page_faults = 0;
static uint64_t page_faults_dirty = 0;

/* ============================================================================
 * Dummy I/O Functions - PROVIDED (do not modify)
 * ============================================================================ */

void read_page_from_disk(uint8_t *page_data, uint32_t disk_block) {
    (void)page_data;
    (void)disk_block;
    /* Simulated - no actual I/O */
}

void write_page_to_disk(const uint8_t *page_data) {
    (void)page_data;
    /* Simulated - no actual I/O */
}

/* ============================================================================
 * Initialization - TO BE IMPLEMENTED
 * ============================================================================ */

void pagetable_init(void) {
    // TODO: Initialize page table entries
    //       - Set all present bits to false
    //       - Set all dirty bits to false
    //       - Set all PPNs to 0
    
    // TODO: Initialize frame table
    //       - Set all entries to NULL
    
    // TODO: Create free page list
    //       - Create NUM_PHYSICAL_PAGES page structures
    //       - Insert all pages into free_page_list using ll_insert_head()
    //       - Store pages in frame_table for quick lookup
    
    // TODO: Initialize used_page_list to NULL
    
    // TODO: Reset statistics
    
    // Hints:
    // - Use a loop to initialize page_table entries
    // - Use calloc() to create page structures
    // - Initialize page->frame_id to the physical frame number
    // - Use ll_insert_head() from ll.c to build free list
}

/* ============================================================================
 * Page Table Lookup - TO BE IMPLEMENTED
 * ============================================================================ */

pt_result_t pagetable_lookup(uint32_t vpn, uint32_t *ppn, bool *dirty) {
    // TODO: Increment pt_accesses
    
    // TODO: Check if page_table[vpn].present is true
    //       If present:
    //         - Set *ppn to page_table[vpn].ppn
    //         - Set *dirty to page_table[vpn].dirty
    //         - Update LRU: move page to head of used_page_list
    //           (use frame_table[ppn] to get page, then ll_move_to_head())
    //         - Return PT_HIT
    
    // TODO: If not present, return PT_MISS
    
    (void)vpn;
    (void)ppn;
    (void)dirty;
    return PT_MISS;  // STUDENT: Replace with actual implementation
}

/* ============================================================================
 * Page Fault Handler - TO BE IMPLEMENTED
 * ============================================================================ */

uint32_t pagetable_handle_fault(uint32_t vpn) {
    // TODO: Increment page_faults
    
    // TODO: Try to get a free page
    //       - Use ll_remove_head(&free_page_list)
    //       - If NULL, no free pages available
    
    // TODO: If no free pages:
    //       - Get victim page using ll_get_tail(used_page_list)
    //       - Remove victim from used_page_list
    //       - If victim->pte->dirty is true:
    //         * Increment page_faults_dirty
    //         * Call write_page_to_disk(victim->data)
    //         * Set victim->pte->dirty to false
    //       - Mark victim->pte->present as false
    
    // TODO: Read new page from disk
    //       - Call read_page_from_disk(page->data, vpn)
    
    // TODO: Update page table entry
    //       - Set page_table[vpn].present = true
    //       - Set page_table[vpn].dirty = false
    //       - Set page_table[vpn].ppn = page->frame_id
    
    // TODO: Link page to PTE
    //       - Set page->pte = &page_table[vpn]
    
    // TODO: Add page to head of used_page_list
    //       - Use ll_insert_head(&used_page_list, page)
    
    // TODO: Return page->frame_id
    
    // Hints:
    // - LRU victim is at the tail of used_page_list
    // - Use ll.h functions for list management
    // - Don't forget to update statistics
    
    (void)vpn;
    return 0;  // STUDENT: Replace with actual implementation
}

/* ============================================================================
 * Dirty Bit Management - TO BE IMPLEMENTED
 * ============================================================================ */

void pagetable_set_dirty(uint32_t vpn) {
    // TODO: If page_table[vpn].present is true,
    //       set page_table[vpn].dirty = true
    
    (void)vpn;
}

/* ============================================================================
 * Statistics - PROVIDED
 * ============================================================================ */

void pagetable_print_stats(void) {
    printf("\n* Page Table Statistics *\n");
    printf("total accesses: %llu\n", (unsigned long long)pt_accesses);
    printf("page faults: %llu\n", (unsigned long long)page_faults);
    printf("page faults with dirty bit: %llu\n", (unsigned long long)page_faults_dirty);
}

void pagetable_print_entries(void) {
    printf("\nPage Table Entries (Present-Bit Dirty-Bit VPN PPN)\n");
    
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        if (page_table[i].present) {
            printf("%d %d 0x%05x 0x%05x\n",
                   1,
                   page_table[i].dirty ? 1 : 0,
                   i,
                   page_table[i].ppn);
        }
    }
}

/* ============================================================================
 * Cleanup - TO BE IMPLEMENTED
 * ============================================================================ */

void pagetable_destroy(void) {
    // TODO: Free all page structures
    //       - Loop through frame_table
    //       - Free each page if not NULL
    //       - Set to NULL after freeing
    
    // TODO: Reset list pointers to NULL
}

