/*
 * client.h - Database Client Header
 * 
 * Implements a client library for connecting to database server.
 * Provides API for sending queries and receiving results.
 * 
 * Key Concepts:
 * - Client: Connects to server
 * - Connection: Client-server channel
 * - Query: SQL command to execute
 * - Result: Query result from server
 * - Session: Client session state
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include <time.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Maximum query length */
#define MAX_QUERY_LENGTH 4096

/* Maximum result size */
#define MAX_RESULT_SIZE 8192

/* Connection timeout (seconds) */
#define CONNECTION_TIMEOUT 30

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * ClientStatus - Status of client connection
 */
typedef enum {
    CLIENT_DISCONNECTED,    /* Not connected */
    CLIENT_CONNECTING,      /* Connecting */
    CLIENT_CONNECTED,       /* Connected */
    CLIENT_ERROR            /* Error state */
} ClientStatus;

/*
 * QueryType - Type of query
 */
typedef enum {
    QUERY_SELECT,           /* SELECT query */
    QUERY_INSERT,           /* INSERT query */
    QUERY_UPDATE,           /* UPDATE query */
    QUERY_DELETE,           /* DELETE query */
    QUERY_CREATE,           /* CREATE query */
    QUERY_DROP              /* DROP query */
} QueryType;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * QueryResult - Result from query execution
 * 
 * Fields:
 *   - success: Whether query succeeded
 *   - rows_affected: Number of rows affected
 *   - result_data: Result data
 *   - error_message: Error message if any
 */
typedef struct {
    int success;                    /* Success flag */
    uint32_t rows_affected;         /* Rows affected */
    char result_data[MAX_RESULT_SIZE];  /* Result data */
    char error_message[256];        /* Error message */
} QueryResult;

/*
 * DatabaseClient - Client connection
 * 
 * Fields:
 *   - host: Server hostname
 *   - port: Server port
 *   - socket_fd: Socket file descriptor
 *   - status: Connection status
 *   - connected_at: When connected
 *   - queries_sent: Number of queries sent
 */
typedef struct {
    char host[256];                 /* Server host */
    uint16_t port;                  /* Server port */
    int socket_fd;                  /* Socket FD */
    ClientStatus status;            /* Connection status */
    time_t connected_at;            /* Connected time */
    uint64_t queries_sent;          /* Queries sent */
} DatabaseClient;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * client_create - Create database client
 * 
 * Allocates and initializes a new database client.
 * 
 * Parameters:
 *   host: Server hostname
 *   port: Server port
 * 
 * Returns:
 *   Pointer to new DatabaseClient, or NULL on error
 */
DatabaseClient* client_create(const char *host, uint16_t port);

/*
 * client_destroy - Free client
 * 
 * Frees the memory allocated for client.
 * 
 * Parameters:
 *   client: Pointer to DatabaseClient
 * 
 * Returns:
 *   void
 */
void client_destroy(DatabaseClient *client);

/*
 * client_connect - Connect to server
 * 
 * Connects to the database server.
 * 
 * Parameters:
 *   client: Pointer to DatabaseClient
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int client_connect(DatabaseClient *client);

/*
 * client_disconnect - Disconnect from server
 * 
 * Disconnects from the database server.
 * 
 * Parameters:
 *   client: Pointer to DatabaseClient
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int client_disconnect(DatabaseClient *client);

/*
 * client_is_connected - Check if connected
 * 
 * Determines if client is connected to server.
 * 
 * Parameters:
 *   client: Pointer to DatabaseClient
 * 
 * Returns:
 *   1 if connected, 0 if not
 */
int client_is_connected(DatabaseClient *client);

/*
 * client_execute_query - Execute query
 * 
 * Sends a query to the server and receives result.
 * 
 * Parameters:
 *   client: Pointer to DatabaseClient
 *   query: SQL query string
 *   result: Pointer to QueryResult
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int client_execute_query(DatabaseClient *client, const char *query,
                        QueryResult *result);

/*
 * client_ping - Ping server
 * 
 * Sends a ping to the server to check connectivity.
 * 
 * Parameters:
 *   client: Pointer to DatabaseClient
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int client_ping(DatabaseClient *client);

/*
 * client_get_status - Get client status
 * 
 * Returns the current client status.
 * 
 * Parameters:
 *   client: Pointer to DatabaseClient
 * 
 * Returns:
 *   Current ClientStatus
 */
ClientStatus client_get_status(DatabaseClient *client);

/*
 * client_get_query_count - Get number of queries sent
 * 
 * Returns the number of queries sent by this client.
 * 
 * Parameters:
 *   client: Pointer to DatabaseClient
 * 
 * Returns:
 *   Number of queries
 */
uint64_t client_get_query_count(DatabaseClient *client);

/*
 * client_print_info - Print client information
 * 
 * Prints client information in human-readable format.
 * 
 * Parameters:
 *   client: Pointer to DatabaseClient
 * 
 * Returns:
 *   void
 */
void client_print_info(DatabaseClient *client);

#endif // CLIENT_H
