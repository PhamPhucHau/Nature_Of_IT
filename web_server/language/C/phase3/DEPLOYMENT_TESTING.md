# Phase 3: Non-Blocking I/O Server - Deployment & Testing Guide

## Overview

This document provides step-by-step instructions for building, deploying, and testing the Phase 3 web server implementation using `select()` for I/O multiplexing.

---

## Build Instructions

### Prerequisites

- macOS or Linux operating system
- GCC compiler (or any C compiler)
- Make utility
- cURL for testing

### Build Steps

```bash
# Navigate to the phase3 directory
cd "/Users/hauphamphuc/Documents/Application/Knowleage and AI/Nature_Of_IT/web_server/language/C/phase3"

# Clean previous build artifacts
make clean

# Build the server
make
```

### Expected Build Output

```
gcc -Wall -Wextra -O2 -g connection.c 03_select_server.c -o server -lpthread
```

No errors or warnings should appear (except for unused parameter warnings which are harmless).

---

## Deployment

### Running the Server

```bash
# Start the server
./server
```

### Expected Server Output

```
========================================
  Phase 3: Non-Blocking I/O Server
  Using select() for Multiplexing
========================================

=== Creating Server Socket ===
[DEBUG] Socket created: fd = 3
[DEBUG] Socket options set (SO_REUSEADDR)
[DEBUG] Socket set to non-blocking mode
[DEBUG] Socket bound to port 8080
[DEBUG] Socket listening for connections
=== Server Socket Created Successfully ===

Server listening on port 8080
Maximum connections: 10
Timeout: 30 seconds

----------------------------------------
  Waiting for activity...
----------------------------------------
```

### Running Server in Background

For testing with curl, run the server in the background:

```bash
# Start server in background
nohup ./server > /tmp/server.log 2>&1 &

# Verify server is running
ps aux | grep 03_select_server

# Check server logs
tail -f /tmp/server.log
```

### Stopping the Server

```bash
# Find and kill the server process
pkill -f "03_select_server"

# Or kill by PID
kill <PID>
```

---

## Testing

### Test 1: Basic HTTP Request with curl

```bash
# Test with verbose output
curl -v http://localhost:8080/

# Expected response:
# HTTP/1.1 200 OK
# Content-Type: text/plain
# Content-Length: 13
# Connection: close
#
# Hello, World!
```

### Test 2: Test with Different Paths

```bash
# Test root path
curl http://localhost:8080/

# Test other paths (will return same response for now)
curl http://localhost:8080/about
curl http://localhost:8080/api/data
```

### Test 3: Test Multiple Concurrent Connections

```bash
# Open multiple terminals and run:
for i in {1..5}; do
    curl -s http://localhost:8080/ &
done
wait
echo "All requests completed"
```

### Test 4: Test with Browser

1. Open a web browser
2. Navigate to `http://localhost:8080/`
3. You should see: `Hello, World!`

### Test 5: Test Error Handling

```bash
# Test with invalid HTTP version
curl -v http://localhost:8080/ --http1.0

# Test with large request
curl -v -H "X-Custom-Header: $(printf 'a%.0s' {1..2000})" http://localhost:8080/
```

---

## Debugging

### Check Server Logs

```bash
# View server output
tail -f /tmp/server.log

# Or check if server is running
ps aux | grep 03_select_server
```

### Common Issues

#### Issue 1: Port Already in Use

```
ERROR: bind() failed with errno 48: Address already in use
```

**Solution:**
```bash
# Kill existing server process
pkill -f "03_select_server"

# Or find and kill process using port 8080
lsof -ti:8080 | xargs kill -9
```

#### Issue 2: Connection Refused

```
curl: (7) Failed to connect to localhost port 8080: Connection refused
```

**Solution:**
- Verify server is running: `ps aux | grep 03_select_server`
- Check server logs for errors
- Ensure no firewall is blocking port 8080

#### Issue 3: Request Timeout

**Solution:**
- Check server is running in background properly
- Verify no blocking operations in server code
- Check network configuration

### Enable More Debug Output

Modify `03_select_server.c`:

```c
// Add more detailed logging in key functions
printf("[DEBUG] Full buffer content: %s\n", conn->read_buffer);
printf("[DEBUG] Connection state: %d\n", conn->state);
```

---

## Performance Testing

### Test Request Rate

```bash
# Send 100 requests
ab -n 100 -c 10 http://localhost:8080/
```

### Test Concurrent Connections

```bash
# Test with 50 concurrent connections
ab -n 1000 -c 50 http://localhost:8080/
```

---

## Verification Checklist

- [ ] Server compiles without errors
- [ ] Server starts and listens on port 8080
- [ ] Server accepts connections (visible in logs)
- [ ] Server receives HTTP requests (visible in logs)
- [ ] Server sends HTTP responses (visible in logs)
- [ ] curl returns HTTP 200 status
- [ ] Browser displays "Hello, World!"
- [ ] Multiple concurrent connections work
- [ ] Server handles connection cleanup properly

---

## Learning Objectives

This implementation demonstrates:

1. **I/O Multiplexing**: Using `select()` to monitor multiple sockets
2. **Non-blocking Sockets**: Setting socket flags with `fcntl()`
3. **Connection State Machine**: Tracking connection states (READING, WRITING, CLOSING)
4. **Buffer Management**: Handling partial reads/writes
5. **Event-driven Architecture**: Responding to socket events rather than blocking

---

## Next Steps

After verifying this implementation works:

1. Implement HTTP request parsing
2. Add static file serving
3. Implement proper response building based on request path
4. Add timeout handling for idle connections
5. Implement connection pooling for better performance
