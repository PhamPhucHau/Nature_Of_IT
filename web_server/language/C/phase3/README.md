# Phase 3: Non-Blocking I/O with select()

## Overview

This phase implements a web server using `select()` for non-blocking I/O to handle multiple connections efficiently.

## Learning Objectives

- Understand `select()` system call for I/O multiplexing
- Learn non-blocking socket operations
- Implement connection state machine
- Handle partial reads/writes
- Manage connection timeouts

## Key Concepts

### select() System Call

```c
int select(int nfds, fd_set *readfds, fd_set *writefds, 
          fd_set *exceptfds, struct timeval *timeout);
```

- Monitors multiple file descriptors for activity
- Blocks until activity occurs or timeout
- Returns number of ready file descriptors

### Non-Blocking Sockets

```c
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

- I/O operations don't block execution
- Must handle EWOULDBLOCK/EAGAIN errors
- Requires state machine for partial operations

### Connection State Machine

```
CONN_READING -> CONN_WRITING -> CONN_CLOSING
     ^                              |
     |------------------------------|
```

## Files

- `connection.h` - Connection structure and state definitions
- `connection.c` - Connection management functions
- `03_select_server.c` - Main server with select() multiplexing
- `makefile` - Build configuration

## Build and Run

```bash
make
./server
```

## Testing

```bash
# Start server
./server

# Test with curl
curl http://localhost:8080/
```

## Debug Output

The server includes detailed debug output showing:
- Connection initialization
- Data read/written
- State transitions
- Error conditions