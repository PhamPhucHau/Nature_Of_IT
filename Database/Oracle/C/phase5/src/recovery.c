/*
 * recovery.c - Crash Recovery Implementation
 * 
 * Implements crash recovery using WAL and checkpoints.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "recovery.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * RECOVERY MANAGER CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * recovery_manager_create - Create recovery manager
 */
RecoveryManager* recovery_manager_create(WriteAheadLog *wal,
                                         CheckpointManager *checkpoint_mgr) {
    if (!wal || !checkpoint_mgr) {
        fprintf(stderr, "ERROR: Invalid parameters\n");
        return NULL;
    }
    
    /* Allocate memory for recovery manager */
    RecoveryManager *mgr = (RecoveryManager *)malloc(sizeof(RecoveryManager));
    if (!mgr) {
        fprintf(stderr, "ERROR: Failed to allocate memory for recovery manager\n");
        return NULL;
    }
    
    /* Initialize recovery manager */
    mgr->wal = wal;
    mgr->checkpoint_mgr = checkpoint_mgr;
    mgr->redo_count = 0;
    mgr->undo_count = 0;
    mgr->recovered = 0;
    
    return mgr;
}

/*
 * recovery_manager_destroy - Free recovery manager
 */
void recovery_manager_destroy(RecoveryManager *mgr) {
    if (mgr) {
        free(mgr);
    }
}

/* ============================================================================
 * RECOVERY PHASES
 * ============================================================================ */

/*
 * recovery_manager_analysis_phase - Analysis phase
 * 
 * Analyzes the log to identify transactions to redo/undo.
 */
int recovery_manager_analysis_phase(RecoveryManager *mgr) {
    if (!mgr || !mgr->wal) {
        fprintf(stderr, "ERROR: Invalid recovery manager\n");
        return -1;
    }
    
    printf("Analysis Phase: Analyzing log...\n");
    
    /* Find last checkpoint */
    Checkpoint *last_cp = checkpoint_manager_get_last_checkpoint(mgr->checkpoint_mgr);
    uint64_t start_lsn = 0;
    
    if (last_cp) {
        start_lsn = last_cp->lsn;
        printf("  Starting from checkpoint LSN: %lu\n", start_lsn);
    }
    
    /* Scan log from checkpoint */
    uint32_t redo_count = 0;
    uint32_t undo_count = 0;
    
    for (uint32_t i = 0; i < wal_get_entry_count(mgr->wal); i++) {
        LogEntry *entry = &mgr->wal->entries[i];
        
        if (entry->lsn <= start_lsn) continue;
        
        switch (entry->type) {
            case LOG_COMMIT:
                redo_count++;
                break;
            case LOG_ABORT:
                undo_count++;
                break;
            case LOG_WRITE:
                /* Count writes for redo/undo */
                break;
            default:
                break;
        }
    }
    
    mgr->redo_count = redo_count;
    mgr->undo_count = undo_count;
    
    printf("  Transactions to redo: %u\n", redo_count);
    printf("  Transactions to undo: %u\n", undo_count);
    
    return 0;
}

/*
 * recovery_manager_redo_phase - Redo phase
 * 
 * Replays all committed transactions.
 */
int recovery_manager_redo_phase(RecoveryManager *mgr) {
    if (!mgr || !mgr->wal) {
        fprintf(stderr, "ERROR: Invalid recovery manager\n");
        return -1;
    }
    
    printf("Redo Phase: Replaying committed transactions...\n");
    
    uint32_t redo_count = 0;
    
    /* Scan log and redo committed transactions */
    for (uint32_t i = 0; i < wal_get_entry_count(mgr->wal); i++) {
        LogEntry *entry = &mgr->wal->entries[i];
        
        if (entry->type == LOG_WRITE) {
            /* In a real implementation, would apply the write */
            printf("  Redo: TXN %u, Page %u, Offset %u\n",
                   entry->txn_id, entry->page_id, entry->offset);
            redo_count++;
        }
    }
    
    printf("  Redone %u operations\n", redo_count);
    
    return 0;
}

/*
 * recovery_manager_undo_phase - Undo phase
 * 
 * Rolls back all uncommitted transactions.
 */
int recovery_manager_undo_phase(RecoveryManager *mgr) {
    if (!mgr || !mgr->wal) {
        fprintf(stderr, "ERROR: Invalid recovery manager\n");
        return -1;
    }
    
    printf("Undo Phase: Rolling back uncommitted transactions...\n");
    
    uint32_t undo_count = 0;
    
    /* Scan log in reverse and undo uncommitted transactions */
    for (int i = wal_get_entry_count(mgr->wal) - 1; i >= 0; i--) {
        LogEntry *entry = &mgr->wal->entries[i];
        
        if (entry->type == LOG_WRITE) {
            /* In a real implementation, would apply the undo */
            printf("  Undo: TXN %u, Page %u, Offset %u\n",
                   entry->txn_id, entry->page_id, entry->offset);
            undo_count++;
        }
    }
    
    printf("  Undone %u operations\n", undo_count);
    
    return 0;
}

/* ============================================================================
 * MAIN RECOVERY OPERATION
 * ============================================================================ */

/*
 * recovery_manager_recover - Perform crash recovery
 */
int recovery_manager_recover(RecoveryManager *mgr) {
    if (!mgr) {
        fprintf(stderr, "ERROR: Invalid recovery manager\n");
        return -1;
    }
    
    printf("\n=== CRASH RECOVERY ===\n");
    
    /* Phase 1: Analysis */
    if (recovery_manager_analysis_phase(mgr) != 0) {
        fprintf(stderr, "ERROR: Analysis phase failed\n");
        return -1;
    }
    
    /* Phase 2: Redo */
    if (recovery_manager_redo_phase(mgr) != 0) {
        fprintf(stderr, "ERROR: Redo phase failed\n");
        return -1;
    }
    
    /* Phase 3: Undo */
    if (recovery_manager_undo_phase(mgr) != 0) {
        fprintf(stderr, "ERROR: Undo phase failed\n");
        return -1;
    }
    
    mgr->recovered = 1;
    printf("=== RECOVERY COMPLETE ===\n\n");
    
    return 0;
}

/* ============================================================================
 * QUERY OPERATIONS
 * ============================================================================ */

/*
 * recovery_manager_get_redo_count - Get redo count
 */
uint32_t recovery_manager_get_redo_count(RecoveryManager *mgr) {
    if (!mgr) return 0;
    return mgr->redo_count;
}

/*
 * recovery_manager_get_undo_count - Get undo count
 */
uint32_t recovery_manager_get_undo_count(RecoveryManager *mgr) {
    if (!mgr) return 0;
    return mgr->undo_count;
}

/*
 * recovery_manager_is_recovered - Check if recovered
 */
int recovery_manager_is_recovered(RecoveryManager *mgr) {
    if (!mgr) return 0;
    return mgr->recovered;
}

/* ============================================================================
 * STATISTICS
 * ============================================================================ */

/*
 * recovery_manager_print_stats - Print statistics
 */
void recovery_manager_print_stats(RecoveryManager *mgr) {
    if (!mgr) {
        printf("NULL recovery manager\n");
        return;
    }
    
    printf("Recovery Statistics:\n");
    printf("  Recovered: %s\n", mgr->recovered ? "yes" : "no");
    printf("  Redo Count: %u\n", mgr->redo_count);
    printf("  Undo Count: %u\n", mgr->undo_count);
}
