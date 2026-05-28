/*
 * main.c - Transaction & WAL Test Program
 * 
 * Comprehensive test suite for transaction management and write ahead logging.
 * Tests transaction creation, WAL operations, checkpoints, and crash recovery.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "transaction.h"
#include "wal.h"
#include "checkpoint.h"
#include "log_manager.h"
#include "recovery.h"
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
 * TRANSACTION TESTS
 * ============================================================================ */

void test_transaction_create_destroy() {
    printf("\n" YELLOW "=== Test: Transaction Create/Destroy ===" RESET "\n");
    
    Transaction *txn = transaction_create(1);
    assert_not_null(txn, "Transaction created");
    assert_equal(txn->txn_id, 1, "Transaction ID is 1");
    assert_equal(txn->state, TXN_ACTIVE, "Transaction is active");
    assert_equal(txn->op_count, 0, "Operation count is 0");
    
    transaction_destroy(txn);
}

void test_transaction_add_operation() {
    printf("\n" YELLOW "=== Test: Transaction Add Operation ===" RESET "\n");
    
    Transaction *txn = transaction_create(1);
    
    int result = transaction_add_operation(txn, OP_INSERT, 1, 0,
                                          "", "value1");
    assert_equal(result, 0, "Operation added successfully");
    assert_equal(txn->op_count, 1, "Operation count is 1");
    
    result = transaction_add_operation(txn, OP_UPDATE, 2, 10,
                                      "old_value", "new_value");
    assert_equal(result, 0, "Second operation added");
    assert_equal(txn->op_count, 2, "Operation count is 2");
    
    transaction_destroy(txn);
}

void test_transaction_commit() {
    printf("\n" YELLOW "=== Test: Transaction Commit ===" RESET "\n");
    
    Transaction *txn = transaction_create(1);
    assert_equal(transaction_is_active(txn), 1, "Transaction is active");
    
    int result = transaction_commit(txn);
    assert_equal(result, 0, "Commit succeeds");
    assert_equal(transaction_is_committed(txn), 1, "Transaction is committed");
    assert_equal(transaction_is_active(txn), 0, "Transaction is not active");
    
    transaction_destroy(txn);
}

void test_transaction_abort() {
    printf("\n" YELLOW "=== Test: Transaction Abort ===" RESET "\n");
    
    Transaction *txn = transaction_create(1);
    assert_equal(transaction_is_active(txn), 1, "Transaction is active");
    
    int result = transaction_abort(txn);
    assert_equal(result, 0, "Abort succeeds");
    assert_equal(txn->state, TXN_ABORTED, "Transaction is aborted");
    assert_equal(transaction_is_active(txn), 0, "Transaction is not active");
    
    transaction_destroy(txn);
}

void test_transaction_get_operation() {
    printf("\n" YELLOW "=== Test: Transaction Get Operation ===" RESET "\n");
    
    Transaction *txn = transaction_create(1);
    
    transaction_add_operation(txn, OP_INSERT, 1, 0, "", "value1");
    transaction_add_operation(txn, OP_UPDATE, 2, 10, "old", "new");
    
    Operation *op = transaction_get_operation(txn, 0);
    assert_not_null(op, "Operation 0 exists");
    assert_equal(op->type, OP_INSERT, "Operation 0 is INSERT");
    assert_equal(op->page_id, 1, "Operation 0 page_id is 1");
    
    op = transaction_get_operation(txn, 1);
    assert_not_null(op, "Operation 1 exists");
    assert_equal(op->type, OP_UPDATE, "Operation 1 is UPDATE");
    assert_equal(op->page_id, 2, "Operation 1 page_id is 2");
    
    transaction_destroy(txn);
}

/* ============================================================================
 * WAL TESTS
 * ============================================================================ */

void test_wal_create_destroy() {
    printf("\n" YELLOW "=== Test: WAL Create/Destroy ===" RESET "\n");
    
    WriteAheadLog *wal = wal_create("test_wal.log");
    assert_not_null(wal, "WAL created");
    assert_equal(wal->entry_count, 0, "Entry count is 0");
    assert_equal(wal->next_lsn, 1, "Next LSN is 1");
    
    wal_destroy(wal);
}

void test_wal_append_entry() {
    printf("\n" YELLOW "=== Test: WAL Append Entry ===" RESET "\n");
    
    WriteAheadLog *wal = wal_create("test_wal2.log");
    
    uint64_t lsn = wal_append_entry(wal, LOG_BEGIN, 1, 0, 0, "", "");
    assert_equal(lsn, 1, "First LSN is 1");
    assert_equal(wal->entry_count, 1, "Entry count is 1");
    
    lsn = wal_append_entry(wal, LOG_WRITE, 1, 1, 0, "old", "new");
    assert_equal(lsn, 2, "Second LSN is 2");
    assert_equal(wal->entry_count, 2, "Entry count is 2");
    
    lsn = wal_append_entry(wal, LOG_COMMIT, 1, 0, 0, "", "");
    assert_equal(lsn, 3, "Third LSN is 3");
    assert_equal(wal->entry_count, 3, "Entry count is 3");
    
    wal_destroy(wal);
}

void test_wal_get_entry() {
    printf("\n" YELLOW "=== Test: WAL Get Entry ===" RESET "\n");
    
    WriteAheadLog *wal = wal_create("test_wal3.log");
    
    wal_append_entry(wal, LOG_BEGIN, 1, 0, 0, "", "");
    wal_append_entry(wal, LOG_WRITE, 1, 1, 0, "old", "new");
    wal_append_entry(wal, LOG_COMMIT, 1, 0, 0, "", "");
    
    LogEntry *entry = wal_get_entry(wal, 1);
    assert_not_null(entry, "Entry 1 exists");
    assert_equal(entry->type, LOG_BEGIN, "Entry 1 is LOG_BEGIN");
    assert_equal(entry->txn_id, 1, "Entry 1 txn_id is 1");
    
    entry = wal_get_entry(wal, 2);
    assert_not_null(entry, "Entry 2 exists");
    assert_equal(entry->type, LOG_WRITE, "Entry 2 is LOG_WRITE");
    assert_equal(entry->page_id, 1, "Entry 2 page_id is 1");
    
    wal_destroy(wal);
}

void test_wal_flush() {
    printf("\n" YELLOW "=== Test: WAL Flush ===" RESET "\n");
    
    WriteAheadLog *wal = wal_create("test_wal4.log");
    
    wal_append_entry(wal, LOG_BEGIN, 1, 0, 0, "", "");
    wal_append_entry(wal, LOG_WRITE, 1, 1, 0, "old", "new");
    
    assert_equal(wal->flushed_lsn, 0, "Flushed LSN is 0 initially");
    
    int result = wal_flush(wal);
    assert_equal(result, 0, "Flush succeeds");
    assert_equal(wal->flushed_lsn, 2, "Flushed LSN is 2");
    
    wal_destroy(wal);
}

void test_wal_flush_to_lsn() {
    printf("\n" YELLOW "=== Test: WAL Flush to LSN ===" RESET "\n");
    
    WriteAheadLog *wal = wal_create("test_wal5.log");
    
    wal_append_entry(wal, LOG_BEGIN, 1, 0, 0, "", "");
    wal_append_entry(wal, LOG_WRITE, 1, 1, 0, "old", "new");
    wal_append_entry(wal, LOG_WRITE, 1, 2, 0, "old2", "new2");
    
    int result = wal_flush_to_lsn(wal, 2);
    assert_equal(result, 0, "Flush to LSN succeeds");
    assert_equal(wal->flushed_lsn, 2, "Flushed LSN is 2");
    
    wal_destroy(wal);
}

/* ============================================================================
 * CHECKPOINT TESTS
 * ============================================================================ */

void test_checkpoint_create_destroy() {
    printf("\n" YELLOW "=== Test: Checkpoint Create/Destroy ===" RESET "\n");
    
    CheckpointManager *cp_mgr = checkpoint_manager_create("test_cp.log");
    assert_not_null(cp_mgr, "Checkpoint manager created");
    assert_equal(cp_mgr->checkpoint_count, 0, "Checkpoint count is 0");
    
    checkpoint_manager_destroy(cp_mgr);
}

void test_checkpoint_create_checkpoint() {
    printf("\n" YELLOW "=== Test: Checkpoint Create Checkpoint ===" RESET "\n");
    
    CheckpointManager *cp_mgr = checkpoint_manager_create("test_cp2.log");
    
    int result = checkpoint_manager_create_checkpoint(cp_mgr, 1, 100);
    assert_equal(result, 0, "Checkpoint created");
    assert_equal(cp_mgr->checkpoint_count, 1, "Checkpoint count is 1");
    
    result = checkpoint_manager_create_checkpoint(cp_mgr, 2, 200);
    assert_equal(result, 0, "Second checkpoint created");
    assert_equal(cp_mgr->checkpoint_count, 2, "Checkpoint count is 2");
    
    checkpoint_manager_destroy(cp_mgr);
}

void test_checkpoint_get_last_checkpoint() {
    printf("\n" YELLOW "=== Test: Checkpoint Get Last Checkpoint ===" RESET "\n");
    
    CheckpointManager *cp_mgr = checkpoint_manager_create("test_cp3.log");
    
    checkpoint_manager_create_checkpoint(cp_mgr, 1, 100);
    checkpoint_manager_create_checkpoint(cp_mgr, 2, 200);
    
    Checkpoint *cp = checkpoint_manager_get_last_checkpoint(cp_mgr);
    assert_not_null(cp, "Last checkpoint exists");
    assert_equal(cp->txn_id, 2, "Last checkpoint txn_id is 2");
    assert_equal(cp->lsn, 200, "Last checkpoint LSN is 200");
    
    checkpoint_manager_destroy(cp_mgr);
}

/* ============================================================================
 * LOG MANAGER TESTS
 * ============================================================================ */

void test_log_manager_create_destroy() {
    printf("\n" YELLOW "=== Test: Log Manager Create/Destroy ===" RESET "\n");
    
    LogManager *log_mgr = log_manager_create("test_log.log");
    assert_not_null(log_mgr, "Log manager created");
    
    log_manager_destroy(log_mgr);
}

void test_log_manager_begin_transaction() {
    printf("\n" YELLOW "=== Test: Log Manager Begin Transaction ===" RESET "\n");
    
    LogManager *log_mgr = log_manager_create("test_log2.log");
    
    uint64_t lsn = log_manager_begin_transaction(log_mgr, 1);
    assert_true(lsn > 0, "Begin transaction returns valid LSN");
    assert_equal(log_mgr->wal->entry_count, 1, "WAL entry count is 1");
    
    log_manager_destroy(log_mgr);
}

void test_log_manager_log_write() {
    printf("\n" YELLOW "=== Test: Log Manager Log Write ===" RESET "\n");
    
    LogManager *log_mgr = log_manager_create("test_log3.log");
    
    log_manager_begin_transaction(log_mgr, 1);
    uint64_t lsn = log_manager_log_write(log_mgr, 1, 1, 0, "old", "new");
    assert_true(lsn > 0, "Log write returns valid LSN");
    assert_equal(log_mgr->wal->entry_count, 2, "WAL entry count is 2");
    
    log_manager_destroy(log_mgr);
}

void test_log_manager_commit_transaction() {
    printf("\n" YELLOW "=== Test: Log Manager Commit Transaction ===" RESET "\n");
    
    LogManager *log_mgr = log_manager_create("test_log4.log");
    
    log_manager_begin_transaction(log_mgr, 1);
    log_manager_log_write(log_mgr, 1, 1, 0, "old", "new");
    
    uint64_t lsn = log_manager_commit_transaction(log_mgr, 1);
    assert_true(lsn > 0, "Commit transaction returns valid LSN");
    assert_equal(log_mgr->wal->entry_count, 3, "WAL entry count is 3");
    
    log_manager_destroy(log_mgr);
}

void test_log_manager_abort_transaction() {
    printf("\n" YELLOW "=== Test: Log Manager Abort Transaction ===" RESET "\n");
    
    LogManager *log_mgr = log_manager_create("test_log5.log");
    
    log_manager_begin_transaction(log_mgr, 1);
    log_manager_log_write(log_mgr, 1, 1, 0, "old", "new");
    
    uint64_t lsn = log_manager_abort_transaction(log_mgr, 1);
    assert_true(lsn > 0, "Abort transaction returns valid LSN");
    assert_equal(log_mgr->wal->entry_count, 3, "WAL entry count is 3");
    
    log_manager_destroy(log_mgr);
}

void test_log_manager_create_checkpoint() {
    printf("\n" YELLOW "=== Test: Log Manager Create Checkpoint ===" RESET "\n");
    
    LogManager *log_mgr = log_manager_create("test_log6.log");
    
    log_manager_begin_transaction(log_mgr, 1);
    log_manager_log_write(log_mgr, 1, 1, 0, "old", "new");
    log_manager_commit_transaction(log_mgr, 1);
    
    uint64_t lsn = log_manager_create_checkpoint(log_mgr);
    assert_true(lsn > 0, "Create checkpoint returns valid LSN");
    assert_equal(log_mgr->checkpoint_mgr->checkpoint_count, 1, "Checkpoint count is 1");
    
    log_manager_destroy(log_mgr);
}

/* ============================================================================
 * RECOVERY TESTS
 * ============================================================================ */

void test_recovery_manager_create_destroy() {
    printf("\n" YELLOW "=== Test: Recovery Manager Create/Destroy ===" RESET "\n");
    
    WriteAheadLog *wal = wal_create("test_recovery_wal.log");
    CheckpointManager *cp_mgr = checkpoint_manager_create("test_recovery_cp.log");
    
    RecoveryManager *rec_mgr = recovery_manager_create(wal, cp_mgr);
    assert_not_null(rec_mgr, "Recovery manager created");
    assert_equal(rec_mgr->recovered, 0, "Not recovered initially");
    
    recovery_manager_destroy(rec_mgr);
    checkpoint_manager_destroy(cp_mgr);
    wal_destroy(wal);
}

void test_recovery_manager_analysis_phase() {
    printf("\n" YELLOW "=== Test: Recovery Manager Analysis Phase ===" RESET "\n");
    
    WriteAheadLog *wal = wal_create("test_recovery_wal2.log");
    CheckpointManager *cp_mgr = checkpoint_manager_create("test_recovery_cp2.log");
    
    /* Simulate log entries */
    wal_append_entry(wal, LOG_BEGIN, 1, 0, 0, "", "");
    wal_append_entry(wal, LOG_WRITE, 1, 1, 0, "old", "new");
    wal_append_entry(wal, LOG_COMMIT, 1, 0, 0, "", "");
    
    RecoveryManager *rec_mgr = recovery_manager_create(wal, cp_mgr);
    
    int result = recovery_manager_analysis_phase(rec_mgr);
    assert_equal(result, 0, "Analysis phase succeeds");
    
    recovery_manager_destroy(rec_mgr);
    checkpoint_manager_destroy(cp_mgr);
    wal_destroy(wal);
}

void test_recovery_manager_redo_phase() {
    printf("\n" YELLOW "=== Test: Recovery Manager Redo Phase ===" RESET "\n");
    
    WriteAheadLog *wal = wal_create("test_recovery_wal3.log");
    CheckpointManager *cp_mgr = checkpoint_manager_create("test_recovery_cp3.log");
    
    wal_append_entry(wal, LOG_BEGIN, 1, 0, 0, "", "");
    wal_append_entry(wal, LOG_WRITE, 1, 1, 0, "old", "new");
    wal_append_entry(wal, LOG_COMMIT, 1, 0, 0, "", "");
    
    RecoveryManager *rec_mgr = recovery_manager_create(wal, cp_mgr);
    recovery_manager_analysis_phase(rec_mgr);
    
    int result = recovery_manager_redo_phase(rec_mgr);
    assert_equal(result, 0, "Redo phase succeeds");
    
    recovery_manager_destroy(rec_mgr);
    checkpoint_manager_destroy(cp_mgr);
    wal_destroy(wal);
}

void test_recovery_manager_undo_phase() {
    printf("\n" YELLOW "=== Test: Recovery Manager Undo Phase ===" RESET "\n");
    
    WriteAheadLog *wal = wal_create("test_recovery_wal4.log");
    CheckpointManager *cp_mgr = checkpoint_manager_create("test_recovery_cp4.log");
    
    wal_append_entry(wal, LOG_BEGIN, 1, 0, 0, "", "");
    wal_append_entry(wal, LOG_WRITE, 1, 1, 0, "old", "new");
    /* No commit - transaction is uncommitted */
    
    RecoveryManager *rec_mgr = recovery_manager_create(wal, cp_mgr);
    recovery_manager_analysis_phase(rec_mgr);
    
    int result = recovery_manager_undo_phase(rec_mgr);
    assert_equal(result, 0, "Undo phase succeeds");
    
    recovery_manager_destroy(rec_mgr);
    checkpoint_manager_destroy(cp_mgr);
    wal_destroy(wal);
}

void test_recovery_manager_full_recovery() {
    printf("\n" YELLOW "=== Test: Recovery Manager Full Recovery ===" RESET "\n");
    
    WriteAheadLog *wal = wal_create("test_recovery_wal5.log");
    CheckpointManager *cp_mgr = checkpoint_manager_create("test_recovery_cp5.log");
    
    /* Simulate mixed committed and uncommitted transactions */
    wal_append_entry(wal, LOG_BEGIN, 1, 0, 0, "", "");
    wal_append_entry(wal, LOG_WRITE, 1, 1, 0, "old1", "new1");
    wal_append_entry(wal, LOG_COMMIT, 1, 0, 0, "", "");
    
    wal_append_entry(wal, LOG_BEGIN, 2, 0, 0, "", "");
    wal_append_entry(wal, LOG_WRITE, 2, 2, 0, "old2", "new2");
    /* Transaction 2 not committed - will be undone */
    
    RecoveryManager *rec_mgr = recovery_manager_create(wal, cp_mgr);
    
    int result = recovery_manager_recover(rec_mgr);
    assert_equal(result, 0, "Full recovery succeeds");
    assert_equal(recovery_manager_is_recovered(rec_mgr), 1, "Recovery completed");
    
    recovery_manager_destroy(rec_mgr);
    checkpoint_manager_destroy(cp_mgr);
    wal_destroy(wal);
}

/* ============================================================================
 * INTEGRATION TESTS
 * ============================================================================ */

void test_transaction_with_wal() {
    printf("\n" YELLOW "=== Test: Transaction with WAL ===" RESET "\n");
    
    LogManager *log_mgr = log_manager_create("test_integration1.log");
    
    /* Create transaction */
    log_manager_begin_transaction(log_mgr, 1);
    
    /* Add operations */
    log_manager_log_write(log_mgr, 1, 1, 0, "old1", "new1");
    log_manager_log_write(log_mgr, 1, 2, 0, "old2", "new2");
    
    /* Commit */
    log_manager_commit_transaction(log_mgr, 1);
    
    assert_equal(log_mgr->wal->entry_count, 4, "WAL has 4 entries");
    
    log_manager_destroy(log_mgr);
}

void test_multiple_transactions() {
    printf("\n" YELLOW "=== Test: Multiple Transactions ===" RESET "\n");
    
    LogManager *log_mgr = log_manager_create("test_integration2.log");
    
    /* Transaction 1 */
    log_manager_begin_transaction(log_mgr, 1);
    log_manager_log_write(log_mgr, 1, 1, 0, "old1", "new1");
    log_manager_commit_transaction(log_mgr, 1);
    
    /* Transaction 2 */
    log_manager_begin_transaction(log_mgr, 2);
    log_manager_log_write(log_mgr, 2, 2, 0, "old2", "new2");
    log_manager_commit_transaction(log_mgr, 2);
    
    assert_equal(log_mgr->wal->entry_count, 6, "WAL has 6 entries");
    
    log_manager_destroy(log_mgr);
}

void test_checkpoint_and_recovery() {
    printf("\n" YELLOW "=== Test: Checkpoint and Recovery ===" RESET "\n");
    
    LogManager *log_mgr = log_manager_create("test_integration3.log");
    
    /* Transaction 1 */
    log_manager_begin_transaction(log_mgr, 1);
    log_manager_log_write(log_mgr, 1, 1, 0, "old1", "new1");
    log_manager_commit_transaction(log_mgr, 1);
    
    /* Create checkpoint */
    log_manager_create_checkpoint(log_mgr);
    
    /* Transaction 2 */
    log_manager_begin_transaction(log_mgr, 2);
    log_manager_log_write(log_mgr, 2, 2, 0, "old2", "new2");
    log_manager_commit_transaction(log_mgr, 2);
    
    assert_equal(log_mgr->checkpoint_mgr->checkpoint_count, 1, "Checkpoint created");
    
    log_manager_destroy(log_mgr);
}

/* ============================================================================
 * MAIN TEST RUNNER
 * ============================================================================ */

int main(void) {
    printf("\n" YELLOW "╔════════════════════════════════════════════════════════════╗\n" RESET);
    printf(YELLOW "║    Transaction & WAL Test Suite - Phase 5                  ║\n" RESET);
    printf(YELLOW "╚════════════════════════════════════════════════════════════╝\n" RESET);
    
    /* Transaction tests */
    test_transaction_create_destroy();
    test_transaction_add_operation();
    test_transaction_commit();
    test_transaction_abort();
    test_transaction_get_operation();
    
    /* WAL tests */
    test_wal_create_destroy();
    test_wal_append_entry();
    test_wal_get_entry();
    test_wal_flush();
    test_wal_flush_to_lsn();
    
    /* Checkpoint tests */
    test_checkpoint_create_destroy();
    test_checkpoint_create_checkpoint();
    test_checkpoint_get_last_checkpoint();
    
    /* Log manager tests */
    test_log_manager_create_destroy();
    test_log_manager_begin_transaction();
    test_log_manager_log_write();
    test_log_manager_commit_transaction();
    test_log_manager_abort_transaction();
    test_log_manager_create_checkpoint();
    
    /* Recovery tests */
    test_recovery_manager_create_destroy();
    test_recovery_manager_analysis_phase();
    test_recovery_manager_redo_phase();
    test_recovery_manager_undo_phase();
    test_recovery_manager_full_recovery();
    
    /* Integration tests */
    test_transaction_with_wal();
    test_multiple_transactions();
    test_checkpoint_and_recovery();
    
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
