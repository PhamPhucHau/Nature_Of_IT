# Phase 2: HTTP Protocol Server

## Overview
This phase implements a complete HTTP server with request parsing and response building capabilities.

## Files
- `http_parser.h` - Header file with data structures and function declarations
- `http_parser.c` - HTTP request parsing implementation
- `http_response.c` - HTTP response building implementation
- `02_http_server.c` - Main server with multi-threaded client handling
- `makefile` - Build configuration

## Features

### HTTP Request Parsing
- Parses request line (method, path, version)
- Parses all headers
- Handles request body
- Supports all HTTP methods (GET, POST, PUT, DELETE, HEAD, OPTIONS, PATCH)

### HTTP Response Building
- Builds proper HTTP response headers
- Sets appropriate status codes (200, 404, 500)
- Serves static files with correct Content-Type
- Supports multi-threaded connections

### Static File Serving
- Automatically detects file types
- Serves HTML, CSS, JS, images, and other files
- Returns 404 for non-existent files
- Returns 500 for server errors

## Build and Run

```bash
# Build the server
make build

# Setup test directory and run
make all

# Or run manually
make setup
./server
```

## Testing

### Using curl

```bash
# Test basic HTML page
curl http://localhost:8080/

# Test 404 error
curl http://localhost:8080/nonexistent.html

# Test with verbose output
curl -v http://localhost:8080/

# Test POST request
curl -X POST -d "key=value" http://localhost:8080/

# Test different content types
curl -H "Accept: text/html" http://localhost:8080/
```

### Using browser
Open `http://localhost:8080/` in your browser to see the test page.

## HTTP Protocol Details

### Request Format
```
METHOD /path HTTP/1.1
Header-Name: Header-Value
...

Body content
```

### Response Format
```
HTTP/1.1 Status-Code Status-Text
Header-Name: Header-Value
...

Body content
```

## Status Codes
- 200 OK - Request successful
- 400 Bad Request - Invalid request format
- 404 Not Found - Resource not found
- 500 Internal Server Error - Server error

## Content Types Supported
- text/html - HTML files
- text/css - CSS files
- application/javascript - JavaScript files
- application/json - JSON files
- image/png, image/jpeg, image/gif - Images
- text/plain - Plain text files

## Multi-threading
The server creates a new thread for each client connection, allowing it to handle multiple requests concurrently.