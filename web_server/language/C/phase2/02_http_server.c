#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include "http_parser.h"

#define SERVER_PORT 8080
#define BUFFER_SIZE 8192
#define MAX_CLIENTS 10

// Global variables
char *web_root = "./www";  // Default web root directory

// Client handler thread
void *handle_client(void *arg) {
    int client_fd = *(int*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    // Read request
    bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        close(client_fd);
        return NULL;
    }
    buffer[bytes_read] = '\0';
    
    // Parse request
    HttpRequest *req = http_request_new();
    if (http_parse_request(buffer, req) != 0) {
        // Send error response
        const char *error_response = 
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 23\r\n"
            "\r\n"
            "<h1>400 Bad Request</h1>";
        send(client_fd, error_response, strlen(error_response), 0);
        http_request_free(req);
        close(client_fd);
        return NULL;
    }
    
    // Log request
    printf("[%d] %s %s %s\n", client_fd, 
           http_method_to_string(req->method), 
           req->path ? req->path : "(no path)",
           req->version ? req->version : "(no version)");
    
    // Build file path
    char file_path[1024];
    if (strcmp(req->path, "/") == 0) {
        strcpy(file_path, "./www/index.html");
    } else {
        snprintf(file_path, sizeof(file_path), "%s%s", web_root, req->path);
    }
    
    // Remove query string if present
    char *query = strchr(file_path, '?');
    if (query) *query = '\0';
    
    // Check if path is a directory
    struct stat st;
    if (stat(file_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        strcat(file_path, "/index.html");
    }
    
    // Serve file
    HttpResponse *resp = http_serve_file(file_path);
    
    // Build and send response
    size_t response_len;
    char *response = http_response_build(resp, &response_len);
    
    if (response) {
        send(client_fd, response, response_len, 0);
        free(response);
    }
    
    // Cleanup
    http_response_free(resp);
    http_request_free(req);
    close(client_fd);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    if (argc > 1) {
        web_root = argv[1];
    }
    
    printf("=== HTTP Server (Phase 2) ===\n");
    printf("Web root: %s\n", web_root);
    printf("Port: %d\n", SERVER_PORT);
    printf("Starting server...\n\n");
    
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        close(server_fd);
        return 1;
    }
    
    // Bind
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }
    
    // Listen
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }
    
    printf("Server listening on port %d\n", SERVER_PORT);
    printf("Press Ctrl+C to stop\n\n");
    
    // Accept connections
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("Client connected: %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));
        
        // Create thread for client
        int *client_fd_ptr = (int*)malloc(sizeof(int));
        *client_fd_ptr = client_fd;
        
        pthread_t thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        
        if (pthread_create(&thread, &attr, handle_client, client_fd_ptr) != 0) {
            perror("Thread creation failed");
            close(client_fd);
            free(client_fd_ptr);
        }
        
        pthread_attr_destroy(&attr);
    }
    
    // Cleanup (unreachable in normal flow)
    close(server_fd);
    return 0;
}