/*
 * server.c - TCP Server Implementation
 * 
 * Implements a TCP server for database connections.
 * Handles client connections and query execution.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * SERVER FUNCTIONS
 * ============================================================================ */

DatabaseServer* server_create(uint16_t port) {
    DatabaseServer *server = (DatabaseServer *)malloc(sizeof(DatabaseServer));
    if (!server) return NULL;
    
    memset(server, 0, sizeof(DatabaseServer));
    server->port = port;
    server->socket_fd = -1;
    server->conn_count = 0;
    server->is_running = 0;
    server->total_requests = 0;
    
    return server;
}

void server_destroy(DatabaseServer *server) {
    if (!server) return;
    
    if (server->is_running) {
        server_stop(server);
    }
    
    free(server);
}

int server_start(DatabaseServer *server) {
    if (!server) return -1;
    
    server->is_running = 1;
    server->socket_fd = 1;  /* Simulated socket */
    
    return 0;
}

int server_stop(DatabaseServer *server) {
    if (!server) return -1;
    
    /* Close all connections */
    for (uint32_t i = 0; i < server->conn_count; i++) {
        server->connections[i].status = CONN_CLOSED;
    }
    
    server->is_running = 0;
    server->socket_fd = -1;
    
    return 0;
}

uint32_t server_accept_connection(DatabaseServer *server) {
    if (!server || !server->is_running) return 0;
    
    if (server->conn_count >= MAX_CONNECTIONS) {
        return 0;
    }
    
    Connection *conn = &server->connections[server->conn_count];
    conn->conn_id = server->conn_count + 1;
    conn->client_id = server->conn_count + 1;
    conn->socket_fd = server->conn_count + 1;
    conn->status = CONN_ACTIVE;
    conn->created_at = time(NULL);
    conn->last_activity = time(NULL);
    conn->requests_processed = 0;
    
    server->conn_count++;
    
    return conn->conn_id;
}

int server_close_connection(DatabaseServer *server, uint32_t conn_id) {
    if (!server) return -1;
    
    for (uint32_t i = 0; i < server->conn_count; i++) {
        if (server->connections[i].conn_id == conn_id) {
            server->connections[i].status = CONN_CLOSED;
            return 0;
        }
    }
    
    return -1;
}

int server_process_request(DatabaseServer *server, uint32_t conn_id,
                          ClientRequest *request, ServerResponse *response) {
    if (!server || !request || !response) return -1;
    
    /* Find connection */
    Connection *conn = NULL;
    for (uint32_t i = 0; i < server->conn_count; i++) {
        if (server->connections[i].conn_id == conn_id) {
            conn = &server->connections[i];
            break;
        }
    }
    
    if (!conn) return -1;
    
    /* Process request */
    response->status = RESP_OK;
    response->rows_affected = 0;
    
    switch (request->type) {
        case REQ_QUERY:
            snprintf(response->data, sizeof(response->data),
                    "Query executed: %s", request->query);
            response->rows_affected = 1;
            break;
            
        case REQ_INSERT:
            snprintf(response->data, sizeof(response->data),
                    "Insert executed: %s", request->query);
            response->rows_affected = 1;
            break;
            
        case REQ_UPDATE:
            snprintf(response->data, sizeof(response->data),
                    "Update executed: %s", request->query);
            response->rows_affected = 1;
            break;
            
        case REQ_DELETE:
            snprintf(response->data, sizeof(response->data),
                    "Delete executed: %s", request->query);
            response->rows_affected = 1;
            break;
            
        case REQ_PING:
            snprintf(response->data, sizeof(response->data), "PONG");
            break;
            
        case REQ_DISCONNECT:
            server_close_connection(server, conn_id);
            snprintf(response->data, sizeof(response->data), "Disconnected");
            break;
            
        default:
            response->status = RESP_INVALID;
            snprintf(response->error_message, sizeof(response->error_message),
                    "Invalid request type");
            return -1;
    }
    
    conn->last_activity = time(NULL);
    conn->requests_processed++;
    server->total_requests++;
    
    return 0;
}

uint32_t server_get_connection_count(DatabaseServer *server) {
    if (!server) return 0;
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < server->conn_count; i++) {
        if (server->connections[i].status != CONN_CLOSED) {
            count++;
        }
    }
    
    return count;
}

void server_get_statistics(DatabaseServer *server, uint64_t *total_requests,
                          uint32_t *active_connections) {
    if (!server) return;
    
    if (total_requests) *total_requests = server->total_requests;
    if (active_connections) *active_connections = server_get_connection_count(server);
}

void server_print_status(DatabaseServer *server) {
    if (!server) return;
    
    printf("\n=== Server Status ===\n");
    printf("Port: %u\n", server->port);
    printf("Running: %s\n", server->is_running ? "Yes" : "No");
    printf("Total Connections: %u\n", server->conn_count);
    printf("Active Connections: %u\n", server_get_connection_count(server));
    printf("Total Requests: %lu\n", server->total_requests);
    
    printf("\nConnections:\n");
    for (uint32_t i = 0; i < server->conn_count; i++) {
        Connection *conn = &server->connections[i];
        if (conn->status != CONN_CLOSED) {
            printf("  Connection %u: Client %u, Status %s, Requests %lu\n",
                   conn->conn_id, conn->client_id,
                   conn->status == CONN_ACTIVE ? "ACTIVE" :
                   conn->status == CONN_BUSY ? "BUSY" : "IDLE",
                   conn->requests_processed);
        }
    }
}
