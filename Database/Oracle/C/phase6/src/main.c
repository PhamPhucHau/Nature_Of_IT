/*
 * main.c - Concurrency Control Test Program
 * 
 * Comprehensive test suite for concurrency control implementation.
 * Tests lock manager, MVCC, deadlock detection, and isolation levels.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "lock.h"
#include "mvcc.h"
#include "deadlock.h"
#include "isolation.h"
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

void assert_true(int condition, const char *test_name) {
    if (condition) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s (condition false)\n", test_name);
        tests_failed++;
    }
}

/* ============================================================================
 * LOCK MANAGER TESTS
 * ============================================================================ */

void test_lock_manager_create_destroy() {
    printf("\n" YELLOW "=== Test: Lock Manager Create/Destroy ===" RESET "\n");
    
    LockManager *mgr = lock_manager_create();
    assert_not_null(mgr, "Lock manager created");
    assert_equal(mgr->lock_count, 0, "Lock count is 0");
    
    lock_manager_destroy(mgr);
}

void test_lock_manager_acquire_shared_lock() {
    printf("\n" YELLOW "=== Test: Lock Manager Acquire Shared Lock ===" RESET "\n");
    
    LockManager *mgr = lock_manager_create();
    
    LockStatus status = lock_manager_acquire_lock(mgr, 1, 1, LOCK_SHARED);
    assert_equal(status, LOCK_GRANTED, "Shared lock granted");
    assert_equal(mgr->lock_count, 1, "Lock count is 1");
    
    lock_manager_destroy(mgr);
}

void test_lock_manager_acquire_exclusive_lock() {
    printf("\n" YELLOW "=== Test: Lock Manager Acquire Exclusive Lock ===" RESET "\n");
    
    LockManager *mgr = lock_manager_create();
    
    LockStatus status = lock_manager_acquire_lock(mgr, 1, 1, LOCK_EXCLUSIVE);
    assert_equal(status, LOCK_GRANTED, "Exclusive lock granted");
    assert_equal(mgr->lock_count, 1, "Lock count is 1");
    
    lock_manager_destroy(mgr);
}

void test_lock_manager_shared_lock_compatibility() {
    printf("\n" YELLOW "=== Test: Lock Manager Shared Lock Compatibility ===" RESET "\n");
    
    LockManager *mgr = lock_manager_create();
    
    /* First transaction acquires shared lock */
    LockStatus status1 = lock_manager_acquire_lock(mgr, 1, 1, LOCK_SHARED);
    assert_equal(status1, LOCK_GRANTED, "First shared lock granted");
    
    /* Second transaction acquires shared lock on same resource */
    LockStatus status2 = lock_manager_acquire_lock(mgr, 2, 1, LOCK_SHARED);
    assert_equal(status2, LOCK_GRANTED, "Second shared lock granted");
    
    lock_manager_destroy(mgr);
}

void test_lock_manager_exclusive_lock_conflict() {
    printf("\n" YELLOW "=== Test: Lock Manager Exclusive Lock Conflict ===" RESET "\n");
    
    LockManager *mgr = lock_manager_create();
    
    /* First transaction acquires exclusive lock */
    LockStatus status1 = lock_manager_acquire_lock(mgr, 1, 1, LOCK_EXCLUSIVE);
    assert_equal(status1, LOCK_GRANTED, "Exclusive lock granted");
    
    /* Second transaction tries to acquire shared lock */
    LockStatus status2 = lock_manager_acquire_lock(mgr, 2, 1, LOCK_SHARED);
    assert_equal(status2, LOCK_WAITING, "Second lock waiting");
    
    lock_manager_destroy(mgr);
}

void test_lock_manager_release_lock() {
    printf("\n" YELLOW "=== Test: Lock Manager Release Lock ===" RESET "\n");
    
    LockManager *mgr = lock_manager_create();
    
    lock_manager_acquire_lock(mgr, 1, 1, LOCK_EXCLUSIVE);
    assert_equal(lock_manager_has_lock(mgr, 1, 1), 1, "Transaction has lock");
    
    int result = lock_manager_release_lock(mgr, 1, 1);
    assert_equal(result, 0, "Lock released");
    assert_equal(lock_manager_has_lock(mgr, 1, 1), 0, "Transaction no longer has lock");
    
    lock_manager_destroy(mgr);
}

void test_lock_manager_release_all_locks() {
    printf("\n" YELLOW "=== Test: Lock Manager Release All Locks ===" RESET "\n");
    
    LockManager *mgr = lock_manager_create();
    
    lock_manager_acquire_lock(mgr, 1, 1, LOCK_SHARED);
    lock_manager_acquire_lock(mgr, 1, 2, LOCK_SHARED);
    lock_manager_acquire_lock(mgr, 1, 3, LOCK_EXCLUSIVE);
    
    uint32_t released = lock_manager_release_all_locks(mgr, 1);
    assert_equal(released, 3, "All 3 locks released");
    
    lock_manager_destroy(mgr);
}

void test_lock_manager_get_lock_type() {
    printf("\n" YELLOW "=== Test: Lock Manager Get Lock Type ===" RESET "\n");
    
    LockManager *mgr = lock_manager_create();
    
    lock_manager_acquire_lock(mgr, 1, 1, LOCK_SHARED);
    int type = lock_manager_get_lock_type(mgr, 1, 1);
    assert_equal(type, LOCK_SHARED, "Lock type is SHARED");
    
    lock_manager_destroy(mgr);
}

/* ============================================================================
 * MVCC TESTS
 * ============================================================================ */

void test_mvcc_manager_create_destroy() {
    printf("\n" YELLOW "=== Test: MVCC Manager Create/Destroy ===" RESET "\n");
    
    MVCCManager *mgr = mvcc_manager_create();
    assert_not_null(mgr, "MVCC manager created");
    assert_equal(mgr->record_count, 0, "Record count is 0");
    
    mvcc_manager_destroy(mgr);
}

void test_mvcc_manager_create_version() {
    printf("\n" YELLOW "=== Test: MVCC Manager Create Version ===" RESET "\n");
    
    MVCCManager *mgr = mvcc_manager_create();
    
    uint64_t version_id = mvcc_manager_create_version(mgr, 1, 1, "data1");
    assert_true(version_id > 0, "Version created");
    assert_equal(mgr->record_count, 1, "Record count is 1");
    
    mvcc_manager_destroy(mgr);
}

void test_mvcc_manager_read_version() {
    printf("\n" YELLOW "=== Test: MVCC Manager Read Version ===" RESET "\n");
    
    MVCCManager *mgr = mvcc_manager_create();
    
    uint64_t version_id = mvcc_manager_create_version(mgr, 1, 1, "data1");
    mvcc_manager_commit_version(mgr, version_id);
    
    Version *version = mvcc_manager_read_version(mgr, 1, 1);
    assert_not_null(version, "Version read");
    assert_equal(version->status, VERSION_COMMITTED, "Version is committed");
    
    mvcc_manager_destroy(mgr);
}

void test_mvcc_manager_commit_version() {
    printf("\n" YELLOW "=== Test: MVCC Manager Commit Version ===" RESET "\n");
    
    MVCCManager *mgr = mvcc_manager_create();
    
    uint64_t version_id = mvcc_manager_create_version(mgr, 1, 1, "data1");
    assert_equal(mvcc_manager_commit_version(mgr, version_id), 0, "Version committed");
    
    mvcc_manager_destroy(mgr);
}

void test_mvcc_manager_abort_version() {
    printf("\n" YELLOW "=== Test: MVCC Manager Abort Version ===" RESET "\n");
    
    MVCCManager *mgr = mvcc_manager_create();
    
    uint64_t version_id = mvcc_manager_create_version(mgr, 1, 1, "data1");
    assert_equal(mvcc_manager_abort_version(mgr, version_id), 0, "Version aborted");
    
    mvcc_manager_destroy(mgr);
}

void test_mvcc_manager_version_chain() {
    printf("\n" YELLOW "=== Test: MVCC Manager Version Chain ===" RESET "\n");
    
    MVCCManager *mgr = mvcc_manager_create();
    
    /* Create multiple versions */
    uint64_t v1 = mvcc_manager_create_version(mgr, 1, 1, "data1");
    uint64_t v2 = mvcc_manager_create_version(mgr, 1, 2, "data2");
    uint64_t v3 = mvcc_manager_create_version(mgr, 1, 3, "data3");
    
    assert_equal(mvcc_manager_get_version_count(mgr, 1), 3, "3 versions created");
    
    mvcc_manager_destroy(mgr);
}

void test_mvcc_manager_garbage_collect() {
    printf("\n" YELLOW "=== Test: MVCC Manager Garbage Collect ===" RESET "\n");
    
    MVCCManager *mgr = mvcc_manager_create();
    
    uint64_t v1 = mvcc_manager_create_version(mgr, 1, 1, "data1");
    uint64_t v2 = mvcc_manager_create_version(mgr, 1, 2, "data2");
    
    mvcc_manager_abort_version(mgr, v1);
    
    uint32_t collected = mvcc_manager_garbage_collect(mgr, 3);
    assert_true(collected > 0, "Versions garbage collected");
    
    mvcc_manager_destroy(mgr);
}

/* ============================================================================
 * DEADLOCK DETECTION TESTS
 * ============================================================================ */

void test_deadlock_detector_create_destroy() {
    printf("\n" YELLOW "=== Test: Deadlock Detector Create/Destroy ===" RESET "\n");
    
    DeadlockDetector *detector = deadlock_detector_create(VICTIM_YOUNGEST);
    assert_not_null(detector, "Deadlock detector created");
    
    deadlock_detector_destroy(detector);
}

void test_deadlock_detector_add_edge() {
    printf("\n" YELLOW "=== Test: Deadlock Detector Add Edge ===" RESET "\n");
    
    DeadlockDetector *detector = deadlock_detector_create(VICTIM_YOUNGEST);
    
    int result = deadlock_detector_add_edge(detector, 1, 2, 1);
    assert_equal(result, 0, "Edge added");
    assert_equal(detector->graph.edge_count, 1, "Edge count is 1");
    
    deadlock_detector_destroy(detector);
}

void test_deadlock_detector_remove_edge() {
    printf("\n" YELLOW "=== Test: Deadlock Detector Remove Edge ===" RESET "\n");
    
    DeadlockDetector *detector = deadlock_detector_create(VICTIM_YOUNGEST);
    
    deadlock_detector_add_edge(detector, 1, 2, 1);
    int result = deadlock_detector_remove_edge(detector, 1, 2);
    assert_equal(result, 0, "Edge removed");
    assert_equal(detector->graph.edge_count, 0, "Edge count is 0");
    
    deadlock_detector_destroy(detector);
}

void test_deadlock_detector_no_cycle() {
    printf("\n" YELLOW "=== Test: Deadlock Detector No Cycle ===" RESET "\n");
    
    DeadlockDetector *detector = deadlock_detector_create(VICTIM_YOUNGEST);
    
    deadlock_detector_add_edge(detector, 1, 2, 1);
    deadlock_detector_add_edge(detector, 2, 3, 2);
    
    int result = deadlock_detector_detect(detector);
    assert_equal(result, 0, "No cycle detected");
    
    deadlock_detector_destroy(detector);
}

void test_deadlock_detector_cycle() {
    printf("\n" YELLOW "=== Test: Deadlock Detector Cycle ===" RESET "\n");
    
    DeadlockDetector *detector = deadlock_detector_create(VICTIM_YOUNGEST);
    
    /* Create cycle: 1 -> 2 -> 3 -> 1 */
    deadlock_detector_add_edge(detector, 1, 2, 1);
    deadlock_detector_add_edge(detector, 2, 3, 2);
    deadlock_detector_add_edge(detector, 3, 1, 3);
    
    int result = deadlock_detector_detect(detector);
    assert_equal(result, 1, "Cycle detected");
    
    deadlock_detector_destroy(detector);
}

void test_deadlock_detector_find_victim() {
    printf("\n" YELLOW "=== Test: Deadlock Detector Find Victim ===" RESET "\n");
    
    DeadlockDetector *detector = deadlock_detector_create(VICTIM_YOUNGEST);
    
    deadlock_detector_add_edge(detector, 1, 2, 1);
    deadlock_detector_add_edge(detector, 2, 3, 2);
    deadlock_detector_add_edge(detector, 3, 1, 3);
    
    uint32_t victim = deadlock_detector_find_victim(detector);
    assert_true(victim > 0, "Victim found");
    
    deadlock_detector_destroy(detector);
}

void test_deadlock_detector_resolve() {
    printf("\n" YELLOW "=== Test: Deadlock Detector Resolve ===" RESET "\n");
    
    DeadlockDetector *detector = deadlock_detector_create(VICTIM_YOUNGEST);
    
    deadlock_detector_add_edge(detector, 1, 2, 1);
    deadlock_detector_add_edge(detector, 2, 3, 2);
    deadlock_detector_add_edge(detector, 3, 1, 3);
    
    uint32_t victim = deadlock_detector_resolve(detector);
    assert_true(victim > 0, "Deadlock resolved");
    assert_equal(detector->deadlocks_resolved, 1, "Deadlock count incremented");
    
    deadlock_detector_destroy(detector);
}

/* ============================================================================
 * ISOLATION LEVEL TESTS
 * ============================================================================ */

void test_isolation_manager_create_destroy() {
    printf("\n" YELLOW "=== Test: Isolation Manager Create/Destroy ===" RESET "\n");
    
    IsolationManager *mgr = isolation_manager_create(ISOLATION_SERIALIZABLE);
    assert_not_null(mgr, "Isolation manager created");
    assert_equal(mgr->level, ISOLATION_SERIALIZABLE, "Level is SERIALIZABLE");
    
    isolation_manager_destroy(mgr);
}

void test_isolation_manager_read_uncommitted() {
    printf("\n" YELLOW "=== Test: Isolation Manager Read Uncommitted ===" RESET "\n");
    
    IsolationManager *mgr = isolation_manager_create(ISOLATION_READ_UNCOMMITTED);
    
    assert_equal(isolation_manager_allows_dirty_read(mgr), 1, "Dirty reads allowed");
    assert_equal(isolation_manager_allows_non_repeatable_read(mgr), 1, "Non-repeatable reads allowed");
    assert_equal(isolation_manager_allows_phantom_read(mgr), 1, "Phantom reads allowed");
    
    isolation_manager_destroy(mgr);
}

void test_isolation_manager_read_committed() {
    printf("\n" YELLOW "=== Test: Isolation Manager Read Committed ===" RESET "\n");
    
    IsolationManager *mgr = isolation_manager_create(ISOLATION_READ_COMMITTED);
    
    assert_equal(isolation_manager_allows_dirty_read(mgr), 0, "Dirty reads not allowed");
    assert_equal(isolation_manager_allows_non_repeatable_read(mgr), 1, "Non-repeatable reads allowed");
    assert_equal(isolation_manager_allows_phantom_read(mgr), 1, "Phantom reads allowed");
    
    isolation_manager_destroy(mgr);
}

void test_isolation_manager_repeatable_read() {
    printf("\n" YELLOW "=== Test: Isolation Manager Repeatable Read ===" RESET "\n");
    
    IsolationManager *mgr = isolation_manager_create(ISOLATION_REPEATABLE_READ);
    
    assert_equal(isolation_manager_allows_dirty_read(mgr), 0, "Dirty reads not allowed");
    assert_equal(isolation_manager_allows_non_repeatable_read(mgr), 0, "Non-repeatable reads not allowed");
    assert_equal(isolation_manager_allows_phantom_read(mgr), 1, "Phantom reads allowed");
    
    isolation_manager_destroy(mgr);
}

void test_isolation_manager_serializable() {
    printf("\n" YELLOW "=== Test: Isolation Manager Serializable ===" RESET "\n");
    
    IsolationManager *mgr = isolation_manager_create(ISOLATION_SERIALIZABLE);
    
    assert_equal(isolation_manager_allows_dirty_read(mgr), 0, "Dirty reads not allowed");
    assert_equal(isolation_manager_allows_non_repeatable_read(mgr), 0, "Non-repeatable reads not allowed");
    assert_equal(isolation_manager_allows_phantom_read(mgr), 0, "Phantom reads not allowed");
    
    isolation_manager_destroy(mgr);
}

void test_isolation_manager_detect_anomaly() {
    printf("\n" YELLOW "=== Test: Isolation Manager Detect Anomaly ===" RESET "\n");
    
    IsolationManager *mgr = isolation_manager_create(ISOLATION_SERIALIZABLE);
    
    int result = isolation_manager_detect_anomaly(mgr, ANOMALY_DIRTY_READ);
    assert_equal(result, 1, "Dirty read anomaly detected");
    assert_equal(mgr->anomalies_detected, 1, "Anomaly count incremented");
    
    isolation_manager_destroy(mgr);
}

void test_isolation_manager_set_level() {
    printf("\n" YELLOW "=== Test: Isolation Manager Set Level ===" RESET "\n");
    
    IsolationManager *mgr = isolation_manager_create(ISOLATION_READ_UNCOMMITTED);
    
    isolation_manager_set_level(mgr, ISOLATION_SERIALIZABLE);
    assert_equal(mgr->level, ISOLATION_SERIALIZABLE, "Level changed to SERIALIZABLE");
    assert_equal(isolation_manager_allows_dirty_read(mgr), 0, "Dirty reads now not allowed");
    
    isolation_manager_destroy(mgr);
}

/* ============================================================================
 * INTEGRATION TESTS
 * ============================================================================ */

void test_lock_and_mvcc_integration() {
    printf("\n" YELLOW "=== Test: Lock and MVCC Integration ===" RESET "\n");
    
    LockManager *lock_mgr = lock_manager_create();
    MVCCManager *mvcc_mgr = mvcc_manager_create();
    
    /* Transaction 1: Acquire lock and create version */
    lock_manager_acquire_lock(lock_mgr, 1, 1, LOCK_EXCLUSIVE);
    uint64_t v1 = mvcc_manager_create_version(mvcc_mgr, 1, 1, "data1");
    mvcc_manager_commit_version(mvcc_mgr, v1);
    
    /* Transaction 2: Read version */
    Version *version = mvcc_manager_read_version(mvcc_mgr, 1, 2);
    assert_not_null(version, "Version read by transaction 2");
    
    lock_manager_destroy(lock_mgr);
    mvcc_manager_destroy(mvcc_mgr);
}

void test_deadlock_and_isolation_integration() {
    printf("\n" YELLOW "=== Test: Deadlock and Isolation Integration ===" RESET "\n");
    
    DeadlockDetector *detector = deadlock_detector_create(VICTIM_YOUNGEST);
    IsolationManager *iso_mgr = isolation_manager_create(ISOLATION_SERIALIZABLE);
    
    /* Create potential deadlock */
    deadlock_detector_add_edge(detector, 1, 2, 1);
    deadlock_detector_add_edge(detector, 2, 1, 2);
    
    int deadlock = deadlock_detector_detect(detector);
    assert_equal(deadlock, 1, "Deadlock detected");
    
    /* Check isolation level prevents anomalies */
    int anomaly = isolation_manager_detect_anomaly(iso_mgr, ANOMALY_DIRTY_READ);
    assert_equal(anomaly, 1, "Anomaly detected at SERIALIZABLE level");
    
    deadlock_detector_destroy(detector);
    isolation_manager_destroy(iso_mgr);
}

/* ============================================================================
 * MAIN TEST RUNNER
 * ============================================================================ */

int main(void) {
    printf("\n" YELLOW "╔════════════════════════════════════════════════════════════╗\n" RESET);
    printf(YELLOW "║      Concurrency Control Test Suite - Phase 6              ║\n" RESET);
    printf(YELLOW "╚════════════════════════════════════════════════════════════╝\n" RESET);
    
    /* Lock Manager tests */
    test_lock_manager_create_destroy();
    test_lock_manager_acquire_shared_lock();
    test_lock_manager_acquire_exclusive_lock();
    test_lock_manager_shared_lock_compatibility();
    test_lock_manager_exclusive_lock_conflict();
    test_lock_manager_release_lock();
    test_lock_manager_release_all_locks();
    test_lock_manager_get_lock_type();
    
    /* MVCC tests */
    test_mvcc_manager_create_destroy();
    test_mvcc_manager_create_version();
    test_mvcc_manager_read_version();
    test_mvcc_manager_commit_version();
    test_mvcc_manager_abort_version();
    test_mvcc_manager_version_chain();
    test_mvcc_manager_garbage_collect();
    
    /* Deadlock Detection tests */
    test_deadlock_detector_create_destroy();
    test_deadlock_detector_add_edge();
    test_deadlock_detector_remove_edge();
    test_deadlock_detector_no_cycle();
    test_deadlock_detector_cycle();
    test_deadlock_detector_find_victim();
    test_deadlock_detector_resolve();
    
    /* Isolation Level tests */
    test_isolation_manager_create_destroy();
    test_isolation_manager_read_uncommitted();
    test_isolation_manager_read_committed();
    test_isolation_manager_repeatable_read();
    test_isolation_manager_serializable();
    test_isolation_manager_detect_anomaly();
    test_isolation_manager_set_level();
    
    /* Integration tests */
    test_lock_and_mvcc_integration();
    test_deadlock_and_isolation_integration();
    
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
