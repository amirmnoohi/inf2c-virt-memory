/**
 * @file ll.h
 * @brief Doubly-linked list utilities for page management
 * 
 * Provides generic doubly-linked list operations used for:
 * - Free page list management
 * - LRU page tracking
 * - Cache line LRU tracking
 * - TLB entry LRU tracking
 * 
 * PROVIDED TO STUDENTS - Do not modify
 */

#ifndef LL_H
#define LL_H

#include "types.h"

/**
 * @brief Insert a page at the head of the list
 * 
 * @param head Pointer to the head pointer of the list
 * @param page Page to insert
 */
void ll_insert_head(page_t **head, page_t *page);

/**
 * @brief Remove and return the page at the head of the list
 * 
 * @param head Pointer to the head pointer of the list
 * @return The removed page, or NULL if list is empty
 */
page_t* ll_remove_head(page_t **head);

/**
 * @brief Remove a specific page from the list
 * 
 * @param head Pointer to the head pointer of the list
 * @param page Page to remove
 */
void ll_remove_page(page_t **head, page_t *page);

/**
 * @brief Move a page to the head of the list (for LRU)
 * 
 * Efficiently moves a page from its current position to the head.
 * Used to mark a page as "most recently used".
 * 
 * @param head Pointer to the head pointer of the list
 * @param page Page to move
 */
void ll_move_to_head(page_t **head, page_t *page);

/**
 * @brief Get the tail of the list (LRU victim)
 * 
 * Does not remove the page from the list, just returns a pointer to it.
 * 
 * @param head Head of the list
 * @return Pointer to the tail page, or NULL if list is empty
 */
page_t* ll_get_tail(page_t *head);

#endif /* LL_H */

