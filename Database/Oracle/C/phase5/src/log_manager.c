/*
 * log_manager.c - Log Manager Implementation
 * 
 * Manages all logging operations including WAL and checkpoints.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "log_manager.h"
#include <stdlib.h>
#include <stdio.h>

/* ============================================================================
 * LOG MANAGER CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * log_manager_create - Create log manager
 */
LogManager* log_manager_create(const char *wal_filename) {
    if (!wal_filename) {
        fprintf(stderr, "ERROR: WAL filename is NULL\n");
        return NULL;
    }
    
    /* Allocate memory for log manager */
    LogManager *mgr = (LogManager *)malloc(sizeof(LogManager));
    if (!mgr) {
        fprintf(stderr, "ERROR: Failed to allocate memory for log manager\n");
        return NULL;
    }
    
    /* Create WAL */
    mgr->wal = wal_create(wal_filename);
    if (!mgr->wal) {
        fprintf(stderr, "ERROR: Failed to create WAL\n");
        free(mgr);
        return NULL;
    }
    
    /* Create checkpoint manager */
    mgr->checkpoint_mgr = checkpoint_manager_create();
    if (!mgr->checkpoint_mgr) {
        fprintf(stderr, "ERROR: Failed to create checkpoint manager\n");
        wal_destroy(mgr->wal);
        free(mgr);
        return NULL;
    }
    
    /* Initialize transaction ID counter */
    mgr->next_txn_id = 1;
    
    return mgr;
}

/*
 * log_manager_destroy - Free log manager
 */
void log_manager_destroy(LogManager *mgr) {
    if (!mgr) return;
    
    if (mgr->wal) {
        wal_destroy(mgr->wal);
    }
    
    if (mgr->checkpoint_mgr) {
        checkpoint_manager_destroy(mgr->checkpoint_mgr);
    }
    
    free(mgr);
}

/* ============================================================================
 * TRANSACTION LOGGING
 * ============================================================================ */

/*
 * log_manager_begin_transaction - Begin transaction
 */
uint32_t log_manager_begin_transaction(LogManager *mgr) {
    if (!mgr || !mgr->wal) {
        fprintf(stderr, "ERROR: Invalid log manager\n");
        return (uint32_t)-1;
    }
    
    uint32_t txn_id = mgr->next_txn_id++;
    
    /* Log BEGIN entry */
    wal_append_entry(mgr->wal, LOG_BEGIN, txn_id, 0, 0, NULL, NULL);
    
    return txn_id;
}

/*
 * log_manager_log_write - Log write operation
 */
uint64_t log_manager_log_write(LogManager *mgr, uint32_t txn_id,
                               uint32_t page_id, uint32_t offset,
                               const char *old_value, const char *new_value) {
    if (!mgr || !mgr->wal) {
        fprintf(stderr, "ERROR: Invalid log manager\n");
        return 0;
    }
    
    /* Log WRITE entry */
    return wal_append_entry(mgr->wal, LOG_WRITE, txn_id, page_id,
                           offset, old_value, new_value);
}

/*
 * log_manager_commit_transaction - Commit transaction
 */
uint64_t log_manager_commit_transaction(LogManager *mgr, uint32_t txn_id) {
    if (!mgr || !mgr->wal) {
        fprintf(stderr, "ERROR: Invalid log manager\n");
        return 0;
    }
    
    /* Log COMMIT entry */
    uint64_t lsn = wal_append_entry(mgr->wal, LOG_COMMIT, txn_id, 0, 0, NULL, NULL);
    
    /* Flush log to ensure durability */
    wal_flush_to_lsn(mgr->wal, lsn);
    
    return lsn;
}

/*
 * log_manager_abort_transaction - Abort transaction
 */
uint64_t log_manager_abort_transaction(LogManager *mgr, uint32_t txn_id) {
    if (!mgr || !mgr->wal) {
        fprintf(stderr, "ERROR: Invalid log manager\n");
        return 0;
    }
    
    /* Log ABORT entry */
    return wal_append_entry(mgr->wal, LOG_ABORT, txn_id, 0, 0, NULL, NULL);
}

/* ============================================================================
 * LOG FLUSH OPERATIONS
 * ============================================================================ */

/*
 * log_manager_flush - Flush log to disk
 */
int log_manager_flush(LogManager *mgr) {
    if (!mgr || !mgr->wal) {
        fprintf(stderr, "ERROR: Invalid log manager\n");
        return -1;
    }
    
    return wal_flush(mgr->wal);
}

/* ============================================================================
 * CHECKPOINT OPERATIONS
 * ============================================================================ */

/*
 * log_manager_create_checkpoint - Create checkpoint
 */
uint32_t log_manager_create_checkpoint(LogManager *mgr,
                                       uint32_t *active_txns,
                                       uint32_t active_count,
                                       uint32_t dirty_pages) {
    if (!mgr || !mgr->wal || !mgr->checkpoint_mgr) {
        fprintf(stderr, "ERROR: Invalid log manager\n");
        return 0;
    }
    
    /* Log CHECKPOINT entry */
    uint64_t lsn = wal_append_entry(mgr->wal, LOG_CHECKPOINT, 0, 0, 0, NULL, NULL);
    
    /* Create checkpoint */
    uint32_t checkpoint_id = checkpoint_manager_create_checkpoint(
        mgr->checkpoint_mgr, lsn, active_txns, active_count, dirty_pages);
    
    return checkpoint_id;
}

/* ============================================================================
 * QUERY OPERATIONS
 * ============================================================================ */

/*
 * log_manager_get_wal - Get WAL
 */
WriteAheadLog* log_manager_get_wal(LogManager *mgr) {
    if (!mgr) return NULL;
    return mgr->wal;
}

/*
 * log_manager_get_checkpoint_manager - Get checkpoint manager
 */
CheckpointManager* log_manager_get_checkpoint_manager(LogManager *mgr) {
    if (!mgr) return NULL;
    return mgr->checkpoint_mgr;
}

/* ============================================================================
 * STATISTICS
 * ============================================================================ */

/*
 * log_manager_print_stats - Print statistics
 */
void log_manager_print_stats(LogManager *mgr) {
    if (!mgr) {
        printf("NULL log manager\n");
        return;
    }
    
    printf("Log Manager Statistics:\n");
    printf("  Next Transaction ID: %u\n", mgr->next_txn_id);
    
    if (mgr->wal) {
        printf("  WAL Entries: %u\n", wal_get_entry_count(mgr->wal));
        printf("  Flushed LSN: %lu\n", wal_get_flushed_lsn(mgr->wal));
    }
    
    if (mgr->checkpoint_mgr) {
        printf("  Checkpoints: %u\n", checkpoint_manager_get_count(mgr->checkpoint_mgr));
    }
}
