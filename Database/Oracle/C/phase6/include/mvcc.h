/*
 * mvcc.h - Multi-Version Concurrency Control Header
 * 
 * Implements MVCC for non-blocking reads and writes.
 * Each transaction sees a consistent snapshot of data.
 * 
 * Key Concepts:
 * - Version: Snapshot of data at a point in time
 * - Read Set: Versions visible to transaction
 * - Write Set: Versions created by transaction
 * - Snapshot Isolation: Consistent view of data
 * - Version Chain: History of versions
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef MVCC_H
#define MVCC_H

#include <stdint.h>
#include <time.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Maximum versions per record */
#define MAX_VERSIONS 1000

/* Maximum records */
#define MAX_RECORDS 10000

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * VersionStatus - Status of a version
 */
typedef enum {
    VERSION_ACTIVE,     /* Version is active */
    VERSION_COMMITTED,  /* Version is committed */
    VERSION_ABORTED     /* Version is aborted */
} VersionStatus;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * Version - Represents a version of a record
 * 
 * Fields:
 *   - version_id: Unique version ID
 *   - record_id: Record being versioned
 *   - txn_id: Transaction that created version
 *   - status: Version status
 *   - data: Version data
 *   - timestamp: When version was created
 *   - next_version: Pointer to next version
 */
typedef struct Version {
    uint64_t version_id;            /* Version ID */
    uint32_t record_id;             /* Record ID */
    uint32_t txn_id;                /* Transaction ID */
    VersionStatus status;           /* Version status */
    char data[256];                 /* Version data */
    time_t timestamp;               /* Timestamp */
    struct Version *next_version;   /* Next version */
} Version;

/*
 * Record - Represents a record with version chain
 * 
 * Fields:
 *   - record_id: Record ID
 *   - head_version: First version in chain
 *   - version_count: Number of versions
 */
typedef struct {
    uint32_t record_id;             /* Record ID */
    Version *head_version;          /* Head of version chain */
    uint32_t version_count;         /* Number of versions */
} Record;

/*
 * ReadSet - Set of versions visible to transaction
 * 
 * Fields:
 *   - versions: Array of visible versions
 *   - version_count: Number of versions
 *   - start_txn_id: Start transaction ID
 *   - end_txn_id: End transaction ID
 */
typedef struct {
    Version *versions[MAX_VERSIONS];  /* Visible versions */
    uint32_t version_count;         /* Number of versions */
    uint32_t start_txn_id;          /* Start transaction ID */
    uint32_t end_txn_id;            /* End transaction ID */
} ReadSet;

/*
 * MVCCManager - Manages multi-version concurrency control
 * 
 * Fields:
 *   - records: Array of records
 *   - record_count: Number of records
 *   - next_version_id: Next version ID
 *   - active_transactions: Count of active transactions
 */
typedef struct {
    Record records[MAX_RECORDS];    /* Records */
    uint32_t record_count;          /* Number of records */
    uint64_t next_version_id;       /* Next version ID */
    uint32_t active_transactions;   /* Active transactions */
} MVCCManager;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * mvcc_manager_create - Create MVCC manager
 * 
 * Allocates and initializes a new MVCC manager.
 * 
 * Returns:
 *   Pointer to new MVCCManager, or NULL on error
 */
MVCCManager* mvcc_manager_create(void);

/*
 * mvcc_manager_destroy - Free MVCC manager
 * 
 * Frees the memory allocated for MVCC manager.
 * 
 * Parameters:
 *   mgr: Pointer to MVCCManager
 * 
 * Returns:
 *   void
 */
void mvcc_manager_destroy(MVCCManager *mgr);

/*
 * mvcc_manager_create_version - Create a new version
 * 
 * Creates a new version of a record.
 * 
 * Parameters:
 *   mgr: Pointer to MVCCManager
 *   record_id: Record ID
 *   txn_id: Transaction ID
 *   data: Version data
 * 
 * Returns:
 *   Version ID on success, 0 on error
 */
uint64_t mvcc_manager_create_version(MVCCManager *mgr, uint32_t record_id,
                                     uint32_t txn_id, const char *data);

/*
 * mvcc_manager_read_version - Read a version
 * 
 * Reads the latest committed version of a record.
 * 
 * Parameters:
 *   mgr: Pointer to MVCCManager
 *   record_id: Record ID
 *   txn_id: Transaction ID
 * 
 * Returns:
 *   Pointer to Version, or NULL if not found
 */
Version* mvcc_manager_read_version(MVCCManager *mgr, uint32_t record_id,
                                   uint32_t txn_id);

/*
 * mvcc_manager_commit_version - Commit a version
 * 
 * Marks a version as committed.
 * 
 * Parameters:
 *   mgr: Pointer to MVCCManager
 *   version_id: Version ID
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int mvcc_manager_commit_version(MVCCManager *mgr, uint64_t version_id);

/*
 * mvcc_manager_abort_version - Abort a version
 * 
 * Marks a version as aborted.
 * 
 * Parameters:
 *   mgr: Pointer to MVCCManager
 *   version_id: Version ID
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int mvcc_manager_abort_version(MVCCManager *mgr, uint64_t version_id);

/*
 * mvcc_manager_get_read_set - Get read set for transaction
 * 
 * Returns the set of versions visible to a transaction.
 * 
 * Parameters:
 *   mgr: Pointer to MVCCManager
 *   txn_id: Transaction ID
 * 
 * Returns:
 *   Pointer to ReadSet, or NULL on error
 */
ReadSet* mvcc_manager_get_read_set(MVCCManager *mgr, uint32_t txn_id);

/*
 * mvcc_manager_get_version_count - Get version count for record
 * 
 * Returns the number of versions for a record.
 * 
 * Parameters:
 *   mgr: Pointer to MVCCManager
 *   record_id: Record ID
 * 
 * Returns:
 *   Number of versions
 */
uint32_t mvcc_manager_get_version_count(MVCCManager *mgr, uint32_t record_id);

/*
 * mvcc_manager_garbage_collect - Garbage collect old versions
 * 
 * Removes versions that are no longer needed.
 * 
 * Parameters:
 *   mgr: Pointer to MVCCManager
 *   min_active_txn_id: Minimum active transaction ID
 * 
 * Returns:
 *   Number of versions collected
 */
uint32_t mvcc_manager_garbage_collect(MVCCManager *mgr, uint32_t min_active_txn_id);

/*
 * mvcc_manager_print_versions - Print version information
 * 
 * Prints all versions in human-readable format.
 * 
 * Parameters:
 *   mgr: Pointer to MVCCManager
 * 
 * Returns:
 *   void
 */
void mvcc_manager_print_versions(MVCCManager *mgr);

#endif // MVCC_H
