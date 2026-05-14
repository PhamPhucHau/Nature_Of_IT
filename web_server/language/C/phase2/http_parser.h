#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stddef.h>

// HTTP Methods
typedef enum {
    METHOD_GET,
    METHOD_POST,
    METHOD_PUT,
    METHOD_DELETE,
    METHOD_HEAD,
    METHOD_OPTIONS,
    METHOD_PATCH,
    METHOD_UNKNOWN
} HttpMethod;

// HTTP Request structure
typedef struct {
    HttpMethod method;
    char *path;
    char *version;
    char *headers[32];      // Simple array for headers
    int header_count;
    char *body;
    size_t body_length;
} HttpRequest;

// HTTP Response structure
typedef struct {
    int status_code;
    char *status_text;
    char *headers[32];
    int header_count;
    char *body;
    size_t body_length;
} HttpResponse;

// Function declarations
HttpRequest* http_request_new(void);
void http_request_free(HttpRequest *req);
int http_parse_request(const char *request, HttpRequest *req);

HttpResponse* http_response_new(void);
void http_response_free(HttpResponse *resp);
void http_response_set_status(HttpResponse *resp, int code, const char *text);
void http_response_add_header(HttpResponse *resp, const char *name, const char *value);
void http_response_set_body(HttpResponse *resp, const char *body, size_t length);
char* http_response_build(HttpResponse *resp, size_t *out_length);

// Utility functions
HttpMethod http_method_from_string(const char *str);
const char* http_method_to_string(HttpMethod method);
const char* http_get_content_type(const char *path);
int http_file_exists(const char *path);

#endif // HTTP_PARSER_H