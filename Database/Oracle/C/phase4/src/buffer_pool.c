/*
 * buffer_pool.c - Buffer Pool Manager Implementation
 * 
 * Implements the buffer pool (page cache) for efficient memory management.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "buffer_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * HASH TABLE OPERATIONS
 * ============================================================================ */

/*
 * Hash function for page ID
 */
static uint32_t hash_page_id(uint32_t page_id) {
    return page_id % PAGE_TABLE_SIZE;
}

/* ============================================================================
 * BUFFER POOL CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * buffer_pool_create - Create a new buffer pool
 * 
 * Allocates and initializes a new buffer pool with the given size.
 */
BufferPool* buffer_pool_create(uint32_t pool_size, DiskManager *disk_manager) {
    if (pool_size == 0 || !disk_manager) {
        fprintf(stderr, "ERROR: Invalid parameters\n");
        return NULL;
    }
    
    /* Allocate memory for buffer pool */
    BufferPool *pool = (BufferPool *)malloc(sizeof(BufferPool));
    if (!pool) {
        fprintf(stderr, "ERROR: Failed to allocate memory for buffer pool\n");
        return NULL;
    }
    
    /* Allocate frames array */
    pool->frames = (Page **)malloc(sizeof(Page *) * pool_size);
    if (!pool->frames) {
        fprintf(stderr, "ERROR: Failed to allocate frames\n");
        free(pool);
        return NULL;
    }
    
    /* Allocate page table */
    pool->page_table = (uint32_t *)malloc(sizeof(uint32_t) * PAGE_TABLE_SIZE);
    if (!pool->page_table) {
        fprintf(stderr, "ERROR: Failed to allocate page table\n");
        free(pool->frames);
        free(pool);
        return NULL;
    }
    
    /* Allocate free list */
    pool->free_list = (uint32_t *)malloc(sizeof(uint32_t) * pool_size);
    if (!pool->free_list) {
        fprintf(stderr, "ERROR: Failed to allocate free list\n");
        free(pool->page_table);
        free(pool->frames);
        free(pool);
        return NULL;
    }
    
    /* Create LRU list */
    pool->lru_list = lru_create();
    if (!pool->lru_list) {
        fprintf(stderr, "ERROR: Failed to create LRU list\n");
        free(pool->free_list);
        free(pool->page_table);
        free(pool->frames);
        free(pool);
        return NULL;
    }
    
    /* Initialize buffer pool */
    pool->pool_size = pool_size;
    pool->disk_manager = disk_manager;
    
    /* Initialize frames */
    for (uint32_t i = 0; i < pool_size; i++) {
        pool->frames[i] = NULL;
    }
    
    /* Initialize page table */
    for (uint32_t i = 0; i < PAGE_TABLE_SIZE; i++) {
        pool->page_table[i] = (uint32_t)-1;
    }
    
    /* Initialize free list */
    for (uint32_t i = 0; i < pool_size; i++) {
        pool->free_list[i] = i;
    }
    pool->free_count = pool_size;
    
    /* Initialize statistics */
    pool->fetch_count = 0;
    pool->hit_count = 0;
    pool->miss_count = 0;
    pool->evict_count = 0;
    pool->flush_count = 0;
    
    return pool;
}

/*
 * buffer_pool_destroy - Free buffer pool memory
 * 
 * Flushes all dirty pages and frees the buffer pool.
 */
void buffer_pool_destroy(BufferPool *pool) {
    if (!pool) return;
    
    /* Flush all dirty pages */
    buffer_pool_flush_all(pool);
    
    /* Free all pages */
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->frames[i]) {
            page_destroy(pool->frames[i]);
        }
    }
    
    /* Free structures */
    if (pool->frames) free(pool->frames);
    if (pool->page_table) free(pool->page_table);
    if (pool->free_list) free(pool->free_list);
    if (pool->lru_list) lru_destroy(pool->lru_list);
    
    free(pool);
}

/* ============================================================================
 * PAGE FETCH AND UNPIN
 * ============================================================================ */

/*
 * buffer_pool_fetch_page - Fetch a page from buffer pool
 * 
 * Fetches a page from the buffer pool. If the page is not in memory,
 * it is loaded from disk. The page is pinned to prevent eviction.
 */
Page* buffer_pool_fetch_page(BufferPool *pool, uint32_t page_id) {
    if (!pool) return NULL;
    
    pool->fetch_count++;
    
    /* Check if page is in buffer pool */
    uint32_t hash = hash_page_id(page_id);
    if (pool->page_table[hash] != (uint32_t)-1) {
        /* Page is in memory (cache hit) */
        uint32_t frame_index = pool->page_table[hash];
        Page *page = pool->frames[frame_index];
        
        if (page && page->page_id == page_id) {
            pool->hit_count++;
            page_pin(page);
            page_update_access_time(page);
            lru_move_to_front(pool->lru_list, page);
            return page;
        }
    }
    
    /* Page not in memory (cache miss) */
    pool->miss_count++;
    
    /* Get a free frame or evict a page */
    uint32_t frame_index;
    if (pool->free_count > 0) {
        /* Use free frame */
        frame_index = pool->free_list[--pool->free_count];
    } else {
        /* Need to evict a page */
        Page *victim = lru_get_lru_page(pool->lru_list);
        if (!victim || page_is_pinned(victim)) {
            fprintf(stderr, "ERROR: No evictable pages\n");
            return NULL;
        }
        
        /* Flush victim if dirty */
        if (page_is_dirty(victim)) {
            disk_manager_write_page(pool->disk_manager, victim);
            pool->flush_count++;
        }
        
        /* Remove victim from page table and LRU */
        uint32_t victim_hash = hash_page_id(victim->page_id);
        pool->page_table[victim_hash] = (uint32_t)-1;
        lru_remove_page(pool->lru_list, victim);
        
        /* Find frame index of victim */
        for (uint32_t i = 0; i < pool->pool_size; i++) {
            if (pool->frames[i] == victim) {
                frame_index = i;
                break;
            }
        }
        
        page_destroy(victim);
        pool->evict_count++;
    }
    
    /* Create new page and load from disk */
    Page *page = page_create(page_id);
    if (!page) {
        fprintf(stderr, "ERROR: Failed to create page\n");
        return NULL;
    }
    
    if (disk_manager_read_page(pool->disk_manager, page_id, page) != 0) {
        fprintf(stderr, "ERROR: Failed to read page from disk\n");
        page_destroy(page);
        return NULL;
    }
    
    /* Add page to buffer pool */
    pool->frames[frame_index] = page;
    pool->page_table[hash] = frame_index;
    
    /* Add to LRU and pin */
    lru_add_page(pool->lru_list, page);
    page_pin(page);
    
    return page;
}

/*
 * buffer_pool_unpin_page - Unpin a page
 * 
 * Unpins a page, allowing it to be evicted if necessary.
 * If is_dirty is 1, marks the page as dirty.
 */
int buffer_pool_unpin_page(BufferPool *pool, uint32_t page_id, int is_dirty) {
    if (!pool) return -1;
    
    /* Find page in buffer pool */
    uint32_t hash = hash_page_id(page_id);
    if (pool->page_table[hash] == (uint32_t)-1) {
        fprintf(stderr, "ERROR: Page %u not in buffer pool\n", page_id);
        return -1;
    }
    
    uint32_t frame_index = pool->page_table[hash];
    Page *page = pool->frames[frame_index];
    
    if (!page || page->page_id != page_id) {
        fprintf(stderr, "ERROR: Page mismatch\n");
        return -1;
    }
    
    /* Mark dirty if needed */
    if (is_dirty) {
        page_mark_dirty(page);
    }
    
    /* Unpin page */
    page_unpin(page);
    
    return 0;
}

/* ============================================================================
 * PAGE FLUSH OPERATIONS
 * ============================================================================ */

/*
 * buffer_pool_flush_page - Flush a page to disk
 * 
 * Writes a page to disk if it is dirty.
 */
int buffer_pool_flush_page(BufferPool *pool, uint32_t page_id) {
    if (!pool) return -1;
    
    /* Find page in buffer pool */
    uint32_t hash = hash_page_id(page_id);
    if (pool->page_table[hash] == (uint32_t)-1) {
        fprintf(stderr, "ERROR: Page %u not in buffer pool\n", page_id);
        return -1;
    }
    
    uint32_t frame_index = pool->page_table[hash];
    Page *page = pool->frames[frame_index];
    
    if (!page || page->page_id != page_id) {
        fprintf(stderr, "ERROR: Page mismatch\n");
        return -1;
    }
    
    /* Flush if dirty */
    if (page_is_dirty(page)) {
        if (disk_manager_write_page(pool->disk_manager, page) != 0) {
            fprintf(stderr, "ERROR: Failed to flush page %u\n", page_id);
            return -1;
        }
        pool->flush_count++;
    }
    
    return 0;
}

/*
 * buffer_pool_flush_all - Flush all dirty pages
 * 
 * Writes all dirty pages to disk.
 */
int buffer_pool_flush_all(BufferPool *pool) {
    if (!pool) return -1;
    
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->frames[i] && page_is_dirty(pool->frames[i])) {
            if (disk_manager_write_page(pool->disk_manager, pool->frames[i]) != 0) {
                fprintf(stderr, "ERROR: Failed to flush page %u\n",
                        pool->frames[i]->page_id);
                return -1;
            }
            pool->flush_count++;
        }
    }
    
    return 0;
}

/* ============================================================================
 * PAGE ALLOCATION AND DELETION
 * ============================================================================ */

/*
 * buffer_pool_new_page - Allocate a new page
 * 
 * Allocates a new page and returns it pinned.
 */
Page* buffer_pool_new_page(BufferPool *pool, uint32_t *page_id) {
    if (!pool || !page_id) return NULL;
    
    /* Allocate page on disk */
    *page_id = disk_manager_allocate_page(pool->disk_manager);
    if (*page_id == INVALID_PAGE_ID) {
        fprintf(stderr, "ERROR: Failed to allocate page on disk\n");
        return NULL;
    }
    
    /* Fetch the new page */
    return buffer_pool_fetch_page(pool, *page_id);
}

/*
 * buffer_pool_delete_page - Delete a page
 * 
 * Deletes a page from the buffer pool and disk.
 */
int buffer_pool_delete_page(BufferPool *pool, uint32_t page_id) {
    if (!pool) return -1;
    
    /* Find page in buffer pool */
    uint32_t hash = hash_page_id(page_id);
    if (pool->page_table[hash] != (uint32_t)-1) {
        uint32_t frame_index = pool->page_table[hash];
        Page *page = pool->frames[frame_index];
        
        if (page && page->page_id == page_id) {
            if (page_is_pinned(page)) {
                fprintf(stderr, "ERROR: Cannot delete pinned page\n");
                return -1;
            }
            
            /* Remove from LRU and page table */
            lru_remove_page(pool->lru_list, page);
            pool->page_table[hash] = (uint32_t)-1;
            
            /* Free page */
            page_destroy(page);
            pool->frames[frame_index] = NULL;
            pool->free_list[pool->free_count++] = frame_index;
        }
    }
    
    return 0;
}

/*
 * buffer_pool_get_page - Get page without pinning
 * 
 * Gets a page from the buffer pool without pinning it.
 * Returns NULL if page is not in memory.
 */
Page* buffer_pool_get_page(BufferPool *pool, uint32_t page_id) {
    if (!pool) return NULL;
    
    uint32_t hash = hash_page_id(page_id);
    if (pool->page_table[hash] != (uint32_t)-1) {
        uint32_t frame_index = pool->page_table[hash];
        Page *page = pool->frames[frame_index];
        
        if (page && page->page_id == page_id) {
            return page;
        }
    }
    
    return NULL;
}

/* ============================================================================
 * STATISTICS
 * ============================================================================ */

/*
 * buffer_pool_get_free_frame_count - Get number of free frames
 * 
 * Returns the number of free frames in the buffer pool.
 */
uint32_t buffer_pool_get_free_frame_count(BufferPool *pool) {
    if (!pool) return 0;
    return pool->free_count;
}

/*
 * buffer_pool_get_stats - Get buffer pool statistics
 * 
 * Returns statistics about buffer pool performance.
 */
int buffer_pool_get_stats(BufferPool *pool, double *hit_rate,
                          uint64_t *evict_count, uint64_t *flush_count) {
    if (!pool || !hit_rate || !evict_count || !flush_count) return -1;
    
    if (pool->fetch_count > 0) {
        *hit_rate = (double)pool->hit_count / pool->fetch_count * 100.0;
    } else {
        *hit_rate = 0.0;
    }
    
    *evict_count = pool->evict_count;
    *flush_count = pool->flush_count;
    
    return 0;
}

/*
 * buffer_pool_print_stats - Print buffer pool statistics
 * 
 * Prints buffer pool statistics in human-readable format.
 */
void buffer_pool_print_stats(BufferPool *pool) {
    if (!pool) {
        printf("NULL buffer pool\n");
        return;
    }
    
    double hit_rate = 0.0;
    if (pool->fetch_count > 0) {
        hit_rate = (double)pool->hit_count / pool->fetch_count * 100.0;
    }
    
    printf("Buffer Pool Statistics:\n");
    printf("  Pool Size: %u frames\n", pool->pool_size);
    printf("  Free Frames: %u\n", pool->free_count);
    printf("  Total Fetches: %lu\n", pool->fetch_count);
    printf("  Cache Hits: %lu\n", pool->hit_count);
    printf("  Cache Misses: %lu\n", pool->miss_count);
    printf("  Hit Rate: %.2f%%\n", hit_rate);
    printf("  Pages Evicted: %lu\n", pool->evict_count);
    printf("  Pages Flushed: %lu\n", pool->flush_count);
}
