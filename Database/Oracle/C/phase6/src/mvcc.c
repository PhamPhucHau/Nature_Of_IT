/*
 * mvcc.c - Multi-Version Concurrency Control Implementation
 * 
 * Implements MVCC for non-blocking reads and writes.
 * Each transaction sees a consistent snapshot of data.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "mvcc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * MVCC MANAGER FUNCTIONS
 * ============================================================================ */

MVCCManager* mvcc_manager_create(void) {
    MVCCManager *mgr = (MVCCManager *)malloc(sizeof(MVCCManager));
    if (!mgr) return NULL;
    
    memset(mgr, 0, sizeof(MVCCManager));
    mgr->record_count = 0;
    mgr->next_version_id = 1;
    mgr->active_transactions = 0;
    
    return mgr;
}

void mvcc_manager_destroy(MVCCManager *mgr) {
    if (!mgr) return;
    
    /* Free all versions */
    for (uint32_t i = 0; i < mgr->record_count; i++) {
        Record *record = &mgr->records[i];
        Version *version = record->head_version;
        while (version) {
            Version *next = version->next_version;
            free(version);
            version = next;
        }
    }
    
    free(mgr);
}

static Record* mvcc_manager_get_or_create_record(MVCCManager *mgr, uint32_t record_id) {
    /* Find existing record */
    for (uint32_t i = 0; i < mgr->record_count; i++) {
        if (mgr->records[i].record_id == record_id) {
            return &mgr->records[i];
        }
    }
    
    /* Create new record if space available */
    if (mgr->record_count >= MAX_RECORDS) {
        return NULL;
    }
    
    Record *record = &mgr->records[mgr->record_count];
    record->record_id = record_id;
    record->head_version = NULL;
    record->version_count = 0;
    mgr->record_count++;
    
    return record;
}

uint64_t mvcc_manager_create_version(MVCCManager *mgr, uint32_t record_id,
                                     uint32_t txn_id, const char *data) {
    if (!mgr || !data) return 0;
    
    Record *record = mvcc_manager_get_or_create_record(mgr, record_id);
    if (!record) return 0;
    
    /* Create new version */
    Version *version = (Version *)malloc(sizeof(Version));
    if (!version) return 0;
    
    version->version_id = mgr->next_version_id++;
    version->record_id = record_id;
    version->txn_id = txn_id;
    version->status = VERSION_ACTIVE;
    strncpy(version->data, data, sizeof(version->data) - 1);
    version->data[sizeof(version->data) - 1] = '\0';
    version->timestamp = time(NULL);
    version->next_version = record->head_version;
    
    /* Add to version chain */
    record->head_version = version;
    record->version_count++;
    
    return version->version_id;
}

Version* mvcc_manager_read_version(MVCCManager *mgr, uint32_t record_id,
                                   uint32_t txn_id) {
    if (!mgr) return NULL;
    
    /* Find record */
    Record *record = NULL;
    for (uint32_t i = 0; i < mgr->record_count; i++) {
        if (mgr->records[i].record_id == record_id) {
            record = &mgr->records[i];
            break;
        }
    }
    
    if (!record || !record->head_version) return NULL;
    
    /* Find latest committed version */
    Version *version = record->head_version;
    while (version) {
        if (version->status == VERSION_COMMITTED) {
            return version;
        }
        version = version->next_version;
    }
    
    return NULL;
}

int mvcc_manager_commit_version(MVCCManager *mgr, uint64_t version_id) {
    if (!mgr) return -1;
    
    /* Find version */
    for (uint32_t i = 0; i < mgr->record_count; i++) {
        Record *record = &mgr->records[i];
        Version *version = record->head_version;
        while (version) {
            if (version->version_id == version_id) {
                version->status = VERSION_COMMITTED;
                return 0;
            }
            version = version->next_version;
        }
    }
    
    return -1;
}

int mvcc_manager_abort_version(MVCCManager *mgr, uint64_t version_id) {
    if (!mgr) return -1;
    
    /* Find version */
    for (uint32_t i = 0; i < mgr->record_count; i++) {
        Record *record = &mgr->records[i];
        Version *version = record->head_version;
        while (version) {
            if (version->version_id == version_id) {
                version->status = VERSION_ABORTED;
                return 0;
            }
            version = version->next_version;
        }
    }
    
    return -1;
}

ReadSet* mvcc_manager_get_read_set(MVCCManager *mgr, uint32_t txn_id) {
    if (!mgr) return NULL;
    
    ReadSet *read_set = (ReadSet *)malloc(sizeof(ReadSet));
    if (!read_set) return NULL;
    
    memset(read_set, 0, sizeof(ReadSet));
    read_set->start_txn_id = txn_id;
    read_set->end_txn_id = mgr->next_version_id;
    
    /* Collect all committed versions */
    uint32_t count = 0;
    for (uint32_t i = 0; i < mgr->record_count && count < MAX_VERSIONS; i++) {
        Record *record = &mgr->records[i];
        Version *version = record->head_version;
        while (version && count < MAX_VERSIONS) {
            if (version->status == VERSION_COMMITTED) {
                read_set->versions[count++] = version;
            }
            version = version->next_version;
        }
    }
    
    read_set->version_count = count;
    return read_set;
}

uint32_t mvcc_manager_get_version_count(MVCCManager *mgr, uint32_t record_id) {
    if (!mgr) return 0;
    
    for (uint32_t i = 0; i < mgr->record_count; i++) {
        if (mgr->records[i].record_id == record_id) {
            return mgr->records[i].version_count;
        }
    }
    
    return 0;
}

uint32_t mvcc_manager_garbage_collect(MVCCManager *mgr, uint32_t min_active_txn_id) {
    if (!mgr) return 0;
    
    uint32_t collected = 0;
    
    /* Collect old versions */
    for (uint32_t i = 0; i < mgr->record_count; i++) {
        Record *record = &mgr->records[i];
        Version *version = record->head_version;
        Version *prev = NULL;
        
        while (version) {
            Version *next = version->next_version;
            
            /* Collect aborted versions and old committed versions */
            if (version->status == VERSION_ABORTED ||
                (version->status == VERSION_COMMITTED && version->txn_id < min_active_txn_id)) {
                
                if (prev) {
                    prev->next_version = next;
                } else {
                    record->head_version = next;
                }
                
                free(version);
                record->version_count--;
                collected++;
            } else {
                prev = version;
            }
            
            version = next;
        }
    }
    
    return collected;
}

void mvcc_manager_print_versions(MVCCManager *mgr) {
    if (!mgr) return;
    
    printf("\n=== MVCC Manager Status ===\n");
    printf("Total Records: %u\n", mgr->record_count);
    printf("Next Version ID: %lu\n", mgr->next_version_id);
    printf("Active Transactions: %u\n", mgr->active_transactions);
    
    for (uint32_t i = 0; i < mgr->record_count; i++) {
        Record *record = &mgr->records[i];
        printf("\nRecord %u (%u versions):\n", record->record_id, record->version_count);
        
        Version *version = record->head_version;
        uint32_t count = 0;
        while (version && count < 10) {
            printf("  Version %lu: TXN %u, Status %s, Data: %s\n",
                   version->version_id, version->txn_id,
                   version->status == VERSION_ACTIVE ? "ACTIVE" :
                   version->status == VERSION_COMMITTED ? "COMMITTED" : "ABORTED",
                   version->data);
            version = version->next_version;
            count++;
        }
        
        if (count == 10 && version) {
            printf("  ... and %u more versions\n", record->version_count - 10);
        }
    }
}
