/*
 * main.c - Buffer Pool Test Program
 * 
 * Comprehensive test suite for buffer pool implementation.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "buffer_pool.h"
#include "page.h"
#include "lru.h"
#include "disk_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ============================================================================
 * TEST UTILITIES
 * ============================================================================ */

#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"
#define YELLOW  "\x1b[33m"
#define RESET   "\x1b[0m"

static int tests_passed = 0;
static int tests_failed = 0;

void assert_equal(int actual, int expected, const char *test_name) {
    if (actual == expected) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s (expected %d, got %d)\n",
               test_name, expected, actual);
        tests_failed++;
    }
}

void assert_not_null(void *ptr, const char *test_name) {
    if (ptr != NULL) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s (NULL pointer)\n", test_name);
        tests_failed++;
    }
}

void assert_null(void *ptr, const char *test_name) {
    if (ptr == NULL) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s (expected NULL)\n", test_name);
        tests_failed++;
    }
}

/* ============================================================================
 * PAGE TESTS
 * ============================================================================ */

void test_page_create_destroy() {
    printf("\n" YELLOW "=== Test: Page Create/Destroy ===" RESET "\n");
    
    Page *page = page_create(1);
    assert_not_null(page, "Page created");
    assert_equal(page->page_id, 1, "Page ID is 1");
    assert_equal(page->pin_count, 0, "Pin count is 0");
    assert_equal(page->is_dirty, 0, "Page is clean");
    
    page_destroy(page);
}

void test_page_pin_unpin() {
    printf("\n" YELLOW "=== Test: Page Pin/Unpin ===" RESET "\n");
    
    Page *page = page_create(1);
    
    page_pin(page);
    assert_equal(page->pin_count, 1, "Pin count is 1");
    assert_equal(page_is_pinned(page), 1, "Page is pinned");
    
    page_unpin(page);
    assert_equal(page->pin_count, 0, "Pin count is 0");
    assert_equal(page_is_pinned(page), 0, "Page is not pinned");
    
    page_destroy(page);
}

void test_page_dirty_flag() {
    printf("\n" YELLOW "=== Test: Page Dirty Flag ===" RESET "\n");
    
    Page *page = page_create(1);
    
    assert_equal(page_is_dirty(page), 0, "Page is clean initially");
    
    page_mark_dirty(page);
    assert_equal(page_is_dirty(page), 1, "Page is dirty");
    
    page_mark_clean(page);
    assert_equal(page_is_dirty(page), 0, "Page is clean");
    
    page_destroy(page);
}

/* ============================================================================
 * LRU TESTS
 * ============================================================================ */

void test_lru_create_destroy() {
    printf("\n" YELLOW "=== Test: LRU Create/Destroy ===" RESET "\n");
    
    LRUList *lru = lru_create();
    assert_not_null(lru, "LRU list created");
    assert_equal(lru_is_empty(lru), 1, "LRU list is empty");
    assert_equal(lru_get_size(lru), 0, "LRU size is 0");
    
    lru_destroy(lru);
}

void test_lru_add_remove() {
    printf("\n" YELLOW "=== Test: LRU Add/Remove ===" RESET "\n");
    
    LRUList *lru = lru_create();
    Page *page1 = page_create(1);
    Page *page2 = page_create(2);
    
    lru_add_page(lru, page1);
    assert_equal(lru_get_size(lru), 1, "LRU size is 1");
    
    lru_add_page(lru, page2);
    assert_equal(lru_get_size(lru), 2, "LRU size is 2");
    
    lru_remove_page(lru, page1);
    assert_equal(lru_get_size(lru), 1, "LRU size is 1 after remove");
    
    lru_destroy(lru);
    page_destroy(page1);
    page_destroy(page2);
}

void test_lru_get_lru_page() {
    printf("\n" YELLOW "=== Test: LRU Get LRU Page ===" RESET "\n");
    
    LRUList *lru = lru_create();
    Page *page1 = page_create(1);
    Page *page2 = page_create(2);
    
    lru_add_page(lru, page1);
    lru_add_page(lru, page2);
    
    Page *lru_page = lru_get_lru_page(lru);
    assert_not_null(lru_page, "LRU page exists");
    assert_equal(lru_page->page_id, 1, "LRU page is page 1");
    
    lru_destroy(lru);
    page_destroy(page1);
    page_destroy(page2);
}

/* ============================================================================
 * DISK MANAGER TESTS
 * ============================================================================ */

void test_disk_manager_create_destroy() {
    printf("\n" YELLOW "=== Test: Disk Manager Create/Destroy ===" RESET "\n");
    
    DiskManager *dm = disk_manager_create("test_db.bin");
    assert_not_null(dm, "Disk manager created");
    
    disk_manager_destroy(dm);
}

void test_disk_manager_allocate_page() {
    printf("\n" YELLOW "=== Test: Disk Manager Allocate Page ===" RESET "\n");
    
    DiskManager *dm = disk_manager_create("test_db2.bin");
    
    uint32_t page_id = disk_manager_allocate_page(dm);
    assert_equal(page_id, 0, "First page ID is 0");
    
    uint32_t count = disk_manager_get_page_count(dm);
    assert_equal(count, 1, "Page count is 1");
    
    disk_manager_destroy(dm);
}

/* ============================================================================
 * BUFFER POOL TESTS
 * ============================================================================ */

void test_buffer_pool_create_destroy() {
    printf("\n" YELLOW "=== Test: Buffer Pool Create/Destroy ===" RESET "\n");
    
    DiskManager *dm = disk_manager_create("test_db3.bin");
    BufferPool *pool = buffer_pool_create(10, dm);
    
    assert_not_null(pool, "Buffer pool created");
    assert_equal(buffer_pool_get_free_frame_count(pool), 10, "Free frames is 10");
    
    buffer_pool_destroy(pool);
    disk_manager_destroy(dm);
}

void test_buffer_pool_fetch_page() {
    printf("\n" YELLOW "=== Test: Buffer Pool Fetch Page ===" RESET "\n");
    
    DiskManager *dm = disk_manager_create("test_db4.bin");
    BufferPool *pool = buffer_pool_create(10, dm);
    
    /* Allocate a page on disk */
    uint32_t page_id = disk_manager_allocate_page(dm);
    
    /* Fetch the page */
    Page *page = buffer_pool_fetch_page(pool, page_id);
    assert_not_null(page, "Page fetched");
    assert_equal(page->page_id, page_id, "Page ID matches");
    assert_equal(page_is_pinned(page), 1, "Page is pinned");
    
    buffer_pool_destroy(pool);
    disk_manager_destroy(dm);
}

void test_buffer_pool_unpin_page() {
    printf("\n" YELLOW "=== Test: Buffer Pool Unpin Page ===" RESET "\n");
    
    DiskManager *dm = disk_manager_create("test_db5.bin");
    BufferPool *pool = buffer_pool_create(10, dm);
    
    uint32_t page_id = disk_manager_allocate_page(dm);
    Page *page = buffer_pool_fetch_page(pool, page_id);
    
    assert_equal(buffer_pool_unpin_page(pool, page_id, 0), 0, "Unpin succeeds");
    assert_equal(page_is_pinned(page), 0, "Page is not pinned");
    
    buffer_pool_destroy(pool);
    disk_manager_destroy(dm);
}

void test_buffer_pool_dirty_page() {
    printf("\n" YELLOW "=== Test: Buffer Pool Dirty Page ===" RESET "\n");
    
    DiskManager *dm = disk_manager_create("test_db6.bin");
    BufferPool *pool = buffer_pool_create(10, dm);
    
    uint32_t page_id = disk_manager_allocate_page(dm);
    Page *page = buffer_pool_fetch_page(pool, page_id);
    
    buffer_pool_unpin_page(pool, page_id, 1);
    assert_equal(page_is_dirty(page), 1, "Page is dirty");
    
    buffer_pool_destroy(pool);
    disk_manager_destroy(dm);
}

void test_buffer_pool_new_page() {
    printf("\n" YELLOW "=== Test: Buffer Pool New Page ===" RESET "\n");
    
    DiskManager *dm = disk_manager_create("test_db7.bin");
    BufferPool *pool = buffer_pool_create(10, dm);
    
    uint32_t page_id;
    Page *page = buffer_pool_new_page(pool, &page_id);
    
    assert_not_null(page, "New page created");
    assert_equal(page_is_pinned(page), 1, "New page is pinned");
    
    buffer_pool_destroy(pool);
    disk_manager_destroy(dm);
}

void test_buffer_pool_statistics() {
    printf("\n" YELLOW "=== Test: Buffer Pool Statistics ===" RESET "\n");
    
    DiskManager *dm = disk_manager_create("test_db8.bin");
    BufferPool *pool = buffer_pool_create(10, dm);
    
    /* Allocate and fetch pages */
    for (int i = 0; i < 5; i++) {
        uint32_t page_id = disk_manager_allocate_page(dm);
        Page *page = buffer_pool_fetch_page(pool, page_id);
        buffer_pool_unpin_page(pool, page_id, 0);
    }
    
    /* Fetch same pages again (should be cache hits) */
    for (int i = 0; i < 5; i++) {
        Page *page = buffer_pool_fetch_page(pool, i);
        if (page) {
            buffer_pool_unpin_page(pool, i, 0);
        }
    }
    
    double hit_rate;
    uint64_t evict_count, flush_count;
    buffer_pool_get_stats(pool, &hit_rate, &evict_count, &flush_count);
    
    printf("  Hit Rate: %.2f%%\n", hit_rate);
    printf("  Evictions: %lu\n", evict_count);
    printf("  Flushes: %lu\n", flush_count);
    
    buffer_pool_destroy(pool);
    disk_manager_destroy(dm);
}

/* ============================================================================
 * MAIN TEST RUNNER
 * ============================================================================ */

int main(void) {
    printf("\n" YELLOW "╔════════════════════════════════════════════════════════════╗\n" RESET);
    printf(YELLOW "║         Buffer Pool Test Suite - Phase 4                    ║\n" RESET);
    printf(YELLOW "╚════════════════════════════════════════════════════════════╝\n" RESET);
    
    /* Page tests */
    test_page_create_destroy();
    test_page_pin_unpin();
    test_page_dirty_flag();
    
    /* LRU tests */
    test_lru_create_destroy();
    test_lru_add_remove();
    test_lru_get_lru_page();
    
    /* Disk manager tests */
    test_disk_manager_create_destroy();
    test_disk_manager_allocate_page();
    
    /* Buffer pool tests */
    test_buffer_pool_create_destroy();
    test_buffer_pool_fetch_page();
    test_buffer_pool_unpin_page();
    test_buffer_pool_dirty_page();
    test_buffer_pool_new_page();
    test_buffer_pool_statistics();
    
    /* Print summary */
    printf("\n" YELLOW "╔════════════════════════════════════════════════════════════╗\n" RESET);
    printf(YELLOW "║                      TEST SUMMARY                           ║\n" RESET);
    printf(YELLOW "╚════════════════════════════════════════════════════════════╝\n" RESET);
    
    int total = tests_passed + tests_failed;
    printf("Total Tests: %d\n", total);
    printf(GREEN "Passed: %d\n" RESET, tests_passed);
    printf(RED "Failed: %d\n" RESET, tests_failed);
    
    if (tests_failed == 0) {
        printf(GREEN "\n✓ All tests passed!\n" RESET);
        return 0;
    } else {
        printf(RED "\n✗ Some tests failed\n" RESET);
        return 1;
    }
}
