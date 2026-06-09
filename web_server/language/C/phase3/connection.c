/**
 * connection.c - Connection State Management Implementation
 * 
 * This file implements the connection state machine for handling
 * multiple client connections efficiently using non-blocking I/O.
 * 
 * Key Functions:
 * - connection_init(): Initialize a new connection
 * - connection_close(): Close and cleanup a connection
 * - connection_reset(): Reset connection for reuse
 */

#include "connection.h"
#include <stdio.h>      /* For printf */
#include <string.h>     /* For memset */
#include <unistd.h>     /* For close */
#include <errno.h>      /* For errno */

/**
 * connection_init() - Initialize a new connection
 * 
 * This function sets up a connection structure for a new client connection.
 * It configures the socket, initializes buffers, and sets the initial state.
 * 
 * @param conn Pointer to connection structure to initialize
 * @param fd File descriptor for the client socket
 * 
 * Debug Output:
 * - Prints connection details when initialized
 * - Shows buffer sizes and initial state
 */
void connection_init(connection_t *conn, int fd) {
    /* Validate parameters */
    if (conn == NULL) {
        fprintf(stderr, "ERROR: connection_init() - NULL connection pointer\n");
        return;
    }
    
    if (fd < 0) {
        fprintf(stderr, "ERROR: connection_init() - Invalid file descriptor: %d\n", fd);
        return;
    }
    
    /* Initialize connection fields */
    conn->fd = fd;
    conn->state = CONN_READING;           /* Start in reading state */
    conn->read_size = 0;                  /* No data read yet */
    conn->write_size = 0;                 /* No data to write */
    conn->write_sent = 0;                 /* Nothing sent yet */
    conn->last_active = time(NULL);       /* Set active timestamp */
    conn->request_complete = 0;           /* Request not complete */
    conn->response_ready = 0;             /* Response not ready */
    
    /* Clear buffers */
    memset(conn->read_buffer, 0, BUFFER_SIZE);
    memset(conn->write_buffer, 0, BUFFER_SIZE);
    
    /* Debug output */
    printf("[DEBUG] Connection initialized:\n");
    printf("  - fd: %d\n", conn->fd);
    printf("  - state: CONN_READING\n");
    printf("  - read_buffer: %d bytes\n", BUFFER_SIZE);
    printf("  - write_buffer: %d bytes\n", BUFFER_SIZE);
    printf("  - last_active: %ld\n", (long)conn->last_active);
    printf("\n");
}

/**
 * connection_close() - Close and cleanup a connection
 * 
 * This function properly closes a connection by:
 * 1. Closing the socket file descriptor
 * 2. Resetting all connection fields
 * 3. Freeing any allocated resources
 * 
 * @param conn Pointer to connection to close
 * 
 * Debug Output:
 * - Prints when connection is being closed
 * - Shows file descriptor being closed
 * - Reports any errors during close
 */
void connection_close(connection_t *conn) {
    /* Validate parameter */
    if (conn == NULL) {
        fprintf(stderr, "ERROR: connection_close() - NULL connection pointer\n");
        return;
    }
    
    /* Debug output */
    printf("[DEBUG] Closing connection:\n");
    printf("  - fd: %d\n", conn->fd);
    printf("  - state: %d\n", conn->state);
    printf("  - read_size: %d\n", conn->read_size);
    printf("  - write_size: %d\n", conn->write_size);
    printf("  - write_sent: %d\n", conn->write_sent);
    
    /* Close socket if valid */
    if (conn->fd >= 0) {
        int result = close(conn->fd);
        if (result < 0) {
            fprintf(stderr, "WARNING: close() failed with errno %d: %s\n", 
                    errno, strerror(errno));
        } else {
            printf("  - Socket closed successfully\n");
        }
    } else {
        printf("  - Socket already closed (fd = %d)\n", conn->fd);
    }
    
    /* Reset connection fields */
    conn->fd = -1;
    conn->state = CONN_CLOSING;
    conn->read_size = 0;
    conn->write_size = 0;
    conn->write_sent = 0;
    conn->request_complete = 0;
    conn->response_ready = 0;
    
    printf("[DEBUG] Connection closed successfully\n\n");
}

/**
 * connection_reset() - Reset connection for reuse
 * 
 * This function resets a connection to its initial state without
 * closing the socket. This allows the connection to be reused for
 * a new client request.
 * 
 * @param conn Pointer to connection to reset
 * 
 * Debug Output:
 * - Prints when connection is being reset
 * - Shows buffer clearing
 */
void connection_reset(connection_t *conn) {
    /* Validate parameter */
    if (conn == NULL) {
        fprintf(stderr, "ERROR: connection_reset() - NULL connection pointer\n");
        return;
    }
    
    /* Debug output */
    printf("[DEBUG] Resetting connection:\n");
    printf("  - fd: %d\n", conn->fd);
    printf("  - previous_state: %d\n", conn->state);
    
    /* Reset connection fields */
    conn->state = CONN_READING;
    conn->read_size = 0;
    conn->write_size = 0;
    conn->write_sent = 0;
    conn->request_complete = 0;
    conn->response_ready = 0;
    conn->last_active = time(NULL);
    
    /* Clear buffers */
    memset(conn->read_buffer, 0, BUFFER_SIZE);
    memset(conn->write_buffer, 0, BUFFER_SIZE);
    
    printf("  - state: CONN_READING (reset)\n");
    printf("  - buffers: cleared\n");
    printf("[DEBUG] Connection reset successfully\n\n");
}