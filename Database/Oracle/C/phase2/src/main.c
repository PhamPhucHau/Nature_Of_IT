/*
 * main.c - B+ Tree Test Program
 * 
 * Comprehensive test suite for B+ tree implementation
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "btree.h"
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

/* ============================================================================
 * TEST CASES
 * ============================================================================ */

void test_create_and_destroy() {
    printf("\n" YELLOW "=== Test: Create and Destroy ===" RESET "\n");
    
    BTree *tree = btree_create();
    assert_int_equal(tree != NULL ? 1 : 0, 1, "Tree created");
    assert_int_equal(btree_count(tree), 0, "Tree is empty");
    
    btree_destroy(tree);
    printf("Tree destroyed successfully\n");
}

void test_single_insert() {
    printf("\n" YELLOW "=== Test: Single Insert ===" RESET "\n");
    
    BTree *tree = btree_create();
    
    int result = btree_insert(tree, 10, "Alice");
    assert_int_equal(result, 0, "Insert returns 0");
    assert_int_equal(btree_count(tree), 1, "Count is 1");
    
    btree_destroy(tree);
}

void test_insert_and_search() {
    printf("\n" YELLOW "=== Test: Insert and Search ===" RESET "\n");
    
    BTree *tree = btree_create();
    
    /* Insert values */
    btree_insert(tree, 10, "Alice");
    btree_insert(tree, 20, "Bob");
    btree_insert(tree, 30, "Charlie");
    
    assert_int_equal(btree_count(tree), 3, "Count is 3");
    
    /* Search for values */
    char value[256];
    int result = btree_search(tree, 10, value);
    assert_int_equal(result, 0, "Search for 10 succeeds");
    assert_equal(value, "Alice", "Value for 10 is Alice");
    
    result = btree_search(tree, 20, value);
    assert_int_equal(result, 0, "Search for 20 succeeds");
    assert_equal(value, "Bob", "Value for 20 is Bob");
    
    result = btree_search(tree, 999, value);
    assert_int_equal(result, -1, "Search for 999 fails");
    
    btree_destroy(tree);
}

void test_multiple_inserts() {
    printf("\n" YELLOW "=== Test: Multiple Inserts ===" RESET "\n");
    
    BTree *tree = btree_create();
    
    /* Insert 10 values */
    for (int i = 1; i <= 10; i++) {
        char value[256];
        sprintf(value, "Value%d", i);
        btree_insert(tree, i * 10, value);
    }
    
    assert_int_equal(btree_count(tree), 10, "Count is 10");
    
    /* Verify all values */
    for (int i = 1; i <= 10; i++) {
        char value[256];
        int result = btree_search(tree, i * 10, value);
        assert_int_equal(result, 0, "Search succeeds");
    }
    
    btree_destroy(tree);
}

void test_tree_structure() {
    printf("\n" YELLOW "=== Test: Tree Structure ===" RESET "\n");
    
    BTree *tree = btree_create();
    
    printf("Initial tree:\n");
    btree_print(tree);
    
    /* Insert values to cause splitting */
    for (int i = 1; i <= 8; i++) {
        char value[256];
        sprintf(value, "Value%d", i);
        btree_insert(tree, i, value);
    }
    
    printf("\nTree after 8 inserts:\n");
    btree_print(tree);
    
    assert_int_equal(btree_count(tree), 8, "Count is 8");
    
    btree_destroy(tree);
}

void range_search_callback(uint32_t key, const char *value, void *data) {
    int *count = (int *)data;
    printf("  [%d] Key: %u, Value: %s\n", ++(*count), key, value);
}

void test_range_search() {
    printf("\n" YELLOW "=== Test: Range Search ===" RESET "\n");
    
    BTree *tree = btree_create();
    
    /* Insert values */
    for (int i = 1; i <= 10; i++) {
        char value[256];
        sprintf(value, "Value%d", i);
        btree_insert(tree, i * 10, value);
    }
    
    printf("Range search (20 to 60):\n");
    int count = 0;
    int result = btree_range_search(tree, 20, 60, range_search_callback, &count);
    assert_int_equal(result, 5, "Range search returns 5 results");
    
    btree_destroy(tree);
}

void test_sequential_inserts() {
    printf("\n" YELLOW "=== Test: Sequential Inserts ===" RESET "\n");
    
    BTree *tree = btree_create();
    
    /* Insert sequential values */
    for (int i = 1; i <= 20; i++) {
        char value[256];
        sprintf(value, "Value%d", i);
        btree_insert(tree, i, value);
    }
    
    assert_int_equal(btree_count(tree), 20, "Count is 20");
    
    /* Verify all values */
    for (int i = 1; i <= 20; i++) {
        char value[256];
        int result = btree_search(tree, i, value);
        assert_int_equal(result, 0, "Search succeeds");
    }
    
    btree_destroy(tree);
}

void test_random_inserts() {
    printf("\n" YELLOW "=== Test: Random Inserts ===" RESET "\n");
    
    BTree *tree = btree_create();
    
    /* Insert random values */
    int keys[] = {50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45, 55, 65, 75, 85};
    int num_keys = sizeof(keys) / sizeof(keys[0]);
    
    for (int i = 0; i < num_keys; i++) {
        char value[256];
        sprintf(value, "Value%d", keys[i]);
        btree_insert(tree, keys[i], value);
    }
    
    assert_int_equal(btree_count(tree), num_keys, "Count matches");
    
    /* Verify all values */
    for (int i = 0; i < num_keys; i++) {
        char value[256];
        int result = btree_search(tree, keys[i], value);
        assert_int_equal(result, 0, "Search succeeds");
    }
    
    btree_destroy(tree);
}

void test_duplicate_keys() {
    printf("\n" YELLOW "=== Test: Duplicate Keys ===" RESET "\n");
    
    BTree *tree = btree_create();
    
    /* Insert same key twice */
    btree_insert(tree, 10, "Alice");
    btree_insert(tree, 10, "Alice2");  /* Should update */
    
    char value[256];
    btree_search(tree, 10, value);
    
    /* Note: Current implementation doesn't handle duplicates */
    printf("  Note: Duplicate key handling not implemented\n");
    
    btree_destroy(tree);
}

void test_large_values() {
    printf("\n" YELLOW "=== Test: Large Values ===" RESET "\n");
    
    BTree *tree = btree_create();
    
    /* Insert with large values */
    char large_value[MAX_VALUE_LENGTH];
    memset(large_value, 'A', MAX_VALUE_LENGTH - 1);
    large_value[MAX_VALUE_LENGTH - 1] = '\0';
    
    btree_insert(tree, 1, large_value);
    
    char retrieved[MAX_VALUE_LENGTH];
    int result = btree_search(tree, 1, retrieved);
    assert_int_equal(result, 0, "Search succeeds");
    assert_equal(retrieved, large_value, "Large value retrieved correctly");
    
    btree_destroy(tree);
}

/* ============================================================================
 * MAIN TEST RUNNER
 * ============================================================================ */

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     B+ Tree - Phase 2 Test Suite                          ║\n");
    printf("║     Database Learning Project                             ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    /* Run all tests */
    test_create_and_destroy();
    test_single_insert();
    test_insert_and_search();
    test_multiple_inserts();
    test_tree_structure();
    test_range_search();
    test_sequential_inserts();
    test_random_inserts();
    test_duplicate_keys();
    test_large_values();
    
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
