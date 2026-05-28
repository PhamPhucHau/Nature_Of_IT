/*
 * isolation.h - Transaction Isolation Levels Header
 * 
 * Implements different transaction isolation levels.
 * Controls visibility of uncommitted data.
 * 
 * Key Concepts:
 * - Isolation Level: Degree of isolation
 * - Dirty Read: Read uncommitted data
 * - Non-Repeatable Read: Data changes between reads
 * - Phantom Read: New rows appear between reads
 * - Serializable: Highest isolation level
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef ISOLATION_H
#define ISOLATION_H

#include <stdint.h>

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * IsolationLevel - Transaction isolation level
 */
typedef enum {
    ISOLATION_READ_UNCOMMITTED,     /* Lowest isolation */
    ISOLATION_READ_COMMITTED,       /* Read committed */
    ISOLATION_REPEATABLE_READ,      /* Repeatable read */
    ISOLATION_SERIALIZABLE          /* Highest isolation */
} IsolationLevel;

/*
 * AnomalyType - Type of read anomaly
 */
typedef enum {
    ANOMALY_DIRTY_READ,             /* Dirty read */
    ANOMALY_NON_REPEATABLE_READ,    /* Non-repeatable read */
    ANOMALY_PHANTOM_READ            /* Phantom read */
} AnomalyType;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * IsolationManager - Manages isolation levels
 * 
 * Fields:
 *   - level: Current isolation level
 *   - dirty_reads_allowed: Whether dirty reads allowed
 *   - non_repeatable_reads_allowed: Whether non-repeatable reads allowed
 *   - phantom_reads_allowed: Whether phantom reads allowed
 *   - anomalies_detected: Count of anomalies detected
 */
typedef struct {
    IsolationLevel level;           /* Isolation level */
    int dirty_reads_allowed;        /* Dirty reads allowed */
    int non_repeatable_reads_allowed;  /* Non-repeatable reads allowed */
    int phantom_reads_allowed;      /* Phantom reads allowed */
    uint64_t anomalies_detected;    /* Anomalies detected */
} IsolationManager;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * isolation_manager_create - Create isolation manager
 * 
 * Allocates and initializes a new isolation manager.
 * 
 * Parameters:
 *   level: Initial isolation level
 * 
 * Returns:
 *   Pointer to new IsolationManager, or NULL on error
 */
IsolationManager* isolation_manager_create(IsolationLevel level);

/*
 * isolation_manager_destroy - Free isolation manager
 * 
 * Frees the memory allocated for isolation manager.
 * 
 * Parameters:
 *   mgr: Pointer to IsolationManager
 * 
 * Returns:
 *   void
 */
void isolation_manager_destroy(IsolationManager *mgr);

/*
 * isolation_manager_set_level - Set isolation level
 * 
 * Sets the transaction isolation level.
 * 
 * Parameters:
 *   mgr: Pointer to IsolationManager
 *   level: New isolation level
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int isolation_manager_set_level(IsolationManager *mgr, IsolationLevel level);

/*
 * isolation_manager_get_level - Get isolation level
 * 
 * Returns the current isolation level.
 * 
 * Parameters:
 *   mgr: Pointer to IsolationManager
 * 
 * Returns:
 *   Current isolation level
 */
IsolationLevel isolation_manager_get_level(IsolationManager *mgr);

/*
 * isolation_manager_allows_dirty_read - Check if dirty reads allowed
 * 
 * Determines if dirty reads are allowed at current level.
 * 
 * Parameters:
 *   mgr: Pointer to IsolationManager
 * 
 * Returns:
 *   1 if allowed, 0 if not
 */
int isolation_manager_allows_dirty_read(IsolationManager *mgr);

/*
 * isolation_manager_allows_non_repeatable_read - Check if non-repeatable reads allowed
 * 
 * Determines if non-repeatable reads are allowed at current level.
 * 
 * Parameters:
 *   mgr: Pointer to IsolationManager
 * 
 * Returns:
 *   1 if allowed, 0 if not
 */
int isolation_manager_allows_non_repeatable_read(IsolationManager *mgr);

/*
 * isolation_manager_allows_phantom_read - Check if phantom reads allowed
 * 
 * Determines if phantom reads are allowed at current level.
 * 
 * Parameters:
 *   mgr: Pointer to IsolationManager
 * 
 * Returns:
 *   1 if allowed, 0 if not
 */
int isolation_manager_allows_phantom_read(IsolationManager *mgr);

/*
 * isolation_manager_detect_anomaly - Detect read anomaly
 * 
 * Detects if a read anomaly has occurred.
 * 
 * Parameters:
 *   mgr: Pointer to IsolationManager
 *   anomaly: Type of anomaly
 * 
 * Returns:
 *   1 if anomaly detected, 0 if not
 */
int isolation_manager_detect_anomaly(IsolationManager *mgr, AnomalyType anomaly);

/*
 * isolation_level_to_string - Convert level to string
 * 
 * Returns string representation of isolation level.
 * 
 * Parameters:
 *   level: IsolationLevel enum value
 * 
 * Returns:
 *   String representation
 */
const char* isolation_level_to_string(IsolationLevel level);

/*
 * isolation_manager_print_info - Print isolation information
 * 
 * Prints isolation manager information.
 * 
 * Parameters:
 *   mgr: Pointer to IsolationManager
 * 
 * Returns:
 *   void
 */
void isolation_manager_print_info(IsolationManager *mgr);

#endif // ISOLATION_H
