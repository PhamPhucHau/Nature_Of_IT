/*
 * deadlock.h - Deadlock Detection Header
 * 
 * Implements deadlock detection using wait-for graph.
 * Detects circular dependencies between transactions.
 * 
 * Key Concepts:
 * - Wait-For Graph: Transaction dependencies
 * - Cycle Detection: Find circular dependencies
 * - Victim Selection: Choose transaction to abort
 * - Deadlock Prevention: Avoid deadlocks
 * - Deadlock Recovery: Recover from deadlocks
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef DEADLOCK_H
#define DEADLOCK_H

#include <stdint.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Maximum transactions */
#define MAX_TRANSACTIONS 1000

/* Maximum edges in wait-for graph */
#define MAX_EDGES 10000

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * VictimSelectionPolicy - Policy for selecting deadlock victim
 */
typedef enum {
    VICTIM_YOUNGEST,    /* Abort youngest transaction */
    VICTIM_OLDEST,      /* Abort oldest transaction */
    VICTIM_LEAST_LOCKS  /* Abort transaction with least locks */
} VictimSelectionPolicy;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * WaitForEdge - Edge in wait-for graph
 * 
 * Fields:
 *   - from_txn: Transaction waiting
 *   - to_txn: Transaction being waited for
 *   - resource_id: Resource causing wait
 */
typedef struct {
    uint32_t from_txn;              /* Waiting transaction */
    uint32_t to_txn;                /* Waited-for transaction */
    uint32_t resource_id;           /* Resource ID */
} WaitForEdge;

/*
 * WaitForGraph - Graph of transaction dependencies
 * 
 * Fields:
 *   - edges: Array of edges
 *   - edge_count: Number of edges
 *   - transactions: Array of transaction IDs
 *   - txn_count: Number of transactions
 */
typedef struct {
    WaitForEdge edges[MAX_EDGES];   /* Edges */
    uint32_t edge_count;            /* Number of edges */
    uint32_t transactions[MAX_TRANSACTIONS];  /* Transactions */
    uint32_t txn_count;             /* Number of transactions */
} WaitForGraph;

/*
 * DeadlockDetector - Detects deadlocks
 * 
 * Fields:
 *   - graph: Wait-for graph
 *   - policy: Victim selection policy
 *   - deadlocks_detected: Count of deadlocks detected
 *   - deadlocks_resolved: Count of deadlocks resolved
 */
typedef struct {
    WaitForGraph graph;             /* Wait-for graph */
    VictimSelectionPolicy policy;   /* Victim selection policy */
    uint64_t deadlocks_detected;    /* Deadlocks detected */
    uint64_t deadlocks_resolved;    /* Deadlocks resolved */
} DeadlockDetector;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * deadlock_detector_create - Create deadlock detector
 * 
 * Allocates and initializes a new deadlock detector.
 * 
 * Parameters:
 *   policy: Victim selection policy
 * 
 * Returns:
 *   Pointer to new DeadlockDetector, or NULL on error
 */
DeadlockDetector* deadlock_detector_create(VictimSelectionPolicy policy);

/*
 * deadlock_detector_destroy - Free deadlock detector
 * 
 * Frees the memory allocated for deadlock detector.
 * 
 * Parameters:
 *   detector: Pointer to DeadlockDetector
 * 
 * Returns:
 *   void
 */
void deadlock_detector_destroy(DeadlockDetector *detector);

/*
 * deadlock_detector_add_edge - Add edge to wait-for graph
 * 
 * Adds an edge indicating transaction dependency.
 * 
 * Parameters:
 *   detector: Pointer to DeadlockDetector
 *   from_txn: Waiting transaction
 *   to_txn: Waited-for transaction
 *   resource_id: Resource ID
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int deadlock_detector_add_edge(DeadlockDetector *detector, uint32_t from_txn,
                               uint32_t to_txn, uint32_t resource_id);

/*
 * deadlock_detector_remove_edge - Remove edge from wait-for graph
 * 
 * Removes an edge when transaction is no longer waiting.
 * 
 * Parameters:
 *   detector: Pointer to DeadlockDetector
 *   from_txn: Waiting transaction
 *   to_txn: Waited-for transaction
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int deadlock_detector_remove_edge(DeadlockDetector *detector, uint32_t from_txn,
                                  uint32_t to_txn);

/*
 * deadlock_detector_detect - Detect deadlocks
 * 
 * Detects cycles in wait-for graph.
 * 
 * Parameters:
 *   detector: Pointer to DeadlockDetector
 * 
 * Returns:
 *   1 if deadlock detected, 0 if not, -1 on error
 */
int deadlock_detector_detect(DeadlockDetector *detector);

/*
 * deadlock_detector_find_victim - Find victim transaction
 * 
 * Selects a transaction to abort to break deadlock.
 * 
 * Parameters:
 *   detector: Pointer to DeadlockDetector
 * 
 * Returns:
 *   Transaction ID of victim, or 0 if none found
 */
uint32_t deadlock_detector_find_victim(DeadlockDetector *detector);

/*
 * deadlock_detector_resolve - Resolve deadlock
 * 
 * Resolves a detected deadlock by aborting victim.
 * 
 * Parameters:
 *   detector: Pointer to DeadlockDetector
 * 
 * Returns:
 *   Transaction ID of aborted victim, or 0 on error
 */
uint32_t deadlock_detector_resolve(DeadlockDetector *detector);

/*
 * deadlock_detector_clear_graph - Clear wait-for graph
 * 
 * Clears all edges from the wait-for graph.
 * 
 * Parameters:
 *   detector: Pointer to DeadlockDetector
 * 
 * Returns:
 *   void
 */
void deadlock_detector_clear_graph(DeadlockDetector *detector);

/*
 * deadlock_detector_get_statistics - Get statistics
 * 
 * Returns deadlock detection statistics.
 * 
 * Parameters:
 *   detector: Pointer to DeadlockDetector
 *   detected: Pointer to store detected count
 *   resolved: Pointer to store resolved count
 * 
 * Returns:
 *   void
 */
void deadlock_detector_get_statistics(DeadlockDetector *detector,
                                      uint64_t *detected, uint64_t *resolved);

/*
 * deadlock_detector_print_graph - Print wait-for graph
 * 
 * Prints the wait-for graph in human-readable format.
 * 
 * Parameters:
 *   detector: Pointer to DeadlockDetector
 * 
 * Returns:
 *   void
 */
void deadlock_detector_print_graph(DeadlockDetector *detector);

#endif // DEADLOCK_H
