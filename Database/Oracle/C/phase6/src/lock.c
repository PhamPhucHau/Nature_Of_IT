/*
 * lock.c - Lock Manager Implementation
 * 
 * Implements lock management for concurrency control.
 * Supports shared (read) and exclusive (write) locks.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "lock.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * LOCK MANAGER FUNCTIONS
 * ============================================================================ */

LockManager* lock_manager_create(void) {
    LockManager *mgr = (LockManager *)malloc(sizeof(LockManager));
    if (!mgr) return NULL;
    
    memset(mgr, 0, sizeof(LockManager));
    mgr->lock_count = 0;
    mgr->total_granted = 0;
    mgr->total_denied = 0;
    
    return mgr;
}

void lock_manager_destroy(LockManager *mgr) {
    if (!mgr) return;
    free(mgr);
}

static Lock* lock_manager_get_or_create_lock(LockManager *mgr, uint32_t resource_id) {
    /* Find existing lock */
    for (uint32_t i = 0; i < mgr->lock_count; i++) {
        if (mgr->locks[i].resource_id == resource_id) {
            return &mgr->locks[i];
        }
    }
    
    /* Create new lock if space available */
    if (mgr->lock_count >= MAX_LOCKS) {
        return NULL;
    }
    
    Lock *lock = &mgr->locks[mgr->lock_count];
    lock->resource_id = resource_id;
    lock->exclusive_holder = INVALID_TXN_ID;
    lock->shared_count = 0;
    lock->queue_size = 0;
    mgr->lock_count++;
    
    return lock;
}

static int lock_manager_is_compatible(Lock *lock, LockType type) {
    /* Exclusive lock is not compatible with anything */
    if (lock->exclusive_holder != INVALID_TXN_ID) {
        return 0;
    }
    
    /* Shared lock is compatible with other shared locks */
    if (type == LOCK_SHARED) {
        return 1;
    }
    
    /* Exclusive lock is not compatible with shared locks */
    if (type == LOCK_EXCLUSIVE && lock->shared_count == 0) {
        return 1;
    }
    
    return 0;
}

LockStatus lock_manager_acquire_lock(LockManager *mgr, uint32_t txn_id,
                                     uint32_t resource_id, LockType lock_type) {
    if (!mgr) return LOCK_DENIED;
    
    Lock *lock = lock_manager_get_or_create_lock(mgr, resource_id);
    if (!lock) return LOCK_DENIED;
    
    /* Check if transaction already has lock */
    if (lock_manager_has_lock(mgr, txn_id, resource_id)) {
        return LOCK_GRANTED;
    }
    
    /* Check if lock is compatible */
    if (lock_manager_is_compatible(lock, lock_type)) {
        if (lock_type == LOCK_SHARED) {
            if (lock->shared_count < MAX_LOCK_HOLDERS) {
                lock->shared_holders[lock->shared_count++] = txn_id;
                mgr->total_granted++;
                return LOCK_GRANTED;
            }
        } else {
            lock->exclusive_holder = txn_id;
            mgr->total_granted++;
            return LOCK_GRANTED;
        }
    }
    
    /* Add to waiting queue */
    if (lock->queue_size < MAX_LOCK_HOLDERS) {
        LockRequest *req = &lock->waiting_queue[lock->queue_size];
        req->txn_id = txn_id;
        req->resource_id = resource_id;
        req->lock_type = lock_type;
        req->status = LOCK_WAITING;
        req->timestamp = time(NULL);
        lock->queue_size++;
        return LOCK_WAITING;
    }
    
    mgr->total_denied++;
    return LOCK_DENIED;
}

int lock_manager_release_lock(LockManager *mgr, uint32_t txn_id,
                              uint32_t resource_id) {
    if (!mgr) return -1;
    
    /* Find lock */
    Lock *lock = NULL;
    for (uint32_t i = 0; i < mgr->lock_count; i++) {
        if (mgr->locks[i].resource_id == resource_id) {
            lock = &mgr->locks[i];
            break;
        }
    }
    
    if (!lock) return -1;
    
    /* Release exclusive lock */
    if (lock->exclusive_holder == txn_id) {
        lock->exclusive_holder = INVALID_TXN_ID;
        return 0;
    }
    
    /* Release shared lock */
    for (uint32_t i = 0; i < lock->shared_count; i++) {
        if (lock->shared_holders[i] == txn_id) {
            /* Remove from shared holders */
            for (uint32_t j = i; j < lock->shared_count - 1; j++) {
                lock->shared_holders[j] = lock->shared_holders[j + 1];
            }
            lock->shared_count--;
            return 0;
        }
    }
    
    return -1;
}

uint32_t lock_manager_release_all_locks(LockManager *mgr, uint32_t txn_id) {
    if (!mgr) return 0;
    
    uint32_t released = 0;
    
    for (uint32_t i = 0; i < mgr->lock_count; i++) {
        Lock *lock = &mgr->locks[i];
        
        /* Release exclusive lock */
        if (lock->exclusive_holder == txn_id) {
            lock->exclusive_holder = INVALID_TXN_ID;
            released++;
        }
        
        /* Release shared locks */
        for (uint32_t j = 0; j < lock->shared_count; j++) {
            if (lock->shared_holders[j] == txn_id) {
                for (uint32_t k = j; k < lock->shared_count - 1; k++) {
                    lock->shared_holders[k] = lock->shared_holders[k + 1];
                }
                lock->shared_count--;
                released++;
                j--;
            }
        }
    }
    
    return released;
}

int lock_manager_has_lock(LockManager *mgr, uint32_t txn_id,
                          uint32_t resource_id) {
    if (!mgr) return 0;
    
    for (uint32_t i = 0; i < mgr->lock_count; i++) {
        Lock *lock = &mgr->locks[i];
        if (lock->resource_id != resource_id) continue;
        
        /* Check exclusive lock */
        if (lock->exclusive_holder == txn_id) {
            return 1;
        }
        
        /* Check shared locks */
        for (uint32_t j = 0; j < lock->shared_count; j++) {
            if (lock->shared_holders[j] == txn_id) {
                return 1;
            }
        }
    }
    
    return 0;
}

int lock_manager_get_lock_type(LockManager *mgr, uint32_t txn_id,
                               uint32_t resource_id) {
    if (!mgr) return -1;
    
    for (uint32_t i = 0; i < mgr->lock_count; i++) {
        Lock *lock = &mgr->locks[i];
        if (lock->resource_id != resource_id) continue;
        
        /* Check exclusive lock */
        if (lock->exclusive_holder == txn_id) {
            return LOCK_EXCLUSIVE;
        }
        
        /* Check shared locks */
        for (uint32_t j = 0; j < lock->shared_count; j++) {
            if (lock->shared_holders[j] == txn_id) {
                return LOCK_SHARED;
            }
        }
    }
    
    return -1;
}

uint32_t lock_manager_get_lock_count(LockManager *mgr) {
    if (!mgr) return 0;
    return mgr->lock_count;
}

void lock_manager_get_statistics(LockManager *mgr, uint64_t *granted,
                                 uint64_t *denied) {
    if (!mgr) return;
    if (granted) *granted = mgr->total_granted;
    if (denied) *denied = mgr->total_denied;
}

void lock_manager_print_locks(LockManager *mgr) {
    if (!mgr) return;
    
    printf("\n=== Lock Manager Status ===\n");
    printf("Total Locks: %u\n", mgr->lock_count);
    printf("Total Granted: %lu\n", mgr->total_granted);
    printf("Total Denied: %lu\n", mgr->total_denied);
    
    for (uint32_t i = 0; i < mgr->lock_count; i++) {
        Lock *lock = &mgr->locks[i];
        printf("\nResource %u:\n", lock->resource_id);
        
        if (lock->exclusive_holder != INVALID_TXN_ID) {
            printf("  Exclusive: TXN %u\n", lock->exclusive_holder);
        }
        
        if (lock->shared_count > 0) {
            printf("  Shared (%u): ", lock->shared_count);
            for (uint32_t j = 0; j < lock->shared_count; j++) {
                printf("TXN %u ", lock->shared_holders[j]);
            }
            printf("\n");
        }
        
        if (lock->queue_size > 0) {
            printf("  Waiting (%u):\n", lock->queue_size);
            for (uint32_t j = 0; j < lock->queue_size; j++) {
                LockRequest *req = &lock->waiting_queue[j];
                printf("    TXN %u (%s)\n", req->txn_id,
                       req->lock_type == LOCK_SHARED ? "SHARED" : "EXCLUSIVE");
            }
        }
    }
}
