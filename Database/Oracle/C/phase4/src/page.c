/*
 * page.c - Page Implementation
 * 
 * Implements page creation, destruction, and utility functions.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "page.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * PAGE CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * page_create - Create a new page
 * 
 * Allocates memory for a new page and initializes it.
 */
Page* page_create(uint32_t page_id) {
    /* Allocate memory for page */
    Page *page = (Page *)malloc(sizeof(Page));
    if (!page) {
        fprintf(stderr, "ERROR: Failed to allocate memory for page\n");
        return NULL;
    }
    
    /* Initialize page fields */
    page->page_id = page_id;
    page->is_dirty = 0;
    page->pin_count = 0;
    page->access_time = time(NULL);
    
    /* Initialize data to zeros */
    memset(page->data, 0, PAGE_SIZE);
    
    return page;
}

/*
 * page_destroy - Free page memory
 * 
 * Frees the memory allocated for a page.
 */
void page_destroy(Page *page) {
    if (page) {
        free(page);
    }
}

/* ============================================================================
 * PAGE PIN/UNPIN OPERATIONS
 * ============================================================================ */

/*
 * page_pin - Increment pin count
 * 
 * Increments the pin count to prevent page eviction.
 */
uint32_t page_pin(Page *page) {
    if (!page) return 0;
    return ++page->pin_count;
}

/*
 * page_unpin - Decrement pin count
 * 
 * Decrements the pin count. Page can be evicted when pin_count reaches 0.
 */
uint32_t page_unpin(Page *page) {
    if (!page || page->pin_count == 0) return 0;
    return --page->pin_count;
}

/* ============================================================================
 * PAGE DIRTY FLAG OPERATIONS
 * ============================================================================ */

/*
 * page_mark_dirty - Mark page as dirty
 * 
 * Marks the page as modified (dirty).
 */
void page_mark_dirty(Page *page) {
    if (page) {
        page->is_dirty = 1;
    }
}

/*
 * page_mark_clean - Mark page as clean
 * 
 * Marks the page as unmodified (clean).
 */
void page_mark_clean(Page *page) {
    if (page) {
        page->is_dirty = 0;
    }
}

/*
 * page_is_dirty - Check if page is dirty
 * 
 * Determines if the page has been modified.
 */
int page_is_dirty(Page *page) {
    if (!page) return 0;
    return page->is_dirty;
}

/* ============================================================================
 * PAGE PIN STATUS OPERATIONS
 * ============================================================================ */

/*
 * page_is_pinned - Check if page is pinned
 * 
 * Determines if the page is currently pinned (cannot be evicted).
 */
int page_is_pinned(Page *page) {
    if (!page) return 0;
    return page->pin_count > 0;
}

/*
 * page_get_pin_count - Get pin count
 * 
 * Returns the current pin count.
 */
uint32_t page_get_pin_count(Page *page) {
    if (!page) return 0;
    return page->pin_count;
}

/* ============================================================================
 * PAGE ACCESS TIME OPERATIONS
 * ============================================================================ */

/*
 * page_update_access_time - Update access time
 * 
 * Updates the last access time to current time.
 * Used for LRU replacement policy.
 */
void page_update_access_time(Page *page) {
    if (page) {
        page->access_time = time(NULL);
    }
}

/*
 * page_get_access_time - Get access time
 * 
 * Returns the last access time.
 */
time_t page_get_access_time(Page *page) {
    if (!page) return 0;
    return page->access_time;
}

/* ============================================================================
 * PAGE PRINTING
 * ============================================================================ */

/*
 * page_print - Print page information
 * 
 * Prints page information in human-readable format.
 */
void page_print(Page *page) {
    if (!page) {
        printf("NULL page\n");
        return;
    }
    
    printf("Page %u:\n", page->page_id);
    printf("  Dirty: %s\n", page->is_dirty ? "yes" : "no");
    printf("  Pin Count: %u\n", page->pin_count);
    printf("  Access Time: %ld\n", page->access_time);
    printf("  Data Size: %d bytes\n", PAGE_SIZE);
}
