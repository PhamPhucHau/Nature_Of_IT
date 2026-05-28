/*
 * log_manager.h - Log Manager Header
 * 
 * Manages all logging operations including WAL and checkpoints.
 * Coordinates between transaction manager and WAL.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include "wal.h"
#include "checkpoint.h"
#include <stdint.h>

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * LogManager - Manages all logging operations
 * 
 * Fields:
 *   - wal: Write ahead log
 *   - checkpoint_mgr: Checkpoint manager
 *   - next_txn_id: Next transaction ID
 */
typedef struct {
    WriteAheadLog *wal;             /* Write ahead log */
    CheckpointManager *checkpoint_mgr;  /* Checkpoint manager */
    uint32_t next_txn_id;           /* Next transaction ID */
} LogManager;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * log_manager_create - Create log manager
 * 
 * Allocates and initializes a new log manager.
 * 
 * Parameters:
 *   wal_filename: WAL file name
 * 
 * Returns:
 *   Pointer to new LogManager, or NULL on error
 */
LogManager* log_manager_create(const char *wal_filename);

/*
 * log_manager_destroy - Free log manager
 * 
 * Frees the memory allocated for log manager.
 * 
 * Parameters:
 *   mgr: Pointer to LogManager
 * 
 * Returns:
 *   void
 */
void log_manager_destroy(LogManager *mgr);

/*
 * log_manager_begin_transaction - Begin transaction
 * 
 * Logs the beginning of a transaction.
 * 
 * Parameters:
 *   mgr: Pointer to LogManager
 * 
 * Returns:
 *   Transaction ID, or INVALID_TXN_ID on error
 */
uint32_t log_manager_begin_transaction(LogManager *mgr);

/*
 * log_manager_log_write - Log write operation
 * 
 * Logs a write operation.
 * 
 * Parameters:
 *   mgr: Pointer to LogManager
 *   txn_id: Transaction ID
 *   page_id: Page ID
 *   offset: Offset in page
 *   old_value: Old value
 *   new_value: New value
 * 
 * Returns:
 *   LSN of log entry, or 0 on error
 */
uint64_t log_manager_log_write(LogManager *mgr, uint32_t txn_id,
                               uint32_t page_id, uint32_t offset,
                               const char *old_value, const char *new_value);

/*
 * log_manager_commit_transaction - Commit transaction
 * 
 * Logs the commit of a transaction.
 * 
 * Parameters:
 *   mgr: Pointer to LogManager
 *   txn_id: Transaction ID
 * 
 * Returns:
 *   LSN of commit log entry, or 0 on error
 */
uint64_t log_manager_commit_transaction(LogManager *mgr, uint32_t txn_id);

/*
 * log_manager_abort_transaction - Abort transaction
 * 
 * Logs the abort of a transaction.
 * 
 * Parameters:
 *   mgr: Pointer to LogManager
 *   txn_id: Transaction ID
 * 
 * Returns:
 *   LSN of abort log entry, or 0 on error
 */
uint64_t log_manager_abort_transaction(LogManager *mgr, uint32_t txn_id);

/*
 * log_manager_flush - Flush log to disk
 * 
 * Flushes all unflushed log entries to disk.
 * 
 * Parameters:
 *   mgr: Pointer to LogManager
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int log_manager_flush(LogManager *mgr);

/*
 * log_manager_create_checkpoint - Create checkpoint
 * 
 * Creates a new checkpoint.
 * 
 * Parameters:
 *   mgr: Pointer to LogManager
 *   active_txns: Array of active transaction IDs
 *   active_count: Number of active transactions
 *   dirty_pages: Number of dirty pages
 * 
 * Returns:
 *   Checkpoint ID, or 0 on error
 */
uint32_t log_manager_create_checkpoint(LogManager *mgr,
                                       uint32_t *active_txns,
                                       uint32_t active_count,
                                       uint32_t dirty_pages);

/*
 * log_manager_get_wal - Get WAL
 * 
 * Returns the write ahead log.
 * 
 * Parameters:
 *   mgr: Pointer to LogManager
 * 
 * Returns:
 *   Pointer to WriteAheadLog
 */
WriteAheadLog* log_manager_get_wal(LogManager *mgr);

/*
 * log_manager_get_checkpoint_manager - Get checkpoint manager
 * 
 * Returns the checkpoint manager.
 * 
 * Parameters:
 *   mgr: Pointer to LogManager
 * 
 * Returns:
 *   Pointer to CheckpointManager
 */
CheckpointManager* log_manager_get_checkpoint_manager(LogManager *mgr);

/*
 * log_manager_print_stats - Print statistics
 * 
 * Prints log manager statistics.
 * 
 * Parameters:
 *   mgr: Pointer to LogManager
 * 
 * Returns:
 *   void
 */
void log_manager_print_stats(LogManager *mgr);

#endif // LOG_MANAGER_H
