/*
 * checkpoint.c - Checkpoint Management Implementation
 * 
 * Implements checkpoint mechanism for reducing recovery time.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "checkpoint.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * CHECKPOINT MANAGER CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * checkpoint_manager_create - Create checkpoint manager
 */
CheckpointManager* checkpoint_manager_create(void) {
    /* Allocate memory for checkpoint manager */
    CheckpointManager *mgr = (CheckpointManager *)malloc(sizeof(CheckpointManager));
    if (!mgr) {
        fprintf(stderr, "ERROR: Failed to allocate memory for checkpoint manager\n");
        return NULL;
    }
    
    /* Initialize checkpoint manager */
    mgr->checkpoint_count = 0;
    mgr->last_checkpoint_id = 0;
    mgr->last_checkpoint_lsn = 0;
    
    return mgr;
}

/*
 * checkpoint_manager_destroy - Free checkpoint manager
 */
void checkpoint_manager_destroy(CheckpointManager *mgr) {
    if (mgr) {
        free(mgr);
    }
}

/* ============================================================================
 * CHECKPOINT OPERATIONS
 * ============================================================================ */

/*
 * checkpoint_manager_create_checkpoint - Create new checkpoint
 */
uint32_t checkpoint_manager_create_checkpoint(CheckpointManager *mgr,
                                              uint64_t lsn,
                                              uint32_t *active_txns,
                                              uint32_t active_count,
                                              uint32_t dirty_pages) {
    if (!mgr) {
        fprintf(stderr, "ERROR: Checkpoint manager is NULL\n");
        return 0;
    }
    
    if (mgr->checkpoint_count >= 100) {
        fprintf(stderr, "ERROR: Too many checkpoints\n");
        return 0;
    }
    
    /* Create new checkpoint */
    Checkpoint *cp = &mgr->checkpoints[mgr->checkpoint_count];
    cp->checkpoint_id = mgr->checkpoint_count + 1;
    cp->lsn = lsn;
    cp->timestamp = time(NULL);
    cp->dirty_pages = dirty_pages;
    cp->active_count = active_count;
    
    /* Copy active transactions */
    if (active_txns && active_count > 0) {
        for (uint32_t i = 0; i < active_count && i < MAX_ACTIVE_TXN; i++) {
            cp->active_txns[i] = active_txns[i];
        }
    }
    
    /* Update manager */
    mgr->checkpoint_count++;
    mgr->last_checkpoint_id = cp->checkpoint_id;
    mgr->last_checkpoint_lsn = lsn;
    
    return cp->checkpoint_id;
}

/* ============================================================================
 * CHECKPOINT QUERY OPERATIONS
 * ============================================================================ */

/*
 * checkpoint_manager_get_last_checkpoint - Get last checkpoint
 */
Checkpoint* checkpoint_manager_get_last_checkpoint(CheckpointManager *mgr) {
    if (!mgr || mgr->checkpoint_count == 0) {
        return NULL;
    }
    
    return &mgr->checkpoints[mgr->checkpoint_count - 1];
}

/*
 * checkpoint_manager_get_checkpoint - Get checkpoint by ID
 */
Checkpoint* checkpoint_manager_get_checkpoint(CheckpointManager *mgr,
                                              uint32_t checkpoint_id) {
    if (!mgr || checkpoint_id == 0) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < mgr->checkpoint_count; i++) {
        if (mgr->checkpoints[i].checkpoint_id == checkpoint_id) {
            return &mgr->checkpoints[i];
        }
    }
    
    return NULL;
}

/*
 * checkpoint_manager_get_count - Get number of checkpoints
 */
uint32_t checkpoint_manager_get_count(CheckpointManager *mgr) {
    if (!mgr) return 0;
    return mgr->checkpoint_count;
}

/* ============================================================================
 * CHECKPOINT PRINTING
 * ============================================================================ */

/*
 * checkpoint_manager_print - Print checkpoints
 */
void checkpoint_manager_print(CheckpointManager *mgr) {
    if (!mgr) {
        printf("NULL checkpoint manager\n");
        return;
    }
    
    printf("Checkpoints (%u total):\n", mgr->checkpoint_count);
    
    for (uint32_t i = 0; i < mgr->checkpoint_count; i++) {
        Checkpoint *cp = &mgr->checkpoints[i];
        printf("  Checkpoint %u:\n", cp->checkpoint_id);
        printf("    LSN: %lu\n", cp->lsn);
        printf("    Time: %ld\n", cp->timestamp);
        printf("    Active Transactions: %u\n", cp->active_count);
        printf("    Dirty Pages: %u\n", cp->dirty_pages);
    }
}
