/*
 * recovery.h - Crash Recovery Header
 * 
 * Implements crash recovery using WAL and checkpoints.
 * Recovers database to consistent state after failure.
 * 
 * Key Concepts:
 * - Analysis Phase: Identify transactions to redo/undo
 * - Redo Phase: Replay committed transactions
 * - Undo Phase: Rollback uncommitted transactions
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef RECOVERY_H
#define RECOVERY_H

#include "wal.h"
#include "checkpoint.h"
#include <stdint.h>

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * RecoveryPhase - Phase of recovery process
 */
typedef enum {
    RECOVERY_ANALYSIS,      /* Analyze log */
    RECOVERY_REDO,          /* Redo committed transactions */
    RECOVERY_UNDO           /* Undo uncommitted transactions */
} RecoveryPhase;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * RecoveryManager - Manages crash recovery
 * 
 * Fields:
 *   - wal: Write ahead log
 *   - checkpoint_mgr: Checkpoint manager
 *   - redo_count: Number of entries to redo
 *   - undo_count: Number of entries to undo
 *   - recovered: Whether recovery completed
 */
typedef struct {
    WriteAheadLog *wal;             /* Write ahead log */
    CheckpointManager *checkpoint_mgr;  /* Checkpoint manager */
    uint32_t redo_count;            /* Entries to redo */
    uint32_t undo_count;            /* Entries to undo */
    int recovered;                  /* Recovery completed */
} RecoveryManager;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * recovery_manager_create - Create recovery manager
 * 
 * Allocates and initializes a new recovery manager.
 * 
 * Parameters:
 *   wal: Pointer to WriteAheadLog
 *   checkpoint_mgr: Pointer to CheckpointManager
 * 
 * Returns:
 *   Pointer to new RecoveryManager, or NULL on error
 */
RecoveryManager* recovery_manager_create(WriteAheadLog *wal,
                                         CheckpointManager *checkpoint_mgr);

/*
 * recovery_manager_destroy - Free recovery manager
 * 
 * Frees the memory allocated for recovery manager.
 * 
 * Parameters:
 *   mgr: Pointer to RecoveryManager
 * 
 * Returns:
 *   void
 */
void recovery_manager_destroy(RecoveryManager *mgr);

/*
 * recovery_manager_recover - Perform crash recovery
 * 
 * Performs complete crash recovery in three phases:
 * 1. Analysis: Identify transactions to redo/undo
 * 2. Redo: Replay committed transactions
 * 3. Undo: Rollback uncommitted transactions
 * 
 * Parameters:
 *   mgr: Pointer to RecoveryManager
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int recovery_manager_recover(RecoveryManager *mgr);

/*
 * recovery_manager_analysis_phase - Analysis phase
 * 
 * Analyzes the log to identify transactions to redo/undo.
 * 
 * Parameters:
 *   mgr: Pointer to RecoveryManager
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int recovery_manager_analysis_phase(RecoveryManager *mgr);

/*
 * recovery_manager_redo_phase - Redo phase
 * 
 * Replays all committed transactions.
 * 
 * Parameters:
 *   mgr: Pointer to RecoveryManager
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int recovery_manager_redo_phase(RecoveryManager *mgr);

/*
 * recovery_manager_undo_phase - Undo phase
 * 
 * Rolls back all uncommitted transactions.
 * 
 * Parameters:
 *   mgr: Pointer to RecoveryManager
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int recovery_manager_undo_phase(RecoveryManager *mgr);

/*
 * recovery_manager_get_redo_count - Get redo count
 * 
 * Returns the number of entries to redo.
 * 
 * Parameters:
 *   mgr: Pointer to RecoveryManager
 * 
 * Returns:
 *   Number of entries to redo
 */
uint32_t recovery_manager_get_redo_count(RecoveryManager *mgr);

/*
 * recovery_manager_get_undo_count - Get undo count
 * 
 * Returns the number of entries to undo.
 * 
 * Parameters:
 *   mgr: Pointer to RecoveryManager
 * 
 * Returns:
 *   Number of entries to undo
 */
uint32_t recovery_manager_get_undo_count(RecoveryManager *mgr);

/*
 * recovery_manager_is_recovered - Check if recovered
 * 
 * Determines if recovery has completed.
 * 
 * Parameters:
 *   mgr: Pointer to RecoveryManager
 * 
 * Returns:
 *   1 if recovered, 0 if not
 */
int recovery_manager_is_recovered(RecoveryManager *mgr);

/*
 * recovery_manager_print_stats - Print statistics
 * 
 * Prints recovery statistics.
 * 
 * Parameters:
 *   mgr: Pointer to RecoveryManager
 * 
 * Returns:
 *   void
 */
void recovery_manager_print_stats(RecoveryManager *mgr);

#endif // RECOVERY_H
