/*
 * deadlock.c - Deadlock Detection Implementation
 * 
 * Implements deadlock detection using wait-for graph.
 * Detects circular dependencies between transactions.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "deadlock.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * DEADLOCK DETECTOR FUNCTIONS
 * ============================================================================ */

DeadlockDetector* deadlock_detector_create(VictimSelectionPolicy policy) {
    DeadlockDetector *detector = (DeadlockDetector *)malloc(sizeof(DeadlockDetector));
    if (!detector) return NULL;
    
    memset(detector, 0, sizeof(DeadlockDetector));
    detector->policy = policy;
    detector->deadlocks_detected = 0;
    detector->deadlocks_resolved = 0;
    
    return detector;
}

void deadlock_detector_destroy(DeadlockDetector *detector) {
    if (!detector) return;
    free(detector);
}

int deadlock_detector_add_edge(DeadlockDetector *detector, uint32_t from_txn,
                               uint32_t to_txn, uint32_t resource_id) {
    if (!detector) return -1;
    
    WaitForGraph *graph = &detector->graph;
    
    /* Check if edge already exists */
    for (uint32_t i = 0; i < graph->edge_count; i++) {
        if (graph->edges[i].from_txn == from_txn && graph->edges[i].to_txn == to_txn) {
            return 0;
        }
    }
    
    /* Add new edge */
    if (graph->edge_count >= MAX_EDGES) {
        return -1;
    }
    
    WaitForEdge *edge = &graph->edges[graph->edge_count];
    edge->from_txn = from_txn;
    edge->to_txn = to_txn;
    edge->resource_id = resource_id;
    graph->edge_count++;
    
    /* Add transactions to graph */
    int from_found = 0, to_found = 0;
    for (uint32_t i = 0; i < graph->txn_count; i++) {
        if (graph->transactions[i] == from_txn) from_found = 1;
        if (graph->transactions[i] == to_txn) to_found = 1;
    }
    
    if (!from_found && graph->txn_count < MAX_TRANSACTIONS) {
        graph->transactions[graph->txn_count++] = from_txn;
    }
    if (!to_found && graph->txn_count < MAX_TRANSACTIONS) {
        graph->transactions[graph->txn_count++] = to_txn;
    }
    
    return 0;
}

int deadlock_detector_remove_edge(DeadlockDetector *detector, uint32_t from_txn,
                                  uint32_t to_txn) {
    if (!detector) return -1;
    
    WaitForGraph *graph = &detector->graph;
    
    for (uint32_t i = 0; i < graph->edge_count; i++) {
        if (graph->edges[i].from_txn == from_txn && graph->edges[i].to_txn == to_txn) {
            /* Remove edge */
            for (uint32_t j = i; j < graph->edge_count - 1; j++) {
                graph->edges[j] = graph->edges[j + 1];
            }
            graph->edge_count--;
            return 0;
        }
    }
    
    return -1;
}

static int deadlock_detector_has_cycle_dfs(DeadlockDetector *detector,
                                           uint32_t txn, uint32_t *visited,
                                           uint32_t *rec_stack) {
    WaitForGraph *graph = &detector->graph;
    
    visited[txn] = 1;
    rec_stack[txn] = 1;
    
    /* Check all outgoing edges */
    for (uint32_t i = 0; i < graph->edge_count; i++) {
        if (graph->edges[i].from_txn == txn) {
            uint32_t next_txn = graph->edges[i].to_txn;
            
            if (!visited[next_txn]) {
                if (deadlock_detector_has_cycle_dfs(detector, next_txn, visited, rec_stack)) {
                    return 1;
                }
            } else if (rec_stack[next_txn]) {
                return 1;
            }
        }
    }
    
    rec_stack[txn] = 0;
    return 0;
}

int deadlock_detector_detect(DeadlockDetector *detector) {
    if (!detector) return -1;
    
    WaitForGraph *graph = &detector->graph;
    
    if (graph->edge_count == 0) {
        return 0;
    }
    
    /* Allocate visited and recursion stack arrays */
    uint32_t *visited = (uint32_t *)calloc(MAX_TRANSACTIONS, sizeof(uint32_t));
    uint32_t *rec_stack = (uint32_t *)calloc(MAX_TRANSACTIONS, sizeof(uint32_t));
    
    if (!visited || !rec_stack) {
        free(visited);
        free(rec_stack);
        return -1;
    }
    
    /* Check for cycles starting from each transaction */
    for (uint32_t i = 0; i < graph->txn_count; i++) {
        uint32_t txn = graph->transactions[i];
        if (!visited[txn]) {
            if (deadlock_detector_has_cycle_dfs(detector, txn, visited, rec_stack)) {
                detector->deadlocks_detected++;
                free(visited);
                free(rec_stack);
                return 1;
            }
        }
    }
    
    free(visited);
    free(rec_stack);
    return 0;
}

uint32_t deadlock_detector_find_victim(DeadlockDetector *detector) {
    if (!detector) return 0;
    
    WaitForGraph *graph = &detector->graph;
    
    if (graph->txn_count == 0) {
        return 0;
    }
    
    uint32_t victim = graph->transactions[0];
    
    switch (detector->policy) {
        case VICTIM_YOUNGEST:
            /* Select transaction with highest ID (youngest) */
            for (uint32_t i = 1; i < graph->txn_count; i++) {
                if (graph->transactions[i] > victim) {
                    victim = graph->transactions[i];
                }
            }
            break;
            
        case VICTIM_OLDEST:
            /* Select transaction with lowest ID (oldest) */
            for (uint32_t i = 1; i < graph->txn_count; i++) {
                if (graph->transactions[i] < victim) {
                    victim = graph->transactions[i];
                }
            }
            break;
            
        case VICTIM_LEAST_LOCKS:
            /* Select transaction with least locks (simplified) */
            /* In real implementation, would track lock counts */
            victim = graph->transactions[graph->txn_count / 2];
            break;
    }
    
    return victim;
}

uint32_t deadlock_detector_resolve(DeadlockDetector *detector) {
    if (!detector) return 0;
    
    /* Detect deadlock */
    if (!deadlock_detector_detect(detector)) {
        return 0;
    }
    
    /* Find victim */
    uint32_t victim = deadlock_detector_find_victim(detector);
    if (victim == 0) {
        return 0;
    }
    
    /* Remove all edges involving victim */
    WaitForGraph *graph = &detector->graph;
    for (uint32_t i = 0; i < graph->edge_count; ) {
        if (graph->edges[i].from_txn == victim || graph->edges[i].to_txn == victim) {
            for (uint32_t j = i; j < graph->edge_count - 1; j++) {
                graph->edges[j] = graph->edges[j + 1];
            }
            graph->edge_count--;
        } else {
            i++;
        }
    }
    
    /* Remove victim from transaction list */
    for (uint32_t i = 0; i < graph->txn_count; i++) {
        if (graph->transactions[i] == victim) {
            for (uint32_t j = i; j < graph->txn_count - 1; j++) {
                graph->transactions[j] = graph->transactions[j + 1];
            }
            graph->txn_count--;
            break;
        }
    }
    
    detector->deadlocks_resolved++;
    return victim;
}

void deadlock_detector_clear_graph(DeadlockDetector *detector) {
    if (!detector) return;
    
    WaitForGraph *graph = &detector->graph;
    graph->edge_count = 0;
    graph->txn_count = 0;
}

void deadlock_detector_get_statistics(DeadlockDetector *detector,
                                      uint64_t *detected, uint64_t *resolved) {
    if (!detector) return;
    if (detected) *detected = detector->deadlocks_detected;
    if (resolved) *resolved = detector->deadlocks_resolved;
}

void deadlock_detector_print_graph(DeadlockDetector *detector) {
    if (!detector) return;
    
    WaitForGraph *graph = &detector->graph;
    
    printf("\n=== Wait-For Graph ===\n");
    printf("Transactions: %u\n", graph->txn_count);
    printf("Edges: %u\n", graph->edge_count);
    printf("Deadlocks Detected: %lu\n", detector->deadlocks_detected);
    printf("Deadlocks Resolved: %lu\n", detector->deadlocks_resolved);
    
    if (graph->txn_count > 0) {
        printf("\nTransactions: ");
        for (uint32_t i = 0; i < graph->txn_count; i++) {
            printf("%u ", graph->transactions[i]);
        }
        printf("\n");
    }
    
    if (graph->edge_count > 0) {
        printf("\nEdges:\n");
        for (uint32_t i = 0; i < graph->edge_count; i++) {
            WaitForEdge *edge = &graph->edges[i];
            printf("  TXN %u -> TXN %u (Resource %u)\n",
                   edge->from_txn, edge->to_txn, edge->resource_id);
        }
    }
}
