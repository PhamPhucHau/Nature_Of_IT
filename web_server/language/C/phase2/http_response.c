#include "http_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

// Create new HTTP response
HttpResponse* http_response_new(void) {
    HttpResponse *resp = (HttpResponse*)malloc(sizeof(HttpResponse));
    if (!resp) return NULL;
    
    resp->status_code = 200;
    resp->status_text = NULL;
    resp->header_count = 0;
    resp->body = NULL;
    resp->body_length = 0;
    
    return resp;
}

// Free HTTP response
void http_response_free(HttpResponse *resp) {
    if (!resp) return;
    
    if (resp->status_text) free(resp->status_text);
    if (resp->body) free(resp->body);
    
    for (int i = 0; i < resp->header_count; i++) {
        if (resp->headers[i]) free(resp->headers[i]);
    }
    
    free(resp);
}

// Set response status
void http_response_set_status(HttpResponse *resp, int code, const char *text) {
    resp->status_code = code;
    resp->status_text = strdup(text);
}

// Add header to response
void http_response_add_header(HttpResponse *resp, const char *name, const char *value) {
    if (resp->header_count >= 32) return;
    
    char *header = (char*)malloc(strlen(name) + strlen(value) + 4);
    sprintf(header, "%s: %s", name, value);
    resp->headers[resp->header_count++] = header;
}

// Set response body
void http_response_set_body(HttpResponse *resp, const char *body, size_t length) {
    resp->body = (char*)malloc(length + 1);
    strncpy(resp->body, body, length);
    resp->body[length] = '\0';
    resp->body_length = length;
}

// Build complete HTTP response string
char* http_response_build(HttpResponse *resp, size_t *out_length) {
    if (!resp || !out_length) return NULL;
    
    // Calculate total length
    size_t total_len = 0;
    
    // Status line
    total_len += strlen("HTTP/1.1 ") + strlen(resp->status_text) + 2;
    
    // Headers
    for (int i = 0; i < resp->header_count; i++) {
        total_len += strlen(resp->headers[i]) + 2;  // +2 for \r\n
    }
    
    // Body
    total_len += 2;  // \r\n before body
    total_len += resp->body_length;
    
    // Allocate buffer
    char *buffer = (char*)malloc(total_len + 1);
    if (!buffer) return NULL;
    
    // Build response
    size_t pos = 0;
    
    // Status line
    pos += sprintf(buffer + pos, "HTTP/1.1 %s\r\n", resp->status_text);
    
    // Headers
    for (int i = 0; i < resp->header_count; i++) {
        pos += sprintf(buffer + pos, "%s\r\n", resp->headers[i]);
    }
    
    // Empty line
    pos += sprintf(buffer + pos, "\r\n");
    
    // Body
    if (resp->body && resp->body_length > 0) {
        memcpy(buffer + pos, resp->body, resp->body_length);
        pos += resp->body_length;
    }
    
    buffer[pos] = '\0';
    *out_length = pos;
    
    return buffer;
}

// Serve static file
HttpResponse* http_serve_file(const char *filepath) {
    HttpResponse *resp = http_response_new();
    if (!resp) return NULL;
    
    // Check if file exists
    if (!http_file_exists(filepath)) {
        http_response_set_status(resp, 404, "Not Found");
        http_response_add_header(resp, "Content-Type", "text/html");
        const char *body = "<html><body><h1>404 Not Found</h1></body></html>";
        http_response_set_body(resp, body, strlen(body));
        return resp;
    }
    
    // Open and read file
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        http_response_set_status(resp, 500, "Internal Server Error");
        http_response_add_header(resp, "Content-Type", "text/html");
        const char *body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        http_response_set_body(resp, body, strlen(body));
        return resp;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Read file content
    char *content = (char*)malloc(file_size + 1);
    if (!content) {
        fclose(file);
        http_response_set_status(resp, 500, "Internal Server Error");
        return resp;
    }
    
    size_t read_size = fread(content, 1, file_size, file);
    content[read_size] = '\0';
    fclose(file);
    
    // Set response
    http_response_set_status(resp, 200, "OK");
    http_response_add_header(resp, "Content-Type", http_get_content_type(filepath));
    http_response_add_header(resp, "Content-Length", (char*)malloc(20));  // Will be set below
    char length_str[20];
    sprintf(length_str, "%ld", file_size);
    http_response_add_header(resp, "Content-Length", length_str);
    http_response_set_body(resp, content, read_size);
    
    free(content);
    
    return resp;
}

// Create simple HTML response
HttpResponse* http_create_html_response(int status_code, const char *status_text, const char *html_body) {
    HttpResponse *resp = http_response_new();
    if (!resp) return NULL;
    
    http_response_set_status(resp, status_code, status_text);
    http_response_add_header(resp, "Content-Type", "text/html");
    
    // Calculate total body size
    const char *html_template = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head><title>%d %s</title></head>\n"
        "<body>\n"
        "<h1>%d %s</h1>\n"
        "%s\n"
        "</body>\n"
        "</html>\n";
    
    size_t body_len = strlen(html_template) + strlen(status_text) * 2 + strlen(html_body) + 20;
    char *body = (char*)malloc(body_len);
    sprintf(body, html_template, status_code, status_text, status_code, status_text, html_body);
    
    http_response_set_body(resp, body, strlen(body));
    free(body);
    
    return resp;
}

// Create JSON response
HttpResponse* http_create_json_response(int status_code, const char *status_text, const char *json_body) {
    HttpResponse *resp = http_response_new();
    if (!resp) return NULL;
    
    http_response_set_status(resp, status_code, status_text);
    http_response_add_header(resp, "Content-Type", "application/json");
    http_response_set_body(resp, json_body, strlen(json_body));
    
    return resp;
}