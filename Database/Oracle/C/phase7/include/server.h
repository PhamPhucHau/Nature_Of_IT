/*
 * server.h - TCP Server Header
 * 
 * Implements a TCP server for database connections.
 * Handles client connections and query execution.
 * 
 * Key Concepts:
 * - Server: Listens for client connections
 * - Connection: Client-server communication channel
 * - Request: Client query or command
 * - Response: Server result or status
 * - Thread Pool: Handle multiple clients
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include <time.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Default server port */
#define DEFAULT_SERVER_PORT 5432

/* Maximum connections */
#define MAX_CONNECTIONS 1000

/* Maximum request size */
#define MAX_REQUEST_SIZE 4096

/* Maximum response size */
#define MAX_RESPONSE_SIZE 8192

/* ============================================================================
 * ENUMERATIONS
 * ============================================================================ */

/*
 * ConnectionStatus - Status of a connection
 */
typedef enum {
    CONN_IDLE,          /* Connection idle */
    CONN_ACTIVE,        /* Connection active */
    CONN_BUSY,          /* Connection processing */
    CONN_CLOSED         /* Connection closed */
} ConnectionStatus;

/*
 * RequestType - Type of client request
 */
typedef enum {
    REQ_QUERY,          /* SQL query */
    REQ_INSERT,         /* Insert operation */
    REQ_UPDATE,         /* Update operation */
    REQ_DELETE,         /* Delete operation */
    REQ_PING,           /* Ping/heartbeat */
    REQ_DISCONNECT      /* Disconnect */
} RequestType;

/*
 * ResponseStatus - Status of server response
 */
typedef enum {
    RESP_OK,            /* Success */
    RESP_ERROR,         /* Error */
    RESP_TIMEOUT,       /* Timeout */
    RESP_INVALID        /* Invalid request */
} ResponseStatus;

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * ClientRequest - Request from client
 * 
 * Fields:
 *   - type: Request type
 *   - query: SQL query string
 *   - client_id: Client identifier
 *   - timestamp: When request was sent
 */
typedef struct {
    RequestType type;               /* Request type */
    char query[MAX_REQUEST_SIZE];   /* Query string */
    uint32_t client_id;             /* Client ID */
    time_t timestamp;               /* Timestamp */
} ClientRequest;

/*
 * ServerResponse - Response from server
 * 
 * Fields:
 *   - status: Response status
 *   - data: Response data
 *   - rows_affected: Number of rows affected
 *   - error_message: Error message if any
 */
typedef struct {
    ResponseStatus status;          /* Response status */
    char data[MAX_RESPONSE_SIZE];   /* Response data */
    uint32_t rows_affected;         /* Rows affected */
    char error_message[256];        /* Error message */
} ServerResponse;

/*
 * Connection - Represents a client connection
 * 
 * Fields:
 *   - conn_id: Connection ID
 *   - client_id: Client ID
 *   - socket_fd: Socket file descriptor
 *   - status: Connection status
 *   - created_at: When connection was created
 *   - last_activity: Last activity time
 *   - requests_processed: Number of requests
 */
typedef struct {
    uint32_t conn_id;               /* Connection ID */
    uint32_t client_id;             /* Client ID */
    int socket_fd;                  /* Socket FD */
    ConnectionStatus status;        /* Connection status */
    time_t created_at;              /* Created time */
    time_t last_activity;           /* Last activity */
    uint64_t requests_processed;    /* Requests processed */
} Connection;

/*
 * DatabaseServer - Main server structure
 * 
 * Fields:
 *   - port: Server port
 *   - socket_fd: Server socket
 *   - connections: Array of connections
 *   - conn_count: Number of connections
 *   - is_running: Server running flag
 *   - total_requests: Total requests processed
 */
typedef struct {
    uint16_t port;                  /* Server port */
    int socket_fd;                  /* Server socket */
    Connection connections[MAX_CONNECTIONS];  /* Connections */
    uint32_t conn_count;            /* Number of connections */
    int is_running;                 /* Running flag */
    uint64_t total_requests;        /* Total requests */
} DatabaseServer;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * server_create - Create database server
 * 
 * Allocates and initializes a new database server.
 * 
 * Parameters:
 *   port: Server port
 * 
 * Returns:
 *   Pointer to new DatabaseServer, or NULL on error
 */
DatabaseServer* server_create(uint16_t port);

/*
 * server_destroy - Free server
 * 
 * Frees the memory allocated for server.
 * 
 * Parameters:
 *   server: Pointer to DatabaseServer
 * 
 * Returns:
 *   void
 */
void server_destroy(DatabaseServer *server);

/*
 * server_start - Start server
 * 
 * Starts the server and begins listening for connections.
 * 
 * Parameters:
 *   server: Pointer to DatabaseServer
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int server_start(DatabaseServer *server);

/*
 * server_stop - Stop server
 * 
 * Stops the server and closes all connections.
 * 
 * Parameters:
 *   server: Pointer to DatabaseServer
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int server_stop(DatabaseServer *server);

/*
 * server_accept_connection - Accept client connection
 * 
 * Accepts a new client connection.
 * 
 * Parameters:
 *   server: Pointer to DatabaseServer
 * 
 * Returns:
 *   Connection ID on success, 0 on error
 */
uint32_t server_accept_connection(DatabaseServer *server);

/*
 * server_close_connection - Close client connection
 * 
 * Closes a client connection.
 * 
 * Parameters:
 *   server: Pointer to DatabaseServer
 *   conn_id: Connection ID
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int server_close_connection(DatabaseServer *server, uint32_t conn_id);

/*
 * server_process_request - Process client request
 * 
 * Processes a request from a client.
 * 
 * Parameters:
 *   server: Pointer to DatabaseServer
 *   conn_id: Connection ID
 *   request: Pointer to ClientRequest
 *   response: Pointer to ServerResponse
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int server_process_request(DatabaseServer *server, uint32_t conn_id,
                          ClientRequest *request, ServerResponse *response);

/*
 * server_get_connection_count - Get number of connections
 * 
 * Returns the number of active connections.
 * 
 * Parameters:
 *   server: Pointer to DatabaseServer
 * 
 * Returns:
 *   Number of connections
 */
uint32_t server_get_connection_count(DatabaseServer *server);

/*
 * server_get_statistics - Get server statistics
 * 
 * Returns server statistics.
 * 
 * Parameters:
 *   server: Pointer to DatabaseServer
 *   total_requests: Pointer to store total requests
 *   active_connections: Pointer to store active connections
 * 
 * Returns:
 *   void
 */
void server_get_statistics(DatabaseServer *server, uint64_t *total_requests,
                          uint32_t *active_connections);

/*
 * server_print_status - Print server status
 * 
 * Prints server status in human-readable format.
 * 
 * Parameters:
 *   server: Pointer to DatabaseServer
 * 
 * Returns:
 *   void
 */
void server_print_status(DatabaseServer *server);

#endif // SERVER_H
