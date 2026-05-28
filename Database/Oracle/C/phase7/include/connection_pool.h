/*
 * connection_pool.h - Connection Pool Header
 * 
 * Implements connection pooling for efficient resource management.
 * Reuses connections instead of creating new ones.
 * 
 * Key Concepts:
 * - Connection Pool: Manages pool of connections
 * - Idle Connection: Available for use
 * - Active Connection: Currently in use
 * - Connection Reuse: Reuse instead of create
 * - Pool Statistics: Track pool usage
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include "client.h"
#include <stdint.h>
#include <time.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Maximum pool size */
#define MAX_POOL_SIZE 100

/* Connection timeout (seconds) */
#define POOL_TIMEOUT 300

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * PoolConnectionStatus - Status of pooled connection
 */
typedef enum {
    POOL_IDLE,          /* Connection idle */
    POOL_ACTIVE,        /* Connection in use */
    POOL_CLOSED         /* Connection closed */
} PoolConnectionStatus;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * PooledConnection - Connection in pool
 * 
 * Fields:
 *   - client: Database client
 *   - status: Connection status
 *   - created_at: When created
 *   - last_used: Last use time
 *   - use_count: Number of times used
 */
typedef struct {
    DatabaseClient *client;         /* Database client */
    PoolConnectionStatus status;    /* Connection status */
    time_t created_at;              /* Created time */
    time_t last_used;               /* Last used time */
    uint64_t use_count;             /* Use count */
} PooledConnection;

/*
 * ConnectionPool - Pool of connections
 * 
 * Fields:
 *   - host: Server hostname
 *   - port: Server port
 *   - connections: Array of pooled connections
 *   - pool_size: Current pool size
 *   - max_size: Maximum pool size
 *   - idle_count: Number of idle connections
 *   - active_count: Number of active connections
 */
typedef struct {
    char host[256];                 /* Server host */
    uint16_t port;                  /* Server port */
    PooledConnection connections[MAX_POOL_SIZE];  /* Connections */
    uint32_t pool_size;             /* Current size */
    uint32_t max_size;              /* Maximum size */
    uint32_t idle_count;            /* Idle count */
    uint32_t active_count;          /* Active count */
} ConnectionPool;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * connection_pool_create - Create connection pool
 * 
 * Allocates and initializes a new connection pool.
 * 
 * Parameters:
 *   host: Server hostname
 *   port: Server port
 *   initial_size: Initial pool size
 *   max_size: Maximum pool size
 * 
 * Returns:
 *   Pointer to new ConnectionPool, or NULL on error
 */
ConnectionPool* connection_pool_create(const char *host, uint16_t port,
                                       uint32_t initial_size, uint32_t max_size);

/*
 * connection_pool_destroy - Free connection pool
 * 
 * Frees the memory allocated for connection pool.
 * 
 * Parameters:
 *   pool: Pointer to ConnectionPool
 * 
 * Returns:
 *   void
 */
void connection_pool_destroy(ConnectionPool *pool);

/*
 * connection_pool_get_connection - Get connection from pool
 * 
 * Gets an idle connection from the pool.
 * Creates new connection if needed.
 * 
 * Parameters:
 *   pool: Pointer to ConnectionPool
 * 
 * Returns:
 *   Pointer to DatabaseClient, or NULL if pool full
 */
DatabaseClient* connection_pool_get_connection(ConnectionPool *pool);

/*
 * connection_pool_return_connection - Return connection to pool
 * 
 * Returns a connection to the pool for reuse.
 * 
 * Parameters:
 *   pool: Pointer to ConnectionPool
 *   client: Pointer to DatabaseClient
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int connection_pool_return_connection(ConnectionPool *pool, DatabaseClient *client);

/*
 * connection_pool_close_connection - Close connection
 * 
 * Closes a connection and removes from pool.
 * 
 * Parameters:
 *   pool: Pointer to ConnectionPool
 *   client: Pointer to DatabaseClient
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int connection_pool_close_connection(ConnectionPool *pool, DatabaseClient *client);

/*
 * connection_pool_get_idle_count - Get idle connection count
 * 
 * Returns the number of idle connections.
 * 
 * Parameters:
 *   pool: Pointer to ConnectionPool
 * 
 * Returns:
 *   Number of idle connections
 */
uint32_t connection_pool_get_idle_count(ConnectionPool *pool);

/*
 * connection_pool_get_active_count - Get active connection count
 * 
 * Returns the number of active connections.
 * 
 * Parameters:
 *   pool: Pointer to ConnectionPool
 * 
 * Returns:
 *   Number of active connections
 */
uint32_t connection_pool_get_active_count(ConnectionPool *pool);

/*
 * connection_pool_get_statistics - Get pool statistics
 * 
 * Returns pool statistics.
 * 
 * Parameters:
 *   pool: Pointer to ConnectionPool
 *   idle: Pointer to store idle count
 *   active: Pointer to store active count
 *   total: Pointer to store total count
 * 
 * Returns:
 *   void
 */
void connection_pool_get_statistics(ConnectionPool *pool, uint32_t *idle,
                                    uint32_t *active, uint32_t *total);

/*
 * connection_pool_cleanup_idle - Clean up idle connections
 * 
 * Closes idle connections that have timed out.
 * 
 * Parameters:
 *   pool: Pointer to ConnectionPool
 *   timeout: Timeout in seconds
 * 
 * Returns:
 *   Number of connections closed
 */
uint32_t connection_pool_cleanup_idle(ConnectionPool *pool, uint32_t timeout);

/*
 * connection_pool_print_status - Print pool status
 * 
 * Prints pool status in human-readable format.
 * 
 * Parameters:
 *   pool: Pointer to ConnectionPool
 * 
 * Returns:
 *   void
 */
void connection_pool_print_status(ConnectionPool *pool);

#endif // CONNECTION_POOL_H
