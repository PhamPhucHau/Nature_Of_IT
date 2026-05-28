/*
 * page.h - Page Structure Header
 * 
 * Defines the page structure used by the buffer pool.
 * A page is the basic unit of disk I/O in a database.
 * 
 * Key Concepts:
 * - Page: Fixed-size block of data (typically 4KB)
 * - Page ID: Unique identifier for a page
 * - Dirty Flag: Indicates if page has been modified
 * - Pin Count: Reference count for the page
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Standard page size (4KB) */
#define PAGE_SIZE 4096

/* Invalid page ID */
#define INVALID_PAGE_ID ((uint32_t)-1)

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * Page - Represents a single page in the buffer pool
 * 
 * A page is the basic unit of disk I/O. It contains:
 * - Page ID: Unique identifier
 * - Data: Actual page content (4KB)
 * - Dirty flag: Whether page has been modified
 * - Pin count: Reference count (prevents eviction)
 * - Access time: For LRU replacement
 * 
 * Fields:
 *   - page_id: Unique page identifier
 *   - data: Page data (4KB)
 *   - is_dirty: 1 if modified, 0 if clean
 *   - pin_count: Number of active references
 *   - access_time: Last access timestamp
 */
typedef struct {
    uint32_t page_id;               /* Unique page identifier */
    char data[PAGE_SIZE];           /* Page data (4KB) */
    int is_dirty;                   /* 1 if dirty, 0 if clean */
    uint32_t pin_count;             /* Reference count */
    time_t access_time;             /* Last access time */
} Page;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * page_create - Create a new page
 * 
 * Allocates and initializes a new page with the given page ID.
 * 
 * Parameters:
 *   page_id: Unique page identifier
 * 
 * Returns:
 *   Pointer to new Page, or NULL on error
 * 
 * Example:
 *   Page *page = page_create(1);
 *   if (!page) {
 *       fprintf(stderr, "Failed to create page\n");
 *       return 1;
 *   }
 */
Page* page_create(uint32_t page_id);

/*
 * page_destroy - Free page memory
 * 
 * Frees the memory allocated for a page.
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   page_destroy(page);
 */
void page_destroy(Page *page);

/*
 * page_pin - Increment pin count
 * 
 * Increments the pin count to prevent page eviction.
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   New pin count
 * 
 * Example:
 *   page_pin(page);  // pin_count becomes 1
 */
uint32_t page_pin(Page *page);

/*
 * page_unpin - Decrement pin count
 * 
 * Decrements the pin count. Page can be evicted when pin_count reaches 0.
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   New pin count
 * 
 * Example:
 *   page_unpin(page);  // pin_count becomes 0
 */
uint32_t page_unpin(Page *page);

/*
 * page_mark_dirty - Mark page as dirty
 * 
 * Marks the page as modified (dirty).
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   page_mark_dirty(page);
 */
void page_mark_dirty(Page *page);

/*
 * page_mark_clean - Mark page as clean
 * 
 * Marks the page as unmodified (clean).
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   page_mark_clean(page);
 */
void page_mark_clean(Page *page);

/*
 * page_is_dirty - Check if page is dirty
 * 
 * Determines if the page has been modified.
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   1 if dirty, 0 if clean
 * 
 * Example:
 *   if (page_is_dirty(page)) {
 *       printf("Page needs to be flushed\n");
 *   }
 */
int page_is_dirty(Page *page);

/*
 * page_is_pinned - Check if page is pinned
 * 
 * Determines if the page is currently pinned (cannot be evicted).
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   1 if pinned, 0 if not pinned
 * 
 * Example:
 *   if (page_is_pinned(page)) {
 *       printf("Page cannot be evicted\n");
 *   }
 */
int page_is_pinned(Page *page);

/*
 * page_get_pin_count - Get pin count
 * 
 * Returns the current pin count.
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   Current pin count
 * 
 * Example:
 *   uint32_t count = page_get_pin_count(page);
 *   printf("Pin count: %u\n", count);
 */
uint32_t page_get_pin_count(Page *page);

/*
 * page_update_access_time - Update access time
 * 
 * Updates the last access time to current time.
 * Used for LRU replacement policy.
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   page_update_access_time(page);
 */
void page_update_access_time(Page *page);

/*
 * page_get_access_time - Get access time
 * 
 * Returns the last access time.
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   Last access time
 * 
 * Example:
 *   time_t last_access = page_get_access_time(page);
 */
time_t page_get_access_time(Page *page);

/*
 * page_print - Print page information
 * 
 * Prints page information in human-readable format.
 * Useful for debugging.
 * 
 * Parameters:
 *   page: Pointer to Page
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   page_print(page);
 */
void page_print(Page *page);

#endif // PAGE_H
