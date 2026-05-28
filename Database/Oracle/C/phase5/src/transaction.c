/*
 * transaction.c - Transaction Management Implementation
 * 
 * Implements transaction creation, state management, and operations.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "transaction.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * TRANSACTION CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * transaction_create - Create a new transaction
 */
Transaction* transaction_create(uint32_t txn_id) {
    /* Allocate memory for transaction */
    Transaction *txn = (Transaction *)malloc(sizeof(Transaction));
    if (!txn) {
        fprintf(stderr, "ERROR: Failed to allocate memory for transaction\n");
        return NULL;
    }
    
    /* Initialize transaction */
    txn->txn_id = txn_id;
    txn->state = TXN_ACTIVE;
    txn->start_time = time(NULL);
    txn->op_count = 0;
    
    return txn;
}

/*
 * transaction_destroy - Free transaction memory
 */
void transaction_destroy(Transaction *txn) {
    if (txn) {
        free(txn);
    }
}

/* ============================================================================
 * TRANSACTION OPERATIONS
 * ============================================================================ */

/*
 * transaction_add_operation - Add operation to transaction
 */
int transaction_add_operation(Transaction *txn, OperationType type,
                              uint32_t page_id, uint32_t offset,
                              const char *old_value, const char *new_value) {
    if (!txn || !old_value || !new_value) {
        fprintf(stderr, "ERROR: Invalid parameters\n");
        return -1;
    }
    
    if (txn->op_count >= MAX_TXN_OPERATIONS) {
        fprintf(stderr, "ERROR: Too many operations in transaction\n");
        return -1;
    }
    
    /* Add operation */
    Operation *op = &txn->operations[txn->op_count];
    op->type = type;
    op->page_id = page_id;
    op->offset = offset;
    strncpy(op->old_value, old_value, 255);
    op->old_value[255] = '\0';
    strncpy(op->new_value, new_value, 255);
    op->new_value[255] = '\0';
    
    txn->op_count++;
    return 0;
}

/* ============================================================================
 * TRANSACTION STATE MANAGEMENT
 * ============================================================================ */

/*
 * transaction_commit - Commit transaction
 */
int transaction_commit(Transaction *txn) {
    if (!txn) return -1;
    
    if (txn->state != TXN_ACTIVE) {
        fprintf(stderr, "ERROR: Cannot commit non-active transaction\n");
        return -1;
    }
    
    txn->state = TXN_COMMITTED;
    return 0;
}

/*
 * transaction_abort - Abort transaction
 */
int transaction_abort(Transaction *txn) {
    if (!txn) return -1;
    
    if (txn->state != TXN_ACTIVE) {
        fprintf(stderr, "ERROR: Cannot abort non-active transaction\n");
        return -1;
    }
    
    txn->state = TXN_ABORTED;
    return 0;
}

/* ============================================================================
 * TRANSACTION QUERY OPERATIONS
 * ============================================================================ */

/*
 * transaction_is_active - Check if transaction is active
 */
int transaction_is_active(Transaction *txn) {
    if (!txn) return 0;
    return txn->state == TXN_ACTIVE;
}

/*
 * transaction_is_committed - Check if transaction is committed
 */
int transaction_is_committed(Transaction *txn) {
    if (!txn) return 0;
    return txn->state == TXN_COMMITTED;
}

/*
 * transaction_get_operation_count - Get number of operations
 */
uint32_t transaction_get_operation_count(Transaction *txn) {
    if (!txn) return 0;
    return txn->op_count;
}

/*
 * transaction_get_operation - Get operation by index
 */
Operation* transaction_get_operation(Transaction *txn, uint32_t index) {
    if (!txn || index >= txn->op_count) {
        return NULL;
    }
    return &txn->operations[index];
}

/* ============================================================================
 * TRANSACTION PRINTING
 * ============================================================================ */

/*
 * transaction_state_to_string - Convert state to string
 */
const char* transaction_state_to_string(TransactionState state) {
    switch (state) {
        case TXN_ACTIVE:    return "ACTIVE";
        case TXN_COMMITTED: return "COMMITTED";
        case TXN_ABORTED:   return "ABORTED";
        default:            return "UNKNOWN";
    }
}

/*
 * transaction_print - Print transaction information
 */
void transaction_print(Transaction *txn) {
    if (!txn) {
        printf("NULL transaction\n");
        return;
    }
    
    printf("Transaction %u:\n", txn->txn_id);
    printf("  State: %s\n", transaction_state_to_string(txn->state));
    printf("  Start Time: %ld\n", txn->start_time);
    printf("  Operations: %u\n", txn->op_count);
    
    for (uint32_t i = 0; i < txn->op_count; i++) {
        Operation *op = &txn->operations[i];
        printf("    Op %u: Page %u, Offset %u\n",
               i, op->page_id, op->offset);
    }
}
