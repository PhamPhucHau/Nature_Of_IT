/*
 * connection_pool.c - Connection Pool Implementation
 * 
 * Implements connection pooling for efficient resource management.
 * Reuses connections instead of creating new ones.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "connection_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * CONNECTION POOL FUNCTIONS
 * ============================================================================ */

ConnectionPool* connection_pool_create(const char *host, uint16_t port,
                                       uint32_t initial_size, uint32_t max_size) {
    ConnectionPool *pool = (ConnectionPool *)malloc(sizeof(ConnectionPool));
    if (!pool) return NULL;
    
    memset(pool, 0, sizeof(ConnectionPool));
    strncpy(pool->host, host, sizeof(pool->host) - 1);
    pool->port = port;
    pool->max_size = max_size > MAX_POOL_SIZE ? MAX_POOL_SIZE : max_size;
    pool->pool_size = 0;
    pool->idle_count = 0;
    pool->active_count = 0;
    
    /* Create initial connections */
    for (uint32_t i = 0; i < initial_size && i < pool->max_size; i++) {
        DatabaseClient *client = client_create(host, port);
        if (!client) continue;
        
        if (client_connect(client) == 0) {
            PooledConnection *pooled = &pool->connections[pool->pool_size];
            pooled->client = client;
            pooled->status = POOL_IDLE;
            pooled->created_at = time(NULL);
            pooled->last_used = time(NULL);
            pooled->use_count = 0;
            
            pool->pool_size++;
            pool->idle_count++;
        } else {
            client_destroy(client);
        }
    }
    
    return pool;
}

void connection_pool_destroy(ConnectionPool *pool) {
    if (!pool) return;
    
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->connections[i].client) {
            client_destroy(pool->connections[i].client);
        }
    }
    
    free(pool);
}

DatabaseClient* connection_pool_get_connection(ConnectionPool *pool) {
    if (!pool) return NULL;
    
    /* Find idle connection */
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->connections[i].status == POOL_IDLE) {
            PooledConnection *pooled = &pool->connections[i];
            pooled->status = POOL_ACTIVE;
            pooled->last_used = time(NULL);
            pooled->use_count++;
            
            pool->idle_count--;
            pool->active_count++;
            
            return pooled->client;
        }
    }
    
    /* Create new connection if space available */
    if (pool->pool_size < pool->max_size) {
        DatabaseClient *client = client_create(pool->host, pool->port);
        if (!client) return NULL;
        
        if (client_connect(client) == 0) {
            PooledConnection *pooled = &pool->connections[pool->pool_size];
            pooled->client = client;
            pooled->status = POOL_ACTIVE;
            pooled->created_at = time(NULL);
            pooled->last_used = time(NULL);
            pooled->use_count = 1;
            
            pool->pool_size++;
            pool->active_count++;
            
            return client;
        } else {
            client_destroy(client);
        }
    }
    
    return NULL;
}

int connection_pool_return_connection(ConnectionPool *pool, DatabaseClient *client) {
    if (!pool || !client) return -1;
    
    /* Find connection in pool */
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->connections[i].client == client) {
            if (pool->connections[i].status == POOL_ACTIVE) {
                pool->connections[i].status = POOL_IDLE;
                pool->connections[i].last_used = time(NULL);
                
                pool->active_count--;
                pool->idle_count++;
                
                return 0;
            }
        }
    }
    
    return -1;
}

int connection_pool_close_connection(ConnectionPool *pool, DatabaseClient *client) {
    if (!pool || !client) return -1;
    
    /* Find connection in pool */
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        if (pool->connections[i].client == client) {
            PooledConnection *pooled = &pool->connections[i];
            
            if (pooled->status == POOL_ACTIVE) {
                pool->active_count--;
            } else if (pooled->status == POOL_IDLE) {
                pool->idle_count--;
            }
            
            client_destroy(pooled->client);
            pooled->client = NULL;
            pooled->status = POOL_CLOSED;
            
            return 0;
        }
    }
    
    return -1;
}

uint32_t connection_pool_get_idle_count(ConnectionPool *pool) {
    if (!pool) return 0;
    return pool->idle_count;
}

uint32_t connection_pool_get_active_count(ConnectionPool *pool) {
    if (!pool) return 0;
    return pool->active_count;
}

void connection_pool_get_statistics(ConnectionPool *pool, uint32_t *idle,
                                    uint32_t *active, uint32_t *total) {
    if (!pool) return;
    
    if (idle) *idle = pool->idle_count;
    if (active) *active = pool->active_count;
    if (total) *total = pool->pool_size;
}

uint32_t connection_pool_cleanup_idle(ConnectionPool *pool, uint32_t timeout) {
    if (!pool) return 0;
    
    uint32_t closed = 0;
    time_t now = time(NULL);
    
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        PooledConnection *pooled = &pool->connections[i];
        
        if (pooled->status == POOL_IDLE) {
            if ((now - pooled->last_used) > timeout) {
                connection_pool_close_connection(pool, pooled->client);
                closed++;
            }
        }
    }
    
    return closed;
}

void connection_pool_print_status(ConnectionPool *pool) {
    if (!pool) return;
    
    printf("\n=== Connection Pool Status ===\n");
    printf("Host: %s\n", pool->host);
    printf("Port: %u\n", pool->port);
    printf("Pool Size: %u / %u\n", pool->pool_size, pool->max_size);
    printf("Idle Connections: %u\n", pool->idle_count);
    printf("Active Connections: %u\n", pool->active_count);
    
    printf("\nConnections:\n");
    for (uint32_t i = 0; i < pool->pool_size; i++) {
        PooledConnection *pooled = &pool->connections[i];
        if (pooled->client) {
            printf("  Connection %u: Status %s, Used %lu times\n",
                   i + 1,
                   pooled->status == POOL_IDLE ? "IDLE" :
                   pooled->status == POOL_ACTIVE ? "ACTIVE" : "CLOSED",
                   pooled->use_count);
        }
    }
}
