/*
 * isolation.c - Transaction Isolation Levels Implementation
 * 
 * Implements different transaction isolation levels.
 * Controls visibility of uncommitted data.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "isolation.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * ISOLATION MANAGER FUNCTIONS
 * ============================================================================ */

IsolationManager* isolation_manager_create(IsolationLevel level) {
    IsolationManager *mgr = (IsolationManager *)malloc(sizeof(IsolationManager));
    if (!mgr) return NULL;
    
    memset(mgr, 0, sizeof(IsolationManager));
    mgr->level = level;
    mgr->anomalies_detected = 0;
    
    /* Set anomaly permissions based on isolation level */
    switch (level) {
        case ISOLATION_READ_UNCOMMITTED:
            mgr->dirty_reads_allowed = 1;
            mgr->non_repeatable_reads_allowed = 1;
            mgr->phantom_reads_allowed = 1;
            break;
            
        case ISOLATION_READ_COMMITTED:
            mgr->dirty_reads_allowed = 0;
            mgr->non_repeatable_reads_allowed = 1;
            mgr->phantom_reads_allowed = 1;
            break;
            
        case ISOLATION_REPEATABLE_READ:
            mgr->dirty_reads_allowed = 0;
            mgr->non_repeatable_reads_allowed = 0;
            mgr->phantom_reads_allowed = 1;
            break;
            
        case ISOLATION_SERIALIZABLE:
            mgr->dirty_reads_allowed = 0;
            mgr->non_repeatable_reads_allowed = 0;
            mgr->phantom_reads_allowed = 0;
            break;
    }
    
    return mgr;
}

void isolation_manager_destroy(IsolationManager *mgr) {
    if (!mgr) return;
    free(mgr);
}

int isolation_manager_set_level(IsolationManager *mgr, IsolationLevel level) {
    if (!mgr) return -1;
    
    mgr->level = level;
    
    /* Update anomaly permissions */
    switch (level) {
        case ISOLATION_READ_UNCOMMITTED:
            mgr->dirty_reads_allowed = 1;
            mgr->non_repeatable_reads_allowed = 1;
            mgr->phantom_reads_allowed = 1;
            break;
            
        case ISOLATION_READ_COMMITTED:
            mgr->dirty_reads_allowed = 0;
            mgr->non_repeatable_reads_allowed = 1;
            mgr->phantom_reads_allowed = 1;
            break;
            
        case ISOLATION_REPEATABLE_READ:
            mgr->dirty_reads_allowed = 0;
            mgr->non_repeatable_reads_allowed = 0;
            mgr->phantom_reads_allowed = 1;
            break;
            
        case ISOLATION_SERIALIZABLE:
            mgr->dirty_reads_allowed = 0;
            mgr->non_repeatable_reads_allowed = 0;
            mgr->phantom_reads_allowed = 0;
            break;
    }
    
    return 0;
}

IsolationLevel isolation_manager_get_level(IsolationManager *mgr) {
    if (!mgr) return ISOLATION_READ_UNCOMMITTED;
    return mgr->level;
}

int isolation_manager_allows_dirty_read(IsolationManager *mgr) {
    if (!mgr) return 0;
    return mgr->dirty_reads_allowed;
}

int isolation_manager_allows_non_repeatable_read(IsolationManager *mgr) {
    if (!mgr) return 0;
    return mgr->non_repeatable_reads_allowed;
}

int isolation_manager_allows_phantom_read(IsolationManager *mgr) {
    if (!mgr) return 0;
    return mgr->phantom_reads_allowed;
}

int isolation_manager_detect_anomaly(IsolationManager *mgr, AnomalyType anomaly) {
    if (!mgr) return 0;
    
    int allowed = 0;
    
    switch (anomaly) {
        case ANOMALY_DIRTY_READ:
            allowed = mgr->dirty_reads_allowed;
            break;
            
        case ANOMALY_NON_REPEATABLE_READ:
            allowed = mgr->non_repeatable_reads_allowed;
            break;
            
        case ANOMALY_PHANTOM_READ:
            allowed = mgr->phantom_reads_allowed;
            break;
    }
    
    if (!allowed) {
        mgr->anomalies_detected++;
        return 1;
    }
    
    return 0;
}

const char* isolation_level_to_string(IsolationLevel level) {
    switch (level) {
        case ISOLATION_READ_UNCOMMITTED:
            return "READ_UNCOMMITTED";
        case ISOLATION_READ_COMMITTED:
            return "READ_COMMITTED";
        case ISOLATION_REPEATABLE_READ:
            return "REPEATABLE_READ";
        case ISOLATION_SERIALIZABLE:
            return "SERIALIZABLE";
        default:
            return "UNKNOWN";
    }
}

void isolation_manager_print_info(IsolationManager *mgr) {
    if (!mgr) return;
    
    printf("\n=== Isolation Manager ===\n");
    printf("Level: %s\n", isolation_level_to_string(mgr->level));
    printf("Dirty Reads Allowed: %s\n", mgr->dirty_reads_allowed ? "Yes" : "No");
    printf("Non-Repeatable Reads Allowed: %s\n", mgr->non_repeatable_reads_allowed ? "Yes" : "No");
    printf("Phantom Reads Allowed: %s\n", mgr->phantom_reads_allowed ? "Yes" : "No");
    printf("Anomalies Detected: %lu\n", mgr->anomalies_detected);
    
    printf("\nAnomaly Prevention:\n");
    printf("  Dirty Read: %s\n", mgr->dirty_reads_allowed ? "ALLOWED" : "PREVENTED");
    printf("  Non-Repeatable Read: %s\n", mgr->non_repeatable_reads_allowed ? "ALLOWED" : "PREVENTED");
    printf("  Phantom Read: %s\n", mgr->phantom_reads_allowed ? "ALLOWED" : "PREVENTED");
}
