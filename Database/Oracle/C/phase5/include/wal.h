/*
 * wal.h - Write Ahead Logging Header
 * 
 * Implements Write Ahead Logging (WAL) for durability.
 * All changes are logged before being written to disk.
 * 
 * Key Concepts:
 * - WAL: Log before write principle
 * - Log Entry: Record of an operation
 * - Flush: Write log to disk
 * - Commit: Mark operation as durable
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef WAL_H
#define WAL_H

#include <stdint.h>
#include <time.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Maximum log entries */
#define MAX_LOG_ENTRIES 10000

/* Log entry size */
#define LOG_ENTRY_SIZE 512

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * LogEntryType - Type of log entry
 */
typedef enum {
    LOG_BEGIN,          /* Begin transaction */
    LOG_WRITE,          /* Write operation */
    LOG_COMMIT,         /* Commit transaction */
    LOG_ABORT,          /* Abort transaction */
    LOG_CHECKPOINT      /* Checkpoint */
} LogEntryType;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * LogEntry - Single entry in write ahead log
 * 
 * Fields:
 *   - lsn: Log Sequence Number (unique ID)
 *   - type: Type of log entry
 *   - txn_id: Transaction ID
 *   - page_id: Page ID (for write operations)
 *   - offset: Offset in page
 *   - old_value: Old value (for undo)
 *   - new_value: New value (for redo)
 *   - timestamp: When entry was created
 *   - is_flushed: Whether entry is flushed to disk
 */
typedef struct {
    uint64_t lsn;                   /* Log Sequence Number */
    LogEntryType type;              /* Entry type */
    uint32_t txn_id;                /* Transaction ID */
    uint32_t page_id;               /* Page ID */
    uint32_t offset;                /* Offset in page */
    char old_value[256];            /* Old value */
    char new_value[256];            /* New value */
    time_t timestamp;               /* Timestamp */
    int is_flushed;                 /* Flushed to disk */
} LogEntry;

/*
 * WriteAheadLog - Write ahead log manager
 * 
 * Fields:
 *   - entries: Array of log entries
 *   - entry_count: Number of entries
 *   - next_lsn: Next log sequence number
 *   - flushed_lsn: Last flushed LSN
 *   - filename: Log file name
 */
typedef struct {
    LogEntry entries[MAX_LOG_ENTRIES];  /* Log entries */
    uint32_t entry_count;           /* Number of entries */
    uint64_t next_lsn;              /* Next LSN */
    uint64_t flushed_lsn;           /* Flushed LSN */
    char filename[256];             /* Log filename */
} WriteAheadLog;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * wal_create - Create a new write ahead log
 * 
 * Allocates and initializes a new WAL.
 * 
 * Parameters:
 *   filename: Log file name
 * 
 * Returns:
 *   Pointer to new WriteAheadLog, or NULL on error
 */
WriteAheadLog* wal_create(const char *filename);

/*
 * wal_destroy - Free WAL memory
 * 
 * Frees the memory allocated for WAL.
 * 
 * Parameters:
 *   wal: Pointer to WriteAheadLog
 * 
 * Returns:
 *   void
 */
void wal_destroy(WriteAheadLog *wal);

/*
 * wal_append_entry - Append entry to log
 * 
 * Appends a new entry to the write ahead log.
 * 
 * Parameters:
 *   wal: Pointer to WriteAheadLog
 *   type: Log entry type
 *   txn_id: Transaction ID
 *   page_id: Page ID
 *   offset: Offset in page
 *   old_value: Old value
 *   new_value: New value
 * 
 * Returns:
 *   LSN of new entry, or 0 on error
 */
uint64_t wal_append_entry(WriteAheadLog *wal, LogEntryType type,
                          uint32_t txn_id, uint32_t page_id,
                          uint32_t offset, const char *old_value,
                          const char *new_value);

/*
 * wal_flush - Flush log to disk
 * 
 * Writes all unflushed entries to disk.
 * 
 * Parameters:
 *   wal: Pointer to WriteAheadLog
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int wal_flush(WriteAheadLog *wal);

/*
 * wal_flush_to_lsn - Flush log up to LSN
 * 
 * Flushes all entries up to the given LSN.
 * 
 * Parameters:
 *   wal: Pointer to WriteAheadLog
 *   lsn: Target LSN
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int wal_flush_to_lsn(WriteAheadLog *wal, uint64_t lsn);

/*
 * wal_get_entry - Get log entry by LSN
 * 
 * Returns the log entry with the given LSN.
 * 
 * Parameters:
 *   wal: Pointer to WriteAheadLog
 *   lsn: Log Sequence Number
 * 
 * Returns:
 *   Pointer to LogEntry, or NULL if not found
 */
LogEntry* wal_get_entry(WriteAheadLog *wal, uint64_t lsn);

/*
 * wal_get_entry_count - Get number of entries
 * 
 * Returns the number of entries in the log.
 * 
 * Parameters:
 *   wal: Pointer to WriteAheadLog
 * 
 * Returns:
 *   Number of entries
 */
uint32_t wal_get_entry_count(WriteAheadLog *wal);

/*
 * wal_get_flushed_lsn - Get flushed LSN
 * 
 * Returns the LSN of the last flushed entry.
 * 
 * Parameters:
 *   wal: Pointer to WriteAheadLog
 * 
 * Returns:
 *   Flushed LSN
 */
uint64_t wal_get_flushed_lsn(WriteAheadLog *wal);

/*
 * wal_print_entries - Print log entries
 * 
 * Prints all log entries in human-readable format.
 * 
 * Parameters:
 *   wal: Pointer to WriteAheadLog
 * 
 * Returns:
 *   void
 */
void wal_print_entries(WriteAheadLog *wal);

#endif // WAL_H
