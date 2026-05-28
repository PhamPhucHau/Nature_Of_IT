/*
 * lru.h - LRU (Least Recently Used) Replacement Policy Header
 * 
 * Implements the LRU replacement policy for buffer pool eviction.
 * When the buffer pool is full, the least recently used page is evicted.
 * 
 * Key Concepts:
 * - LRU List: Doubly linked list of pages ordered by access time
 * - Head: Most recently used page
 * - Tail: Least recently used page
 * - Move to Front: Update page access time
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef LRU_H
#define LRU_H

#include "page.h"
#include <stdint.h>

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * LRUNode - Node in LRU list
 * 
 * Represents a page in the LRU list.
 * 
 * Fields:
 *   - page: Pointer to page
 *   - prev: Previous node (more recently used)
 *   - next: Next node (less recently used)
 */
typedef struct LRUNode {
    Page *page;                     /* Pointer to page */
    struct LRUNode *prev;           /* Previous node */
    struct LRUNode *next;           /* Next node */
} LRUNode;

/*
 * LRUList - LRU replacement policy
 * 
 * Maintains a doubly linked list of pages ordered by access time.
 * Head is most recently used, tail is least recently used.
 * 
 * Fields:
 *   - head: Most recently used page
 *   - tail: Least recently used page
 *   - size: Number of pages in list
 */
typedef struct {
    LRUNode *head;                  /* Most recently used */
    LRUNode *tail;                  /* Least recently used */
    uint32_t size;                  /* Number of pages */
} LRUList;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * lru_create - Create a new LRU list
 * 
 * Allocates and initializes a new LRU list.
 * 
 * Returns:
 *   Pointer to new LRUList, or NULL on error
 * 
 * Example:
 *   LRUList *lru = lru_create();
 *   if (!lru) {
 *       fprintf(stderr, "Failed to create LRU list\n");
 *       return 1;
 *   }
 */
LRUList* lru_create(void);

/*
 * lru_destroy - Free LRU list memory
 * 
 * Frees all memory allocated for the LRU list.
 * Note: Does not free pages themselves.
 * 
 * Parameters:
 *   lru: Pointer to LRUList
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   lru_destroy(lru);
 */
void lru_destroy(LRUList *lru);

/*
 * lru_add_page - Add page to LRU list
 * 
 * Adds a page to the front of the LRU list (most recently used).
 * 
 * Parameters:
 *   lru: Pointer to LRUList
 *   page: Pointer to Page
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   if (lru_add_page(lru, page) != 0) {
 *       fprintf(stderr, "Failed to add page to LRU\n");
 *   }
 */
int lru_add_page(LRUList *lru, Page *page);

/*
 * lru_remove_page - Remove page from LRU list
 * 
 * Removes a page from the LRU list.
 * 
 * Parameters:
 *   lru: Pointer to LRUList
 *   page: Pointer to Page
 * 
 * Returns:
 *   0 on success, -1 if page not found
 * 
 * Example:
 *   if (lru_remove_page(lru, page) != 0) {
 *       fprintf(stderr, "Page not in LRU list\n");
 *   }
 */
int lru_remove_page(LRUList *lru, Page *page);

/*
 * lru_move_to_front - Move page to front of LRU list
 * 
 * Moves a page to the front of the LRU list (most recently used).
 * Used when a page is accessed.
 * 
 * Parameters:
 *   lru: Pointer to LRUList
 *   page: Pointer to Page
 * 
 * Returns:
 *   0 on success, -1 if page not found
 * 
 * Example:
 *   if (lru_move_to_front(lru, page) != 0) {
 *       fprintf(stderr, "Failed to move page\n");
 *   }
 */
int lru_move_to_front(LRUList *lru, Page *page);

/*
 * lru_get_lru_page - Get least recently used page
 * 
 * Returns the least recently used page (at tail of list).
 * This is the page that should be evicted.
 * 
 * Parameters:
 *   lru: Pointer to LRUList
 * 
 * Returns:
 *   Pointer to least recently used Page, or NULL if list is empty
 * 
 * Example:
 *   Page *victim = lru_get_lru_page(lru);
 *   if (victim) {
 *       printf("Evicting page %u\n", victim->page_id);
 *   }
 */
Page* lru_get_lru_page(LRUList *lru);

/*
 * lru_get_size - Get number of pages in LRU list
 * 
 * Returns the number of pages currently in the LRU list.
 * 
 * Parameters:
 *   lru: Pointer to LRUList
 * 
 * Returns:
 *   Number of pages
 * 
 * Example:
 *   uint32_t size = lru_get_size(lru);
 *   printf("LRU list size: %u\n", size);
 */
uint32_t lru_get_size(LRUList *lru);

/*
 * lru_is_empty - Check if LRU list is empty
 * 
 * Determines if the LRU list is empty.
 * 
 * Parameters:
 *   lru: Pointer to LRUList
 * 
 * Returns:
 *   1 if empty, 0 if not empty
 * 
 * Example:
 *   if (lru_is_empty(lru)) {
 *       printf("LRU list is empty\n");
 *   }
 */
int lru_is_empty(LRUList *lru);

/*
 * lru_print - Print LRU list
 * 
 * Prints the LRU list in order from most to least recently used.
 * Useful for debugging.
 * 
 * Parameters:
 *   lru: Pointer to LRUList
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   lru_print(lru);
 */
void lru_print(LRUList *lru);

#endif // LRU_H
