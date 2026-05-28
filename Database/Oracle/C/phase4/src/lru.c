/*
 * lru.c - LRU Replacement Policy Implementation
 * 
 * Implements the LRU (Least Recently Used) replacement policy
 * using a doubly linked list.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "lru.h"
#include <stdlib.h>
#include <stdio.h>

/* ============================================================================
 * LRU LIST CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * lru_create - Create a new LRU list
 * 
 * Allocates and initializes a new LRU list.
 */
LRUList* lru_create(void) {
    /* Allocate memory for LRU list */
    LRUList *lru = (LRUList *)malloc(sizeof(LRUList));
    if (!lru) {
        fprintf(stderr, "ERROR: Failed to allocate memory for LRU list\n");
        return NULL;
    }
    
    /* Initialize LRU list */
    lru->head = NULL;
    lru->tail = NULL;
    lru->size = 0;
    
    return lru;
}

/*
 * lru_destroy - Free LRU list memory
 * 
 * Frees all memory allocated for the LRU list.
 * Note: Does not free pages themselves.
 */
void lru_destroy(LRUList *lru) {
    if (!lru) return;
    
    /* Free all nodes */
    LRUNode *current = lru->head;
    while (current) {
        LRUNode *next = current->next;
        free(current);
        current = next;
    }
    
    free(lru);
}

/* ============================================================================
 * LRU PAGE OPERATIONS
 * ============================================================================ */

/*
 * lru_add_page - Add page to LRU list
 * 
 * Adds a page to the front of the LRU list (most recently used).
 */
int lru_add_page(LRUList *lru, Page *page) {
    if (!lru || !page) return -1;
    
    /* Allocate new node */
    LRUNode *node = (LRUNode *)malloc(sizeof(LRUNode));
    if (!node) {
        fprintf(stderr, "ERROR: Failed to allocate memory for LRU node\n");
        return -1;
    }
    
    /* Initialize node */
    node->page = page;
    node->prev = NULL;
    node->next = lru->head;
    
    /* Update list */
    if (lru->head) {
        lru->head->prev = node;
    } else {
        lru->tail = node;
    }
    lru->head = node;
    lru->size++;
    
    return 0;
}

/*
 * lru_remove_page - Remove page from LRU list
 * 
 * Removes a page from the LRU list.
 */
int lru_remove_page(LRUList *lru, Page *page) {
    if (!lru || !page) return -1;
    
    /* Find node with matching page */
    LRUNode *current = lru->head;
    while (current) {
        if (current->page->page_id == page->page_id) {
            /* Found the node, remove it */
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                lru->head = current->next;
            }
            
            if (current->next) {
                current->next->prev = current->prev;
            } else {
                lru->tail = current->prev;
            }
            
            free(current);
            lru->size--;
            return 0;
        }
        current = current->next;
    }
    
    return -1;  /* Page not found */
}

/*
 * lru_move_to_front - Move page to front of LRU list
 * 
 * Moves a page to the front of the LRU list (most recently used).
 * Used when a page is accessed.
 */
int lru_move_to_front(LRUList *lru, Page *page) {
    if (!lru || !page) return -1;
    
    /* Find node with matching page */
    LRUNode *current = lru->head;
    while (current) {
        if (current->page->page_id == page->page_id) {
            /* If already at head, nothing to do */
            if (current == lru->head) {
                return 0;
            }
            
            /* Remove from current position */
            if (current->prev) {
                current->prev->next = current->next;
            }
            if (current->next) {
                current->next->prev = current->prev;
            } else {
                lru->tail = current->prev;
            }
            
            /* Move to front */
            current->prev = NULL;
            current->next = lru->head;
            if (lru->head) {
                lru->head->prev = current;
            }
            lru->head = current;
            
            return 0;
        }
        current = current->next;
    }
    
    return -1;  /* Page not found */
}

/* ============================================================================
 * LRU QUERY OPERATIONS
 * ============================================================================ */

/*
 * lru_get_lru_page - Get least recently used page
 * 
 * Returns the least recently used page (at tail of list).
 * This is the page that should be evicted.
 */
Page* lru_get_lru_page(LRUList *lru) {
    if (!lru || !lru->tail) {
        return NULL;
    }
    return lru->tail->page;
}

/*
 * lru_get_size - Get number of pages in LRU list
 * 
 * Returns the number of pages currently in the LRU list.
 */
uint32_t lru_get_size(LRUList *lru) {
    if (!lru) return 0;
    return lru->size;
}

/*
 * lru_is_empty - Check if LRU list is empty
 * 
 * Determines if the LRU list is empty.
 */
int lru_is_empty(LRUList *lru) {
    if (!lru) return 1;
    return lru->size == 0;
}

/* ============================================================================
 * LRU PRINTING
 * ============================================================================ */

/*
 * lru_print - Print LRU list
 * 
 * Prints the LRU list in order from most to least recently used.
 */
void lru_print(LRUList *lru) {
    if (!lru) {
        printf("NULL LRU list\n");
        return;
    }
    
    printf("LRU List (size: %u):\n", lru->size);
    printf("  Most Recent -> Least Recent\n");
    
    LRUNode *current = lru->head;
    int index = 0;
    while (current) {
        printf("  %d: Page %u (pin_count=%u, dirty=%d)\n",
               index++,
               current->page->page_id,
               current->page->pin_count,
               current->page->is_dirty);
        current = current->next;
    }
}
