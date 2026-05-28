/*
 * buffer_pool.h - Buffer Pool Manager Header
 * 
 * Implements the buffer pool (page cache) for efficient memory management.
 * The buffer pool caches frequently accessed disk pages in memory.
 * 
 * Key Concepts:
 * - Buffer Pool: In-memory cache for disk pages
 * - Page Table: Hash map of page_id -> frame
 * - LRU List: Doubly linked list for replacement policy
 * - Free List: Available frames
 * - Dirty Pages: Pages modified in memory
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include "page.h"
#include "lru.h"
#include "disk_manager.h"
#include <stdint.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Default buffer pool size (number of frames) */
#define DEFAULT_BUFFER_POOL_SIZE 10

/* Hash table size for page table */
#define PAGE_TABLE_SIZE 997

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * BufferPool - Buffer pool manager
 * 
 * Manages in-memory cache of disk pages.
 * 
 * Fields:
 *   - frames: Array of pages (buffer frames)
 *   - page_table: Hash map of page_id -> frame index
 *   - lru_list: LRU replacement policy
 *   - disk_manager: Disk I/O manager
 *   - pool_size: Number of frames
 *   - free_list: List of free frame indices
 *   - free_count: Number of free frames
 *   - stats: Performance statistics
 */
typedef struct {
    Page **frames;                  /* Array of page pointers */
    uint32_t *page_table;           /* Hash table: page_id -> frame index */
    LRUList *lru_list;              /* LRU replacement policy */
    DiskManager *disk_manager;      /* Disk I/O manager */
    uint32_t pool_size;             /* Number of frames */
    uint32_t *free_list;            /* Free frame indices */
    uint32_t free_count;            /* Number of free frames */
    
    /* Statistics */
    uint64_t fetch_count;           /* Total fetches */
    uint64_t hit_count;             /* Cache hits */
    uint64_t miss_count;            /* Cache misses */
    uint64_t evict_count;           /* Pages evicted */
    uint64_t flush_count;           /* Pages flushed */
} BufferPool;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * buffer_pool_create - Create a new buffer pool
 * 
 * Allocates and initializes a new buffer pool with the given size.
 * 
 * Parameters:
 *   pool_size: Number of frames in buffer pool
 *   disk_manager: Pointer to DiskManager
 * 
 * Returns:
 *   Pointer to new BufferPool, or NULL on error
 * 
 * Example:
 *   DiskManager *dm = disk_manager_create("database.db");
 *   BufferPool *pool = buffer_pool_create(10, dm);
 *   if (!pool) {
 *       fprintf(stderr, "Failed to create buffer pool\n");
 *       return 1;
 *   }
 */
BufferPool* buffer_pool_create(uint32_t pool_size, DiskManager *disk_manager);

/*
 * buffer_pool_destroy - Free buffer pool memory
 * 
 * Flushes all dirty pages and frees the buffer pool.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   buffer_pool_destroy(pool);
 */
void buffer_pool_destroy(BufferPool *pool);

/*
 * buffer_pool_fetch_page - Fetch a page from buffer pool
 * 
 * Fetches a page from the buffer pool. If the page is not in memory,
 * it is loaded from disk. The page is pinned to prevent eviction.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 *   page_id: Page ID to fetch
 * 
 * Returns:
 *   Pointer to Page, or NULL on error
 * 
 * Example:
 *   Page *page = buffer_pool_fetch_page(pool, 1);
 *   if (!page) {
 *       fprintf(stderr, "Failed to fetch page\n");
 *   }
 */
Page* buffer_pool_fetch_page(BufferPool *pool, uint32_t page_id);

/*
 * buffer_pool_unpin_page - Unpin a page
 * 
 * Unpins a page, allowing it to be evicted if necessary.
 * If is_dirty is 1, marks the page as dirty.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 *   page_id: Page ID to unpin
 *   is_dirty: 1 if page was modified, 0 otherwise
 * 
 * Returns:
 *   0 on success, -1 if page not found
 * 
 * Example:
 *   if (buffer_pool_unpin_page(pool, 1, 1) != 0) {
 *       fprintf(stderr, "Failed to unpin page\n");
 *   }
 */
int buffer_pool_unpin_page(BufferPool *pool, uint32_t page_id, int is_dirty);

/*
 * buffer_pool_flush_page - Flush a page to disk
 * 
 * Writes a page to disk if it is dirty.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 *   page_id: Page ID to flush
 * 
 * Returns:
 *   0 on success, -1 if page not found
 * 
 * Example:
 *   if (buffer_pool_flush_page(pool, 1) != 0) {
 *       fprintf(stderr, "Failed to flush page\n");
 *   }
 */
int buffer_pool_flush_page(BufferPool *pool, uint32_t page_id);

/*
 * buffer_pool_flush_all - Flush all dirty pages
 * 
 * Writes all dirty pages to disk.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   if (buffer_pool_flush_all(pool) != 0) {
 *       fprintf(stderr, "Failed to flush all pages\n");
 *   }
 */
int buffer_pool_flush_all(BufferPool *pool);

/*
 * buffer_pool_new_page - Allocate a new page
 * 
 * Allocates a new page and returns it pinned.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 *   page_id: Output parameter for new page ID
 * 
 * Returns:
 *   Pointer to new Page, or NULL on error
 * 
 * Example:
 *   uint32_t page_id;
 *   Page *page = buffer_pool_new_page(pool, &page_id);
 *   if (!page) {
 *       fprintf(stderr, "Failed to allocate new page\n");
 *   }
 */
Page* buffer_pool_new_page(BufferPool *pool, uint32_t *page_id);

/*
 * buffer_pool_delete_page - Delete a page
 * 
 * Deletes a page from the buffer pool and disk.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 *   page_id: Page ID to delete
 * 
 * Returns:
 *   0 on success, -1 if page not found
 * 
 * Example:
 *   if (buffer_pool_delete_page(pool, 1) != 0) {
 *       fprintf(stderr, "Failed to delete page\n");
 *   }
 */
int buffer_pool_delete_page(BufferPool *pool, uint32_t page_id);

/*
 * buffer_pool_get_page - Get page without pinning
 * 
 * Gets a page from the buffer pool without pinning it.
 * Returns NULL if page is not in memory.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 *   page_id: Page ID to get
 * 
 * Returns:
 *   Pointer to Page, or NULL if not in memory
 * 
 * Example:
 *   Page *page = buffer_pool_get_page(pool, 1);
 *   if (page) {
 *       printf("Page is in memory\n");
 *   }
 */
Page* buffer_pool_get_page(BufferPool *pool, uint32_t page_id);

/*
 * buffer_pool_get_free_frame_count - Get number of free frames
 * 
 * Returns the number of free frames in the buffer pool.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 * 
 * Returns:
 *   Number of free frames
 * 
 * Example:
 *   uint32_t free = buffer_pool_get_free_frame_count(pool);
 *   printf("Free frames: %u\n", free);
 */
uint32_t buffer_pool_get_free_frame_count(BufferPool *pool);

/*
 * buffer_pool_get_stats - Get buffer pool statistics
 * 
 * Returns statistics about buffer pool performance.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 *   hit_rate: Output parameter for cache hit rate (0-100)
 *   evict_count: Output parameter for eviction count
 *   flush_count: Output parameter for flush count
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   double hit_rate;
 *   uint64_t evict_count, flush_count;
 *   buffer_pool_get_stats(pool, &hit_rate, &evict_count, &flush_count);
 *   printf("Hit rate: %.2f%%\n", hit_rate);
 */
int buffer_pool_get_stats(BufferPool *pool, double *hit_rate,
                          uint64_t *evict_count, uint64_t *flush_count);

/*
 * buffer_pool_print_stats - Print buffer pool statistics
 * 
 * Prints buffer pool statistics in human-readable format.
 * 
 * Parameters:
 *   pool: Pointer to BufferPool
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   buffer_pool_print_stats(pool);
 */
void buffer_pool_print_stats(BufferPool *pool);

#endif // BUFFER_POOL_H
