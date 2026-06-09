/**
 * connection.h - Connection State Management Header
 * 
 * This header defines the connection structure and state machine for
 * handling multiple client connections using non-blocking I/O.
 * 
 * Connection States:
 * - CONN_READING: Waiting to receive data from client
 * - CONN_WRITING: Sending response data to client
 * - CONN_CLOSING: Connection is being closed
 * 
 * Key Concepts:
 * - Non-blocking sockets: Sockets that don't block execution when no data available
 * - State machine: Each connection transitions between states based on activity
 * - Buffer management: Separate read/write buffers for efficient data handling
 * - Timeout tracking: last_active timestamp for connection cleanup
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include <time.h>

/* Buffer size for read/write operations - 4KB is a good default */
#define BUFFER_SIZE 4096

/* Maximum number of concurrent connections */
#define MAX_CONNECTIONS 10

/* Connection states for the state machine */
typedef enum {
    CONN_READING,   /* Waiting to receive data from client */
    CONN_WRITING,   /* Sending response data to client */
    CONN_CLOSING    /* Connection is being closed */
} connection_state_t;

/**
 * connection_t - Represents a single client connection
 * 
 * This structure tracks all state for a single client connection:
 * - fd: Socket file descriptor for this connection
 * - state: Current state in the connection state machine
 * - read_buffer: Buffer for incoming data from client
 * - read_size: Number of bytes currently in read buffer
 * - write_buffer: Buffer for outgoing data to client
 * - write_size: Total bytes to send
 * - write_sent: Number of bytes already sent
 * - last_active: Timestamp of last activity (for timeout management)
 */
typedef struct {
    int fd;                         /* Socket file descriptor */
    
    connection_state_t state;       /* Current connection state */
    
    char read_buffer[BUFFER_SIZE];  /* Buffer for incoming data */
    int read_size;                  /* Bytes currently in read buffer */
    
    char write_buffer[BUFFER_SIZE]; /* Buffer for outgoing data */
    int write_size;                 /* Total bytes to send */
    int write_sent;                 /* Bytes already sent */
    
    time_t last_active;             /* Last activity timestamp */
    
    /* Additional fields for HTTP parsing */
    int request_complete;           /* Has complete request been received? */
    int response_ready;             /* Is response ready to send? */
} connection_t;

/* Function declarations */
void connection_init(connection_t *conn, int fd);
void connection_close(connection_t *conn);
void connection_reset(connection_t *conn);

#endif /* CONNECTION_H */