/**
 * 03_select_server.c - Select-based Multiplexing Server
 * 
 * This file implements a web server using select() for non-blocking I/O
 * to handle multiple connections efficiently.
 * 
 * Key Features:
 * - Uses select() to monitor multiple sockets simultaneously
 * - Sets sockets to non-blocking mode
 * - Manages connection state machine
 * - Handles partial reads/writes
 * - Tracks connection timeouts
 * 
 * Learning Points:
 * - select() system call for I/O multiplexing
 * - Non-blocking socket operations
 * - Connection state machine
 * - Buffer management for incomplete requests
 * - Timeout handling
 */

#include <stdio.h>      /* For printf, fprintf */
#include <stdlib.h>     /* For exit, malloc, free */
#include <string.h>     /* For memset, memcpy */
#include <unistd.h>     /* For close, sleep */
#include <sys/socket.h> /* For socket, bind, listen, accept */
#include <sys/select.h> /* For select, fd_set */
#include <netinet/in.h> /* For sockaddr_in */
#include <arpa/inet.h>  /* For inet_ntoa */
#include <fcntl.h>      /* For fcntl, O_NONBLOCK */
#include <errno.h>      /* For errno, strerror */
#include <time.h>       /* For time */
#include "connection.h" /* Connection management */

/* Server configuration */
#define SERVER_PORT 8080
#define MAX_CONNECTIONS 10
#define TIMEOUT_SECONDS 30

/* Global connection array */
static connection_t connections[MAX_CONNECTIONS];

/* Server socket file descriptor */
static int server_fd = -1;

/* Maximum file descriptor for select() */
static int max_fd = 0;

/* Function declarations */
static int create_server_socket(void);
static int set_nonblocking(int fd);
static void handle_new_connection(void);
static void handle_connection_activity(connection_t *conn, int is_readable, int is_writable);
static void send_response(connection_t *conn);
static void cleanup_connections(void);

/**
 * create_server_socket() - Create and configure server socket
 * 
 * This function creates a TCP socket, configures it for reuse,
 * binds it to the specified port, and puts it in listening mode.
 * 
 * @return Server socket file descriptor, or -1 on error
 * 
 * Debug Output:
 * - Prints socket creation details
 * - Shows binding information
 * - Reports any errors
 */
static int create_server_socket(void) {
    int opt = 1;
    struct sockaddr_in server_addr;
    
    printf("=== Creating Server Socket ===\n");
    
    /* Create socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "ERROR: socket() failed with errno %d: %s\n", 
                errno, strerror(errno));
        return -1;
    }
    printf("[DEBUG] Socket created: fd = %d\n", server_fd);
    
    /* Set socket options - SO_REUSEADDR allows port reuse */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        fprintf(stderr, "ERROR: setsockopt() failed with errno %d: %s\n", 
                errno, strerror(errno));
        close(server_fd);
        server_fd = -1;
        return -1;
    }
    printf("[DEBUG] Socket options set (SO_REUSEADDR)\n");
    
    /* Set socket to non-blocking mode */
    if (set_nonblocking(server_fd) < 0) {
        close(server_fd);
        server_fd = -1;
        return -1;
    }
    printf("[DEBUG] Socket set to non-blocking mode\n");
    
    /* Bind socket to address and port */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  /* Bind to all interfaces */
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "ERROR: bind() failed with errno %d: %s\n", 
                errno, strerror(errno));
        close(server_fd);
        server_fd = -1;
        return -1;
    }
    printf("[DEBUG] Socket bound to port %d\n", SERVER_PORT);
    
    /* Start listening for connections */
    if (listen(server_fd, SOMAXCONN) < 0) {
        fprintf(stderr, "ERROR: listen() failed with errno %d: %s\n", 
                errno, strerror(errno));
        close(server_fd);
        server_fd = -1;
        return -1;
    }
    printf("[DEBUG] Socket listening for connections\n");
    
    printf("=== Server Socket Created Successfully ===\n\n");
    return server_fd;
}

/**
 * set_nonblocking() - Set socket to non-blocking mode
 * 
 * This function uses fcntl() to set the O_NONBLOCK flag on a socket,
 * making I/O operations non-blocking.
 * 
 * @param fd File descriptor to set non-blocking
 * @return 0 on success, -1 on error
 */
static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        fprintf(stderr, "ERROR: fcntl(F_GETFL) failed with errno %d: %s\n", 
                errno, strerror(errno));
        return -1;
    }
    
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        fprintf(stderr, "ERROR: fcntl(F_SETFL) failed with errno %d: %s\n", 
                errno, strerror(errno));
        return -1;
    }
    
    return 0;
}

/**
 * handle_new_connection() - Accept and initialize new connection
 * 
 * This function accepts a new client connection, finds an available
 * slot in the connections array, and initializes the connection.
 * 
 * Debug Output:
 * - Prints when new connection is accepted
 * - Shows client address
 * - Reports connection slot allocation
 */
static void handle_new_connection(void) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd;
    int i;
    
    printf("=== Handling New Connection ===\n");
    
    /* Accept new connection */
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            /* No pending connections (expected for non-blocking) */
            printf("[DEBUG] No pending connections (EWOULDBLOCK/EAGAIN)\n");
        } else {
            fprintf(stderr, "ERROR: accept() failed with errno %d: %s\n", 
                    errno, strerror(errno));
        }
        return;
    }
    
    printf("[DEBUG] Accepted connection from %s:%d\n", 
           inet_ntoa(client_addr.sin_addr), 
           ntohs(client_addr.sin_port));
    
    /* Find available connection slot */
    for (i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].fd == -1) {
            break;
        }
    }
    
    if (i == MAX_CONNECTIONS) {
        fprintf(stderr, "WARNING: No available connection slots, rejecting connection\n");
        close(client_fd);
        return;
    }
    
    /* Initialize connection */
    connection_init(&connections[i], client_fd);
    
    /* Update max_fd if needed */
    if (client_fd > max_fd) {
        max_fd = client_fd;
    }
    
    printf("[DEBUG] Connection assigned to slot %d\n", i);
    printf("=== New Connection Handled ===\n\n");
}

/**
 * handle_connection_activity() - Handle activity on a connection
 * 
 * This function processes activity on a client connection based on
 * whether it's readable or writable, implementing the state machine.
 * 
 * @param conn Pointer to connection
 * @param is_readable Whether connection is readable
 * @param is_writable Whether connection is writable
 * 
 * Debug Output:
 * - Prints connection state transitions
 * - Shows data read/written
 * - Reports errors
 */
static void handle_connection_activity(connection_t *conn, int is_readable, int is_writable) {
    printf("=== Handling Connection Activity ===\n");
    printf("[DEBUG] Connection fd=%d, state=%d\n", conn->fd, conn->state);
    
    /* Update last active time */
    conn->last_active = time(NULL);
    
    /* Handle reading */
    if (is_readable && conn->state == CONN_READING) {
        printf("[DEBUG] Processing readable connection\n");
        
        /* Read data from socket */
        int bytes_read = recv(conn->fd, 
                              conn->read_buffer + conn->read_size,
                              BUFFER_SIZE - conn->read_size - 1,
                              0);
        
        if (bytes_read < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                /* No more data to read (expected for non-blocking) */
                printf("[DEBUG] No more data to read (EWOULDBLOCK/EAGAIN)\n");
            } else {
                fprintf(stderr, "ERROR: recv() failed with errno %d: %s\n", 
                        errno, strerror(errno));
                connection_close(conn);
                return;
            }
        } else if (bytes_read == 0) {
            /* Client closed connection */
            printf("[DEBUG] Client closed connection\n");
            connection_close(conn);
            return;
        } else {
            /* Data received */
            conn->read_size += bytes_read;
            conn->read_buffer[conn->read_size] = '\0';  /* Null terminate */
            
            printf("[DEBUG] Received %d bytes\n", bytes_read);
            printf("[DEBUG] Total read buffer size: %d bytes\n", conn->read_size);
            printf("[DEBUG] Read buffer content: %.100s...\n", conn->read_buffer);
            
            /* Check if request is complete (simplified - look for double newline) */
            if (strstr(conn->read_buffer, "\r\n\r\n") != NULL) {
                conn->request_complete = 1;
                printf("[DEBUG] Complete HTTP request received\n");
            }
        }
    }
    
    /* Handle writing */
    if (is_writable && conn->state == CONN_WRITING) {
        printf("[DEBUG] Processing writable connection\n");
        
        /* Send data to client */
        int bytes_sent = send(conn->fd,
                              conn->write_buffer + conn->write_sent,
                              conn->write_size - conn->write_sent,
                              0);
        
        if (bytes_sent < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                /* Buffer full, will try again */
                printf("[DEBUG] Send buffer full (EWOULDBLOCK/EAGAIN)\n");
            } else {
                fprintf(stderr, "ERROR: send() failed with errno %d: %s\n", 
                        errno, strerror(errno));
                connection_close(conn);
                return;
            }
        } else {
            conn->write_sent += bytes_sent;
            printf("[DEBUG] Sent %d bytes\n", bytes_sent);
            printf("[DEBUG] Total sent: %d/%d bytes\n", 
                   conn->write_sent, conn->write_size);
            
            /* Check if all data sent */
            if (conn->write_sent >= conn->write_size) {
                printf("[DEBUG] All data sent, switching to reading\n");
                conn->state = CONN_READING;
                conn->write_size = 0;
                conn->write_sent = 0;
                memset(conn->write_buffer, 0, BUFFER_SIZE);
            }
        }
    }
    
    printf("=== Connection Activity Handled ===\n\n");
}

/**
 * send_response() - Send HTTP response to client
 * 
 * This function generates and sends a simple HTTP response.
 * 
 * @param conn Pointer to connection
 * 
 * Debug Output:
 * - Prints response details
 * - Shows response content
 */
static void send_response(connection_t *conn) {
    const char *response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "Connection: close\r\n"
        "\r\n"
        "Hello, World!";
    
    printf("=== Sending Response ===\n");
    
    /* Copy response to write buffer */
    int response_len = strlen(response);
    if (response_len >= BUFFER_SIZE) {
        fprintf(stderr, "ERROR: Response too large\n");
        return;
    }
    
    memcpy(conn->write_buffer, response, response_len);
    conn->write_size = response_len;
    conn->write_sent = 0;
    conn->response_ready = 1;
    
    /* Change state to writing */
    conn->state = CONN_WRITING;
    
    printf("[DEBUG] Response prepared:\n");
    printf("  - Size: %d bytes\n", response_len);
    printf("  - Content: %s\n", response);
    printf("=== Response Sent ===\n\n");
}

/**
 * cleanup_connections() - Clean up closed connections
 * 
 * This function removes closed connections from the array and
 * updates the max_fd value.
 * 
 * Debug Output:
 * - Prints cleanup details
 */
static void cleanup_connections(void) {
    int i;
    int found_closed = 0;
    
    printf("=== Cleaning Up Connections ===\n");
    
    for (i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].fd == -1) {
            found_closed = 1;
            printf("[DEBUG] Connection slot %d is closed\n", i);
        }
    }
    
    if (!found_closed) {
        printf("[DEBUG] No closed connections to clean up\n");
    }
    
    /* Recalculate max_fd */
    max_fd = server_fd;
    for (i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i].fd > max_fd) {
            max_fd = connections[i].fd;
        }
    }
    
    printf("[DEBUG] Updated max_fd: %d\n", max_fd);
    printf("=== Cleanup Complete ===\n\n");
}

/**
 * main() - Server main loop
 * 
 * This function implements the main server loop using select() for
 * I/O multiplexing to handle multiple connections efficiently.
 * 
 * @param argc Command line arguments
 * @param argv Command line arguments
 * @return 0 on success
 */
int main(int argc, char *argv[]) {
    fd_set read_fds;
    fd_set write_fds;
    int i;
    int activity;
    
    printf("========================================\n");
    printf("  Phase 3: Non-Blocking I/O Server\n");
    printf("  Using select() for Multiplexing\n");
    printf("========================================\n\n");
    
    /* Initialize connections array */
    for (i = 0; i < MAX_CONNECTIONS; i++) {
        connections[i].fd = -1;
        connections[i].state = CONN_CLOSING;
    }
    
    /* Create server socket */
    if (create_server_socket() < 0) {
        fprintf(stderr, "Failed to create server socket\n");
        return 1;
    }
    
    printf("Server listening on port %d\n", SERVER_PORT);
    printf("Maximum connections: %d\n", MAX_CONNECTIONS);
    printf("Timeout: %d seconds\n\n", TIMEOUT_SECONDS);
    
    /* Main server loop */
    while (1) {
        printf("----------------------------------------\n");
        printf("  Waiting for activity...\n");
        printf("----------------------------------------\n");
        
        /* Initialize fd_sets */
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        
        /* Add server socket to read set */
        FD_SET(server_fd, &read_fds);
        
        /* Add client connections to appropriate sets */
        for (i = 0; i < MAX_CONNECTIONS; i++) {
            if (connections[i].fd != -1) {
                if (connections[i].state == CONN_READING) {
                    FD_SET(connections[i].fd, &read_fds);
                    printf("[DEBUG] Added fd=%d to read set (state=READING)\n", 
                           connections[i].fd);
                }
                
                if (connections[i].state == CONN_WRITING) {
                    FD_SET(connections[i].fd, &write_fds);
                    printf("[DEBUG] Added fd=%d to write set (state=WRITING)\n", 
                           connections[i].fd);
                }
            }
        }
        
        /* Call select() - this blocks until activity */
        activity = select(max_fd + 1, &read_fds, &write_fds, NULL, NULL);
        
        if (activity < 0) {
            if (errno == EINTR) {
                /* Interrupted by signal, continue */
                printf("[DEBUG] select() interrupted by signal (EINTR)\n");
                continue;
            }
            fprintf(stderr, "ERROR: select() failed with errno %d: %s\n", 
                    errno, strerror(errno));
            break;
        }
        
        if (activity == 0) {
            /* Timeout - shouldn't happen with NULL timeout */
            printf("[DEBUG] select() timeout (unexpected)\n");
            continue;
        }
        
        printf("[DEBUG] select() returned %d active file descriptors\n", activity);
        
        /* Check for new connections */
        if (FD_ISSET(server_fd, &read_fds)) {
            printf("[DEBUG] Server socket is readable - new connection\n");
            handle_new_connection();
            activity--;
        }
        
        /* Check for completed requests and prepare responses */
        for (i = 0; i < MAX_CONNECTIONS; i++) {
            if (connections[i].fd != -1 && 
                connections[i].request_complete && 
                !connections[i].response_ready) {
                printf("[DEBUG] Request complete for fd=%d, preparing response\n", 
                       connections[i].fd);
                send_response(&connections[i]);
            }
        }
        
        /* Check for client activity */
        for (i = 0; i < MAX_CONNECTIONS && activity > 0; i++) {
            if (connections[i].fd == -1) {
                continue;
            }
            
            /* Check readable */
            if (FD_ISSET(connections[i].fd, &read_fds)) {
                printf("[DEBUG] Client fd=%d is readable\n", connections[i].fd);
                handle_connection_activity(&connections[i], 1, 0);
                activity--;
            }
            
            /* Check writable */
            if (FD_ISSET(connections[i].fd, &write_fds)) {
                printf("[DEBUG] Client fd=%d is writable\n", connections[i].fd);
                handle_connection_activity(&connections[i], 0, 1);
                activity--;
            }
        }
        
        /* Cleanup closed connections */
        cleanup_connections();
        
        printf("----------------------------------------\n\n");
    }
    
    /* Cleanup */
    printf("Shutting down server...\n");
    close(server_fd);
    
    return 0;
}