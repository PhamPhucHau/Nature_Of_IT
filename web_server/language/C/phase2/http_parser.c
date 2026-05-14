#include "http_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Create new HTTP request
HttpRequest* http_request_new(void) {
    HttpRequest *req = (HttpRequest*)malloc(sizeof(HttpRequest));
    if (!req) return NULL;
    
    req->method = METHOD_UNKNOWN;
    req->path = NULL;
    req->version = NULL;
    req->header_count = 0;
    req->body = NULL;
    req->body_length = 0;
    
    return req;
}

// Free HTTP request
void http_request_free(HttpRequest *req) {
    if (!req) return;
    
    if (req->path) free(req->path);
    if (req->version) free(req->version);
    if (req->body) free(req->body);
    
    for (int i = 0; i < req->header_count; i++) {
        if (req->headers[i]) free(req->headers[i]);
    }
    
    free(req);
}

// Convert string to HttpMethod
HttpMethod http_method_from_string(const char *str) {
    if (strcmp(str, "GET") == 0) return METHOD_GET;
    if (strcmp(str, "POST") == 0) return METHOD_POST;
    if (strcmp(str, "PUT") == 0) return METHOD_PUT;
    if (strcmp(str, "DELETE") == 0) return METHOD_DELETE;
    if (strcmp(str, "HEAD") == 0) return METHOD_HEAD;
    if (strcmp(str, "OPTIONS") == 0) return METHOD_OPTIONS;
    if (strcmp(str, "PATCH") == 0) return METHOD_PATCH;
    return METHOD_UNKNOWN;
}

// Convert HttpMethod to string
const char* http_method_to_string(HttpMethod method) {
    switch (method) {
        case METHOD_GET: return "GET";
        case METHOD_POST: return "POST";
        case METHOD_PUT: return "PUT";
        case METHOD_DELETE: return "DELETE";
        case METHOD_HEAD: return "HEAD";
        case METHOD_OPTIONS: return "OPTIONS";
        case METHOD_PATCH: return "PATCH";
        default: return "UNKNOWN";
    }
}

// Check if file exists
int http_file_exists(const char *path) {
    FILE *file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

// Get content type based on file extension
const char* http_get_content_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".svg") == 0) return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0) return "image/x-icon";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    
    return "application/octet-stream";
}

// Parse HTTP request
int http_parse_request(const char *request, HttpRequest *req) {
    if (!request || !req) return -1;
    
    const char *line_start = request;
    const char *line_end;
    char line[1024];
    int line_num = 0;
    
    while ((line_end = strchr(line_start, '\n')) != NULL) {
        // Get line length (exclude \r if present)
        size_t line_len = line_end - line_start;
        if (line_len > 0 && line_start[line_len - 1] == '\r') {
            line_len--;
        }
        
        // Copy line to buffer
        if (line_len >= sizeof(line)) line_len = sizeof(line) - 1;
        strncpy(line, line_start, line_len);
        line[line_len] = '\0';
        
        // Skip empty lines (end of headers)
        if (line_len == 0) {
            line_start = line_end + 1;
            break;
        }
        
        // Parse request line (first line)
        if (line_num == 0) {
            char method[16] = {0};
            char path[512] = {0};
            char version[16] = {0};
            
            sscanf(line, "%15s %511s %15s", method, path, version);
            
            req->method = http_method_from_string(method);
            req->path = strdup(path);
            req->version = strdup(version);
        } 
        // Parse headers
        else {
            // Find colon separator
            char *colon = strchr(line, ':');
            if (colon) {
                *colon = '\0';
                char *name = strdup(line);
                char *value = strdup(colon + 1);
                
                // Trim whitespace from value
                while (*value == ' ' || *value == '\t') value++;
                // Remove trailing whitespace
                char *end = value + strlen(value) - 1;
                while (end > value && (*end == ' ' || *end == '\t' || *end == '\r')) {
                    *end = '\0';
                    end--;
                }
                
                if (req->header_count < 32) {
                    req->headers[req->header_count++] = name;
                    // Store value as part of header string for simplicity
                    char *full_header = (char*)malloc(strlen(name) + strlen(value) + 3);
                    sprintf(full_header, "%s: %s", name, value);
                    req->headers[req->header_count - 1] = full_header;
                    
                    free(value);
                }
            }
        }
        
        line_start = line_end + 1;
        line_num++;
    }
    
    // Parse body if present
    if (*line_start != '\0') {
        size_t body_len = strlen(line_start);
        if (body_len > 0) {
            req->body = strdup(line_start);
            req->body_length = body_len;
        }
    }
    
    return 0;
}