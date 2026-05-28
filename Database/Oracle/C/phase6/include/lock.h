/*
 * lock.h - Lock Manager Header
 * 
 * Implements lock management for concurrency control.
 * Supports shared (read) and exclusive (write) locks.
 * 
 * Key Concepts:
 * - Lock: Synchronization primitive
 * - Shared Lock: Multiple readers
 * - Exclusive Lock: Single writer
 * - Lock Compatibility: Which locks can coexist
 * - Lock Waiting: Queue for lock requests
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef LOCK_H
#define LOCK_H

#include <stdint.h>
#include <time.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Maximum locks */
#define MAX_LOCKS 10000

/* Maximum lock holders */
#define MAX_LOCK_HOLDERS 1000

/* Lock timeout (milliseconds) */
#define LOCK_TIMEOUT 5000

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * LockType - Type of lock
 */
typedef enum {
    LOCK_SHARED,        /* Shared (read) lock */
    LOCK_EXCLUSIVE      /* Exclusive (write) lock */
} LockType;

/*
 * LockStatus - Status of lock request
 */
typedef enum {
    LOCK_GRANTED,       /* Lock granted */
    LOCK_WAITING,       /* Waiting for lock */
    LOCK_DENIED,        /* Lock denied */
    LOCK_TIMEOUT        /* Lock request timed out */
} LockStatus;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * LockRequest - Request for a lock
 * 
 * Fields:
 *   - txn_id: Transaction requesting lock
 *   - resource_id: Resource to lock
 *   - lock_type: Type of lock
 *   - status: Current status
 *   - timestamp: When request was made
 */
typedef struct {
    uint32_t txn_id;                /* Transaction ID */
    uint32_t resource_id;           /* Resource ID */
    LockType lock_type;             /* Lock type */
    LockStatus status;              /* Lock status */
    time_t timestamp;               /* Request timestamp */
} LockRequest;

/*
 * Lock - Represents a lock on a resource
 * 
 * Fields:
 *   - resource_id: Resource being locked
 *   - exclusive_holder: Transaction holding exclusive lock
 *   - shared_holders: Array of transactions holding shared locks
 *   - shared_count: Number of shared lock holders
 *   - waiting_queue: Queue of waiting lock requests
 *   - queue_size: Size of waiting queue
 */
typedef struct {
    uint32_t resource_id;           /* Resource ID */
    uint32_t exclusive_holder;      /* Exclusive lock holder */
    uint32_t shared_holders[MAX_LOCK_HOLDERS];  /* Shared holders */
    uint32_t shared_count;          /* Number of shared holders */
    LockRequest waiting_queue[MAX_LOCK_HOLDERS];  /* Waiting queue */
    uint32_t queue_size;            /* Queue size */
} Lock;

/*
 * LockManager - Manages all locks
 * 
 * Fields:
 *   - locks: Array of locks
 *   - lock_count: Number of locks
 *   - total_granted: Total locks granted
 *   - total_denied: Total locks denied
 */
typedef struct {
    Lock locks[MAX_LOCKS];          /* Locks */
    uint32_t lock_count;            /* Number of locks */
    uint64_t total_granted;         /* Total granted */
    uint64_t total_denied;          /* Total denied */
} LockManager;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * lock_manager_create - Create lock manager
 * 
 * Allocates and initializes a new lock manager.
 * 
 * Returns:
 *   Pointer to new LockManager, or NULL on error
 */
LockManager* lock_manager_create(void);

/*
 * lock_manager_destroy - Free lock manager
 * 
 * Frees the memory allocated for lock manager.
 * 
 * Parameters:
 *   mgr: Pointer to LockManager
 * 
 * Returns:
 *   void
 */
void lock_manager_destroy(LockManager *mgr);

/*
 * lock_manager_acquire_lock - Acquire a lock
 * 
 * Attempts to acquire a lock on a resource.
 * Returns immediately if lock is available.
 * Blocks if lock is not available.
 * 
 * Parameters:
 *   mgr: Pointer to LockManager
 *   txn_id: Transaction ID
 *   resource_id: Resource ID
 *   lock_type: Type of lock
 * 
 * Returns:
 *   LOCK_GRANTED on success, LOCK_WAITING/LOCK_DENIED on failure
 */
LockStatus lock_manager_acquire_lock(LockManager *mgr, uint32_t txn_id,
                                     uint32_t resource_id, LockType lock_type);

/*
 * lock_manager_release_lock - Release a lock
 * 
 * Releases a lock held by a transaction.
 * 
 * Parameters:
 *   mgr: Pointer to LockManager
 *   txn_id: Transaction ID
 *   resource_id: Resource ID
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int lock_manager_release_lock(LockManager *mgr, uint32_t txn_id,
                              uint32_t resource_id);

/*
 * lock_manager_release_all_locks - Release all locks for transaction
 * 
 * Releases all locks held by a transaction.
 * 
 * Parameters:
 *   mgr: Pointer to LockManager
 *   txn_id: Transaction ID
 * 
 * Returns:
 *   Number of locks released
 */
uint32_t lock_manager_release_all_locks(LockManager *mgr, uint32_t txn_id);

/*
 * lock_manager_has_lock - Check if transaction has lock
 * 
 * Determines if transaction holds a lock on resource.
 * 
 * Parameters:
 *   mgr: Pointer to LockManager
 *   txn_id: Transaction ID
 *   resource_id: Resource ID
 * 
 * Returns:
 *   1 if has lock, 0 if not
 */
int lock_manager_has_lock(LockManager *mgr, uint32_t txn_id,
                          uint32_t resource_id);

/*
 * lock_manager_get_lock_type - Get lock type held by transaction
 * 
 * Returns the type of lock held by transaction on resource.
 * 
 * Parameters:
 *   mgr: Pointer to LockManager
 *   txn_id: Transaction ID
 *   resource_id: Resource ID
 * 
 * Returns:
 *   Lock type, or -1 if no lock
 */
int lock_manager_get_lock_type(LockManager *mgr, uint32_t txn_id,
                               uint32_t resource_id);

/*
 * lock_manager_get_lock_count - Get number of locks
 * 
 * Returns the number of locks in the manager.
 * 
 * Parameters:
 *   mgr: Pointer to LockManager
 * 
 * Returns:
 *   Number of locks
 */
uint32_t lock_manager_get_lock_count(LockManager *mgr);

/*
 * lock_manager_get_statistics - Get lock statistics
 * 
 * Returns lock statistics.
 * 
 * Parameters:
 *   mgr: Pointer to LockManager
 *   granted: Pointer to store granted count
 *   denied: Pointer to store denied count
 * 
 * Returns:
 *   void
 */
void lock_manager_get_statistics(LockManager *mgr, uint64_t *granted,
                                 uint64_t *denied);

/*
 * lock_manager_print_locks - Print lock information
 * 
 * Prints all locks in human-readable format.
 * 
 * Parameters:
 *   mgr: Pointer to LockManager
 * 
 * Returns:
 *   void
 */
void lock_manager_print_locks(LockManager *mgr);

#endif // LOCK_H
