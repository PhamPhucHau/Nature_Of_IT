/*
 * main.c - Key-Value Store Test Program
 * 
 * This program demonstrates the key-value store functionality:
 * 1. Create a storage
 * 2. Add some test data
 * 3. Retrieve data
 * 4. Save to disk
 * 5. Load from disk
 * 6. Verify data integrity
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "storage.h"
#include "serializer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * ============================================================================
 * TEST UTILITIES
 * ============================================================================
 */

/* Color codes for output */
#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"
#define YELLOW  "\x1b[33m"
#define RESET   "\x1b[0m"

/* Test result tracking */
static int tests_passed = 0;
static int tests_failed = 0;

/*
 * assert_equal - Assert that two strings are equal
 * 
 * Parameters:
 *   actual: The actual value
 *   expected: The expected value
 *   test_name: Name of the test
 */
void assert_equal(const char *actual, const char *expected, const char *test_name) {
    if (actual == NULL && expected == NULL) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else if (actual == NULL || expected == NULL) {
        printf(RED "✗ FAIL" RESET ": %s (one is NULL)\n", test_name);
        tests_failed++;
    } else if (strcmp(actual, expected) == 0) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s\n", test_name);
        printf("  Expected: %s\n", expected);
        printf("  Actual:   %s\n", actual);
        tests_failed++;
    }
}

/*
 * assert_int_equal - Assert that two integers are equal
 * 
 * Parameters:
 *   actual: The actual value
 *   expected: The expected value
 *   test_name: Name of the test
 */
void assert_int_equal(int actual, int expected, const char *test_name) {
    if (actual == expected) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s\n", test_name);
        printf("  Expected: %d\n", expected);
        printf("  Actual:   %d\n", actual);
        tests_failed++;
    }
}

/*
 * print_entry - Callback function for listing entries
 * 
 * This function is called for each entry in the storage.
 * It's used to demonstrate the storage_list function.
 */
void print_entry(const char *key, const char *value, void *data) {
    int *count = (int *)data;
    printf("  [%d] %s => %s\n", ++(*count), key, value);
}

/*
 * ============================================================================
 * TEST CASES
 * ============================================================================
 */

/*
 * test_create_and_destroy - Test storage creation and destruction
 * 
 * This test verifies that:
 * 1. Storage can be created
 * 2. Storage can be destroyed without errors
 */
void test_create_and_destroy() {
    printf("\n" YELLOW "=== Test: Create and Destroy ===" RESET "\n");
    
    Storage *store = storage_create("test.db");
    assert_int_equal(store != NULL ? 1 : 0, 1, "Storage created");
    assert_int_equal(storage_count(store), 0, "Storage is empty");
    
    storage_destroy(store);
    printf("Storage destroyed successfully\n");
}

/*
 * test_put_and_get - Test basic put and get operations
 * 
 * This test verifies that:
 * 1. Values can be stored
 * 2. Values can be retrieved
 * 3. Retrieved values match stored values
 */
void test_put_and_get() {
    printf("\n" YELLOW "=== Test: Put and Get ===" RESET "\n");
    
    Storage *store = storage_create("test.db");
    
    /* Test 1: Put and get a single entry */
    storage_put(store, "user:1", "Alice");
    const char *value = storage_get(store, "user:1");
    assert_equal(value, "Alice", "Get returns correct value");
    
    /* Test 2: Put and get multiple entries */
    storage_put(store, "user:2", "Bob");
    storage_put(store, "user:3", "Charlie");
    
    assert_equal(storage_get(store, "user:2"), "Bob", "Get user:2");
    assert_equal(storage_get(store, "user:3"), "Charlie", "Get user:3");
    
    /* Test 3: Get non-existent key */
    assert_equal(storage_get(store, "user:999"), NULL, "Get non-existent key returns NULL");
    
    /* Test 4: Check entry count */
    assert_int_equal(storage_count(store), 3, "Entry count is 3");
    
    storage_destroy(store);
}

/*
 * test_update - Test updating existing values
 * 
 * This test verifies that:
 * 1. Existing values can be updated
 * 2. Entry count doesn't increase on update
 */
void test_update() {
    printf("\n" YELLOW "=== Test: Update ===" RESET "\n");
    
    Storage *store = storage_create("test.db");
    
    /* Add initial value */
    storage_put(store, "user:1", "Alice");
    assert_int_equal(storage_count(store), 1, "Initial count is 1");
    
    /* Update value */
    storage_put(store, "user:1", "Alice Updated");
    assert_equal(storage_get(store, "user:1"), "Alice Updated", "Value updated");
    assert_int_equal(storage_count(store), 1, "Count still 1 after update");
    
    storage_destroy(store);
}

/*
 * test_delete - Test deleting entries
 * 
 * This test verifies that:
 * 1. Entries can be deleted
 * 2. Deleted entries cannot be retrieved
 * 3. Entry count decreases after deletion
 */
void test_delete() {
    printf("\n" YELLOW "=== Test: Delete ===" RESET "\n");
    
    Storage *store = storage_create("test.db");
    
    /* Add entries */
    storage_put(store, "user:1", "Alice");
    storage_put(store, "user:2", "Bob");
    storage_put(store, "user:3", "Charlie");
    assert_int_equal(storage_count(store), 3, "Initial count is 3");
    
    /* Delete an entry */
    int result = storage_delete(store, "user:2");
    assert_int_equal(result, 0, "Delete returns 0 on success");
    assert_equal(storage_get(store, "user:2"), NULL, "Deleted entry not found");
    assert_int_equal(storage_count(store), 2, "Count decreased to 2");
    
    /* Try to delete non-existent entry */
    result = storage_delete(store, "user:999");
    assert_int_equal(result, -1, "Delete non-existent returns -1");
    
    storage_destroy(store);
}

/*
 * test_exists - Test checking if key exists
 * 
 * This test verifies that:
 * 1. Existing keys are found
 * 2. Non-existent keys are not found
 */
void test_exists() {
    printf("\n" YELLOW "=== Test: Exists ===" RESET "\n");
    
    Storage *store = storage_create("test.db");
    
    storage_put(store, "user:1", "Alice");
    
    assert_int_equal(storage_exists(store, "user:1"), 1, "Existing key found");
    assert_int_equal(storage_exists(store, "user:999"), 0, "Non-existent key not found");
    
    storage_destroy(store);
}

/*
 * test_list - Test listing all entries
 * 
 * This test verifies that:
 * 1. All entries can be listed
 * 2. Callback is called for each entry
 */
void test_list() {
    printf("\n" YELLOW "=== Test: List ===" RESET "\n");
    
    Storage *store = storage_create("test.db");
    
    storage_put(store, "user:1", "Alice");
    storage_put(store, "user:2", "Bob");
    storage_put(store, "user:3", "Charlie");
    
    printf("Listing all entries:\n");
    int count = 0;
    int result = storage_list(store, print_entry, &count);
    assert_int_equal(result, 3, "List returns 3 entries");
    assert_int_equal(count, 3, "Callback called 3 times");
    
    storage_destroy(store);
}

/*
 * test_clear - Test clearing all entries
 * 
 * This test verifies that:
 * 1. All entries can be cleared
 * 2. Entry count becomes 0
 */
void test_clear() {
    printf("\n" YELLOW "=== Test: Clear ===" RESET "\n");
    
    Storage *store = storage_create("test.db");
    
    storage_put(store, "user:1", "Alice");
    storage_put(store, "user:2", "Bob");
    assert_int_equal(storage_count(store), 2, "Initial count is 2");
    
    storage_clear(store);
    assert_int_equal(storage_count(store), 0, "Count is 0 after clear");
    assert_equal(storage_get(store, "user:1"), NULL, "Entry not found after clear");
    
    storage_destroy(store);
}

/*
 * test_save_and_load - Test persistence (save and load)
 * 
 * This test verifies that:
 * 1. Data can be saved to disk
 * 2. Data can be loaded from disk
 * 3. Loaded data matches saved data
 */
void test_save_and_load() {
    printf("\n" YELLOW "=== Test: Save and Load ===" RESET "\n");
    
    const char *filename = "test_persistence.db";
    
    /* Create and populate storage */
    Storage *store1 = storage_create(filename);
    storage_put(store1, "user:1", "Alice");
    storage_put(store1, "user:2", "Bob");
    storage_put(store1, "user:3", "Charlie");
    
    printf("Saving storage to disk...\n");
    int result = storage_save(store1);
    assert_int_equal(result, 0, "Save returns 0 on success");
    
    storage_destroy(store1);
    
    /* Load storage from disk */
    Storage *store2 = storage_create(filename);
    printf("Loading storage from disk...\n");
    result = storage_load(store2);
    assert_int_equal(result, 0, "Load returns 0 on success");
    
    /* Verify loaded data */
    assert_int_equal(storage_count(store2), 3, "Loaded storage has 3 entries");
    assert_equal(storage_get(store2, "user:1"), "Alice", "Loaded user:1");
    assert_equal(storage_get(store2, "user:2"), "Bob", "Loaded user:2");
    assert_equal(storage_get(store2, "user:3"), "Charlie", "Loaded user:3");
    
    storage_destroy(store2);
    
    /* Clean up */
    remove(filename);
}

/*
 * test_statistics - Test getting storage statistics
 * 
 * This test verifies that:
 * 1. Statistics can be retrieved
 * 2. Statistics are accurate
 */
void test_statistics() {
    printf("\n" YELLOW "=== Test: Statistics ===" RESET "\n");
    
    Storage *store = storage_create("test.db");
    
    storage_put(store, "user:1", "Alice");
    storage_put(store, "user:2", "Bob");
    storage_put(store, "user:3", "Charlie");
    
    uint32_t total, empty, max_chain;
    int result = storage_get_stats(store, &total, &empty, &max_chain);
    
    assert_int_equal(result, 0, "Get stats returns 0");
    assert_int_equal(total, 3, "Total entries is 3");
    printf("  Empty buckets: %u\n", empty);
    printf("  Max chain length: %u\n", max_chain);
    
    storage_destroy(store);
}

/*
 * ============================================================================
 * MAIN TEST RUNNER
 * ============================================================================
 */

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     Key-Value Store - Phase 1 Test Suite                  ║\n");
    printf("║     Database Learning Project                             ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    /* Run all tests */
    test_create_and_destroy();
    test_put_and_get();
    test_update();
    test_delete();
    test_exists();
    test_list();
    test_clear();
    test_save_and_load();
    test_statistics();
    
    /* Print summary */
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    TEST SUMMARY                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf(GREEN "Passed: %d" RESET "\n", tests_passed);
    printf(RED "Failed: %d" RESET "\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n" GREEN "✓ All tests passed!" RESET "\n\n");
        return 0;
    } else {
        printf("\n" RED "✗ Some tests failed!" RESET "\n\n");
        return 1;
    }
}
