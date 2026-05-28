/*
 * checkpoint.h - Checkpoint Management Header
 * 
 * Implements checkpoint mechanism for reducing recovery time.
 * Checkpoints mark consistency points in the log.
 * 
 * Key Concepts:
 * - Checkpoint: Consistency point
 * - Fuzzy Checkpoint: Checkpoint while transactions active
 * - Sharp Checkpoint: Checkpoint with no active transactions
 * - Recovery: Start from last checkpoint
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <stdint.h>
#include <time.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Maximum active transactions at checkpoint */
#define MAX_ACTIVE_TXN 100

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * Checkpoint - Represents a checkpoint
 * 
 * Fields:
 *   - checkpoint_id: Unique checkpoint ID
 *   - lsn: Log Sequence Number of checkpoint
 *   - active_txns: Array of active transaction IDs
 *   - active_count: Number of active transactions
 *   - timestamp: When checkpoint was created
 *   - dirty_pages: Number of dirty pages at checkpoint
 */
typedef struct {
    uint32_t checkpoint_id;         /* Checkpoint ID */
    uint64_t lsn;                   /* LSN of checkpoint */
    uint32_t active_txns[MAX_ACTIVE_TXN];  /* Active transactions */
    uint32_t active_count;          /* Number of active transactions */
    time_t timestamp;               /* Timestamp */
    uint32_t dirty_pages;           /* Number of dirty pages */
} Checkpoint;

/*
 * CheckpointManager - Manages checkpoints
 * 
 * Fields:
 *   - checkpoints: Array of checkpoints
 *   - checkpoint_count: Number of checkpoints
 *   - last_checkpoint_id: ID of last checkpoint
 *   - last_checkpoint_lsn: LSN of last checkpoint
 */
typedef struct {
    Checkpoint checkpoints[100];    /* Checkpoints */
    uint32_t checkpoint_count;      /* Number of checkpoints */
    uint32_t last_checkpoint_id;    /* Last checkpoint ID */
    uint64_t last_checkpoint_lsn;   /* Last checkpoint LSN */
} CheckpointManager;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * checkpoint_manager_create - Create checkpoint manager
 * 
 * Allocates and initializes a new checkpoint manager.
 * 
 * Returns:
 *   Pointer to new CheckpointManager, or NULL on error
 */
CheckpointManager* checkpoint_manager_create(void);

/*
 * checkpoint_manager_destroy - Free checkpoint manager
 * 
 * Frees the memory allocated for checkpoint manager.
 * 
 * Parameters:
 *   mgr: Pointer to CheckpointManager
 * 
 * Returns:
 *   void
 */
void checkpoint_manager_destroy(CheckpointManager *mgr);

/*
 * checkpoint_manager_create_checkpoint - Create new checkpoint
 * 
 * Creates a new checkpoint at the given LSN.
 * 
 * Parameters:
 *   mgr: Pointer to CheckpointManager
 *   lsn: Log Sequence Number
 *   active_txns: Array of active transaction IDs
 *   active_count: Number of active transactions
 *   dirty_pages: Number of dirty pages
 * 
 * Returns:
 *   Checkpoint ID, or 0 on error
 */
uint32_t checkpoint_manager_create_checkpoint(CheckpointManager *mgr,
                                              uint64_t lsn,
                                              uint32_t *active_txns,
                                              uint32_t active_count,
                                              uint32_t dirty_pages);

/*
 * checkpoint_manager_get_last_checkpoint - Get last checkpoint
 * 
 * Returns the last checkpoint created.
 * 
 * Parameters:
 *   mgr: Pointer to CheckpointManager
 * 
 * Returns:
 *   Pointer to Checkpoint, or NULL if no checkpoints
 */
Checkpoint* checkpoint_manager_get_last_checkpoint(CheckpointManager *mgr);

/*
 * checkpoint_manager_get_checkpoint - Get checkpoint by ID
 * 
 * Returns the checkpoint with the given ID.
 * 
 * Parameters:
 *   mgr: Pointer to CheckpointManager
 *   checkpoint_id: Checkpoint ID
 * 
 * Returns:
 *   Pointer to Checkpoint, or NULL if not found
 */
Checkpoint* checkpoint_manager_get_checkpoint(CheckpointManager *mgr,
                                              uint32_t checkpoint_id);

/*
 * checkpoint_manager_get_count - Get number of checkpoints
 * 
 * Returns the number of checkpoints.
 * 
 * Parameters:
 *   mgr: Pointer to CheckpointManager
 * 
 * Returns:
 *   Number of checkpoints
 */
uint32_t checkpoint_manager_get_count(CheckpointManager *mgr);

/*
 * checkpoint_manager_print - Print checkpoints
 * 
 * Prints all checkpoints in human-readable format.
 * 
 * Parameters:
 *   mgr: Pointer to CheckpointManager
 * 
 * Returns:
 *   void
 */
void checkpoint_manager_print(CheckpointManager *mgr);

#endif // CHECKPOINT_H
