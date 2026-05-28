/*
 * client.c - Database Client Implementation
 * 
 * Implements a client library for connecting to database server.
 * Provides API for sending queries and receiving results.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "client.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ============================================================================
 * CLIENT FUNCTIONS
 * ============================================================================ */

DatabaseClient* client_create(const char *host, uint16_t port) {
    DatabaseClient *client = (DatabaseClient *)malloc(sizeof(DatabaseClient));
    if (!client) return NULL;
    
    memset(client, 0, sizeof(DatabaseClient));
    strncpy(client->host, host, sizeof(client->host) - 1);
    client->port = port;
    client->socket_fd = -1;
    client->status = CLIENT_DISCONNECTED;
    client->queries_sent = 0;
    
    return client;
}

void client_destroy(DatabaseClient *client) {
    if (!client) return;
    
    if (client->status == CLIENT_CONNECTED) {
        client_disconnect(client);
    }
    
    free(client);
}

int client_connect(DatabaseClient *client) {
    if (!client) return -1;
    
    client->status = CLIENT_CONNECTING;
    client->socket_fd = 1;  /* Simulated socket */
    client->status = CLIENT_CONNECTED;
    client->connected_at = time(NULL);
    
    return 0;
}

int client_disconnect(DatabaseClient *client) {
    if (!client) return -1;
    
    if (client->socket_fd >= 0) {
        client->socket_fd = -1;
    }
    
    client->status = CLIENT_DISCONNECTED;
    
    return 0;
}

int client_is_connected(DatabaseClient *client) {
    if (!client) return 0;
    return client->status == CLIENT_CONNECTED;
}

int client_execute_query(DatabaseClient *client, const char *query,
                        QueryResult *result) {
    if (!client || !query || !result) return -1;
    
    if (!client_is_connected(client)) {
        result->success = 0;
        snprintf(result->error_message, sizeof(result->error_message),
                "Not connected to server");
        return -1;
    }
    
    /* Simulate query execution */
    result->success = 1;
    result->rows_affected = 1;
    snprintf(result->result_data, sizeof(result->result_data),
            "Query executed: %s", query);
    
    client->queries_sent++;
    
    return 0;
}

int client_ping(DatabaseClient *client) {
    if (!client) return -1;
    
    if (!client_is_connected(client)) {
        return -1;
    }
    
    /* Simulate ping */
    return 0;
}

ClientStatus client_get_status(DatabaseClient *client) {
    if (!client) return CLIENT_DISCONNECTED;
    return client->status;
}

uint64_t client_get_query_count(DatabaseClient *client) {
    if (!client) return 0;
    return client->queries_sent;
}

void client_print_info(DatabaseClient *client) {
    if (!client) return;
    
    printf("\n=== Client Information ===\n");
    printf("Host: %s\n", client->host);
    printf("Port: %u\n", client->port);
    printf("Status: %s\n",
           client->status == CLIENT_DISCONNECTED ? "DISCONNECTED" :
           client->status == CLIENT_CONNECTING ? "CONNECTING" :
           client->status == CLIENT_CONNECTED ? "CONNECTED" : "ERROR");
    printf("Queries Sent: %lu\n", client->queries_sent);
    
    if (client->status == CLIENT_CONNECTED) {
        time_t now = time(NULL);
        printf("Connected for: %ld seconds\n", now - client->connected_at);
    }
}
