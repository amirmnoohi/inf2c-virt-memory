/**
 * @file ll.c
 * @brief Doubly-linked list implementation
 * @author Amir Noohi
 * @copyright Copyright (c) 2025 Amir Noohi. All rights reserved.
 * 
 * PROVIDED TO STUDENTS - Reference implementation
 */

#include <stddef.h>
#include "ll.h"
#include "types.h"

void ll_insert_head(page_t **head, page_t *page) {
    if (!page) return;
    
    page->next = *head;
    page->prev = NULL;
    
    if (*head) {
        (*head)->prev = page;
    }
    
    *head = page;
}

page_t* ll_remove_head(page_t **head) {
    if (!*head) return NULL;
    
    page_t *removed = *head;
    *head = removed->next;
    
    if (*head) {
        (*head)->prev = NULL;
    }
    
    removed->next = NULL;
    removed->prev = NULL;
    
    return removed;
}

void ll_remove_page(page_t **head, page_t *page) {
    if (!page || !*head) return;
    
    /* If removing the head */
    if (page == *head) {
        *head = page->next;
        if (*head) {
            (*head)->prev = NULL;
        }
    } else {
        /* Remove from middle or tail */
        if (page->prev) {
            page->prev->next = page->next;
        }
        if (page->next) {
            page->next->prev = page->prev;
        }
    }
    
    page->next = NULL;
    page->prev = NULL;
}

void ll_move_to_head(page_t **head, page_t *page) {
    if (!page || !*head || page == *head) return;
    
    /* Remove from current position */
    if (page->prev) {
        page->prev->next = page->next;
    }
    if (page->next) {
        page->next->prev = page->prev;
    }
    
    /* Insert at head */
    page->next = *head;
    page->prev = NULL;
    (*head)->prev = page;
    *head = page;
}

page_t* ll_get_tail(page_t *head) {
    if (!head) return NULL;
    
    page_t *tail = head;
    while (tail->next) {
        tail = tail->next;
    }
    
    return tail;
}

