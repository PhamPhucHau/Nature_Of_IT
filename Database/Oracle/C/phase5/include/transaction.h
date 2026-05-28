/*
 * transaction.h - Transaction Management Header
 * 
 * Defines transaction structures and API for transaction management.
 * Transactions are atomic units of work with ACID properties.
 * 
 * Key Concepts:
 * - Transaction: Atomic unit of work
 * - Transaction ID: Unique identifier
 * - Transaction State: Active, Committed, Aborted
 * - Undo/Redo Logs: Recovery information
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stdint.h>
#include <time.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Invalid transaction ID */
#define INVALID_TXN_ID ((uint32_t)-1)

/* Maximum operations per transaction */
#define MAX_TXN_OPERATIONS 1000

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * TransactionState - State of a transaction
 */
typedef enum {
    TXN_ACTIVE,         /* Transaction is active */
    TXN_COMMITTED,      /* Transaction committed */
    TXN_ABORTED         /* Transaction aborted */
} TransactionState;

/*
 * OperationType - Type of operation in transaction
 */
typedef enum {
    OP_INSERT,          /* Insert operation */
    OP_UPDATE,          /* Update operation */
    OP_DELETE           /* Delete operation */
} OperationType;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * Operation - Represents a single operation in a transaction
 * 
 * Fields:
 *   - type: Type of operation
 *   - page_id: Page affected
 *   - offset: Offset in page
 *   - old_value: Value before operation (for undo)
 *   - new_value: Value after operation (for redo)
 */
typedef struct {
    OperationType type;             /* Operation type */
    uint32_t page_id;               /* Page ID */
    uint32_t offset;                /* Offset in page */
    char old_value[256];            /* Old value (undo) */
    char new_value[256];            /* New value (redo) */
} Operation;

/*
 * Transaction - Represents a transaction
 * 
 * Fields:
 *   - txn_id: Unique transaction ID
 *   - state: Current state
 *   - start_time: When transaction started
 *   - operations: Array of operations
 *   - op_count: Number of operations
 */
typedef struct {
    uint32_t txn_id;                /* Transaction ID */
    TransactionState state;         /* Current state */
    time_t start_time;              /* Start time */
    Operation operations[MAX_TXN_OPERATIONS];  /* Operations */
    uint32_t op_count;              /* Number of operations */
} Transaction;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * transaction_create - Create a new transaction
 * 
 * Allocates and initializes a new transaction.
 * 
 * Parameters:
 *   txn_id: Unique transaction ID
 * 
 * Returns:
 *   Pointer to new Transaction, or NULL on error
 */
Transaction* transaction_create(uint32_t txn_id);

/*
 * transaction_destroy - Free transaction memory
 * 
 * Frees the memory allocated for a transaction.
 * 
 * Parameters:
 *   txn: Pointer to Transaction
 * 
 * Returns:
 *   void
 */
void transaction_destroy(Transaction *txn);

/*
 * transaction_add_operation - Add operation to transaction
 * 
 * Adds an operation to the transaction's operation log.
 * 
 * Parameters:
 *   txn: Pointer to Transaction
 *   type: Operation type
 *   page_id: Page ID
 *   offset: Offset in page
 *   old_value: Old value
 *   new_value: New value
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int transaction_add_operation(Transaction *txn, OperationType type,
                              uint32_t page_id, uint32_t offset,
                              const char *old_value, const char *new_value);

/*
 * transaction_commit - Commit transaction
 * 
 * Marks transaction as committed.
 * 
 * Parameters:
 *   txn: Pointer to Transaction
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int transaction_commit(Transaction *txn);

/*
 * transaction_abort - Abort transaction
 * 
 * Marks transaction as aborted.
 * 
 * Parameters:
 *   txn: Pointer to Transaction
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int transaction_abort(Transaction *txn);

/*
 * transaction_is_active - Check if transaction is active
 * 
 * Determines if transaction is currently active.
 * 
 * Parameters:
 *   txn: Pointer to Transaction
 * 
 * Returns:
 *   1 if active, 0 if not
 */
int transaction_is_active(Transaction *txn);

/*
 * transaction_is_committed - Check if transaction is committed
 * 
 * Determines if transaction has been committed.
 * 
 * Parameters:
 *   txn: Pointer to Transaction
 * 
 * Returns:
 *   1 if committed, 0 if not
 */
int transaction_is_committed(Transaction *txn);

/*
 * transaction_get_operation_count - Get number of operations
 * 
 * Returns the number of operations in the transaction.
 * 
 * Parameters:
 *   txn: Pointer to Transaction
 * 
 * Returns:
 *   Number of operations
 */
uint32_t transaction_get_operation_count(Transaction *txn);

/*
 * transaction_get_operation - Get operation by index
 * 
 * Returns the operation at the given index.
 * 
 * Parameters:
 *   txn: Pointer to Transaction
 *   index: Operation index
 * 
 * Returns:
 *   Pointer to Operation, or NULL if index out of bounds
 */
Operation* transaction_get_operation(Transaction *txn, uint32_t index);

/*
 * transaction_print - Print transaction information
 * 
 * Prints transaction information in human-readable format.
 * 
 * Parameters:
 *   txn: Pointer to Transaction
 * 
 * Returns:
 *   void
 */
void transaction_print(Transaction *txn);

/*
 * transaction_state_to_string - Convert state to string
 * 
 * Returns string representation of transaction state.
 * 
 * Parameters:
 *   state: TransactionState enum value
 * 
 * Returns:
 *   String representation
 */
const char* transaction_state_to_string(TransactionState state);

#endif // TRANSACTION_H
