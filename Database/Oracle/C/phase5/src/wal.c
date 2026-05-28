/*
 * wal.c - Write Ahead Logging Implementation
 * 
 * Implements write ahead logging for durability.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "wal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * WAL CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * wal_create - Create a new write ahead log
 */
WriteAheadLog* wal_create(const char *filename) {
    if (!filename) {
        fprintf(stderr, "ERROR: Filename is NULL\n");
        return NULL;
    }
    
    /* Allocate memory for WAL */
    WriteAheadLog *wal = (WriteAheadLog *)malloc(sizeof(WriteAheadLog));
    if (!wal) {
        fprintf(stderr, "ERROR: Failed to allocate memory for WAL\n");
        return NULL;
    }
    
    /* Initialize WAL */
    strncpy(wal->filename, filename, 255);
    wal->filename[255] = '\0';
    wal->entry_count = 0;
    wal->next_lsn = 1;
    wal->flushed_lsn = 0;
    
    return wal;
}

/*
 * wal_destroy - Free WAL memory
 */
void wal_destroy(WriteAheadLog *wal) {
    if (wal) {
        free(wal);
    }
}

/* ============================================================================
 * LOG ENTRY OPERATIONS
 * ============================================================================ */

/*
 * wal_append_entry - Append entry to log
 */
uint64_t wal_append_entry(WriteAheadLog *wal, LogEntryType type,
                          uint32_t txn_id, uint32_t page_id,
                          uint32_t offset, const char *old_value,
                          const char *new_value) {
    if (!wal) {
        fprintf(stderr, "ERROR: WAL is NULL\n");
        return 0;
    }
    
    if (wal->entry_count >= MAX_LOG_ENTRIES) {
        fprintf(stderr, "ERROR: Log is full\n");
        return 0;
    }
    
    /* Create new log entry */
    LogEntry *entry = &wal->entries[wal->entry_count];
    entry->lsn = wal->next_lsn;
    entry->type = type;
    entry->txn_id = txn_id;
    entry->page_id = page_id;
    entry->offset = offset;
    entry->timestamp = time(NULL);
    entry->is_flushed = 0;
    
    /* Copy values */
    if (old_value) {
        strncpy(entry->old_value, old_value, 255);
        entry->old_value[255] = '\0';
    } else {
        entry->old_value[0] = '\0';
    }
    
    if (new_value) {
        strncpy(entry->new_value, new_value, 255);
        entry->new_value[255] = '\0';
    } else {
        entry->new_value[0] = '\0';
    }
    
    /* Update counters */
    uint64_t lsn = wal->next_lsn;
    wal->next_lsn++;
    wal->entry_count++;
    
    return lsn;
}

/* ============================================================================
 * LOG FLUSH OPERATIONS
 * ============================================================================ */

/*
 * wal_flush - Flush log to disk
 */
int wal_flush(WriteAheadLog *wal) {
    if (!wal) return -1;
    
    /* In a real implementation, this would write to disk */
    /* For now, just mark entries as flushed */
    for (uint32_t i = 0; i < wal->entry_count; i++) {
        if (!wal->entries[i].is_flushed) {
            wal->entries[i].is_flushed = 1;
            wal->flushed_lsn = wal->entries[i].lsn;
        }
    }
    
    return 0;
}

/*
 * wal_flush_to_lsn - Flush log up to LSN
 */
int wal_flush_to_lsn(WriteAheadLog *wal, uint64_t lsn) {
    if (!wal) return -1;
    
    /* Mark entries up to LSN as flushed */
    for (uint32_t i = 0; i < wal->entry_count; i++) {
        if (wal->entries[i].lsn <= lsn) {
            wal->entries[i].is_flushed = 1;
            wal->flushed_lsn = wal->entries[i].lsn;
        }
    }
    
    return 0;
}

/* ============================================================================
 * LOG QUERY OPERATIONS
 * ============================================================================ */

/*
 * wal_get_entry - Get log entry by LSN
 */
LogEntry* wal_get_entry(WriteAheadLog *wal, uint64_t lsn) {
    if (!wal) return NULL;
    
    for (uint32_t i = 0; i < wal->entry_count; i++) {
        if (wal->entries[i].lsn == lsn) {
            return &wal->entries[i];
        }
    }
    
    return NULL;
}

/*
 * wal_get_entry_count - Get number of entries
 */
uint32_t wal_get_entry_count(WriteAheadLog *wal) {
    if (!wal) return 0;
    return wal->entry_count;
}

/*
 * wal_get_flushed_lsn - Get flushed LSN
 */
uint64_t wal_get_flushed_lsn(WriteAheadLog *wal) {
    if (!wal) return 0;
    return wal->flushed_lsn;
}

/* ============================================================================
 * LOG PRINTING
 * ============================================================================ */

/*
 * wal_print_entries - Print log entries
 */
void wal_print_entries(WriteAheadLog *wal) {
    if (!wal) {
        printf("NULL WAL\n");
        return;
    }
    
    printf("Write Ahead Log (%u entries):\n", wal->entry_count);
    printf("  Flushed LSN: %lu\n", wal->flushed_lsn);
    
    for (uint32_t i = 0; i < wal->entry_count; i++) {
        LogEntry *entry = &wal->entries[i];
        printf("  LSN %lu: TXN %u, Type %d, Page %u, Flushed: %s\n",
               entry->lsn, entry->txn_id, entry->type,
               entry->page_id, entry->is_flushed ? "yes" : "no");
    }
}
