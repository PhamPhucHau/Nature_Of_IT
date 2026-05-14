# ⚡ Quick Reference Guide

## 🔧 Essential C Functions for Web Server

### Socket Creation & Setup
```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Create socket
int socket(int domain, int type, int protocol);
// domain: AF_INET (IPv4), AF_INET6 (IPv6)
// type: SOCK_STREAM (TCP), SOCK_DGRAM (UDP)
// protocol: 0 (default)

// Bind socket to address
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

// Listen for connections
int listen(int sockfd, int backlog);
// backlog: max pending connections

// Accept connection
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

// Connect to server
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

### Socket Options
```c
#include <sys/socket.h>

// Set socket option
int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen);

// Common options:
int reuse = 1;
setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

int timeout = 5000;  // 5 seconds
setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

// Set non-blocking
int flags = fcntl(sock, F_GETFL, 0);
fcntl(sock, F_SETFL, flags | O_NONBLOCK);
```

### I/O Operations
```c
#include <unistd.h>

// Read from socket
ssize_t read(int fd, void *buf, size_t count);
// Returns: bytes read, 0 (EOF), -1 (error)

// Write to socket
ssize_t write(int fd, const void *buf, size_t count);
// Returns: bytes written, -1 (error)

// Close socket
int close(int fd);

// Shutdown socket
int shutdown(int sockfd, int how);
// how: SHUT_RD, SHUT_WR, SHUT_RDWR
```

### I/O Multiplexing

#### select()
```c
#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);

// File descriptor set operations
void FD_ZERO(fd_set *set);           // Clear set
void FD_SET(int fd, fd_set *set);    // Add fd to set
void FD_CLR(int fd, fd_set *set);    // Remove fd from set
int FD_ISSET(int fd, fd_set *set);   // Check if fd in set

// Example
fd_set readfds;
FD_ZERO(&readfds);
FD_SET(server_fd, &readfds);
FD_SET(client_fd, &readfds);

struct timeval tv;
tv.tv_sec = 5;
tv.tv_usec = 0;

int activity = select(max_fd + 1, &readfds, NULL, NULL, &tv);
if (FD_ISSET(server_fd, &readfds)) {
    // Server socket ready
}
```

#### epoll()
```c
#include <sys/epoll.h>

// Create epoll instance
int epoll_create1(int flags);
// flags: 0 or EPOLL_CLOEXEC

// Add/modify/remove file descriptor
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
// op: EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL

// Wait for events
int epoll_wait(int epfd, struct epoll_event *events,
               int maxevents, int timeout);
// timeout: -1 (block), 0 (non-block), >0 (milliseconds)

// Example
struct epoll_event ev, events[MAX_EVENTS];
int epfd = epoll_create1(0);

ev.events = EPOLLIN;
ev.data.fd = server_fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
for (int i = 0; i < nfds; i++) {
    if (events[i].events & EPOLLIN) {
        // Read event
    }
    if (events[i].events & EPOLLOUT) {
        // Write event
    }
}
```

### Process Management
```c
#include <unistd.h>
#include <sys/wait.h>

// Fork process
pid_t fork(void);
// Returns: child PID (parent), 0 (child), -1 (error)

// Execute program
int execve(const char *filename, char *const argv[],
           char *const envp[]);
// Replaces current process image

// Wait for child
pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);

// Get process ID
pid_t getpid(void);
pid_t getppid(void);

// Exit process
void exit(int status);
void _exit(int status);  // Immediate exit
```

### Signal Handling
```c
#include <signal.h>

// Set signal handler
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);

// More reliable signal handling
int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact);

// Example
void handle_sigterm(int sig) {
    printf("Received SIGTERM\n");
    exit(0);
}

signal(SIGTERM, handle_sigterm);

// Common signals
SIGTERM  // Termination signal
SIGKILL  // Kill signal (cannot be caught)
SIGCHLD  // Child process terminated
SIGHUP   // Hangup signal
SIGUSR1  // User-defined signal 1
SIGUSR2  // User-defined signal 2
```

### Memory Management
```c
#include <stdlib.h>
#include <string.h>

// Allocate memory
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);  // Zeroed
void *realloc(void *ptr, size_t size);

// Free memory
void free(void *ptr);

// Memory operations
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

// String operations
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
```

---

## 📋 HTTP Protocol Quick Reference

### Request Format
```
METHOD PATH HTTP/VERSION\r\n
Header1: Value1\r\n
Header2: Value2\r\n
\r\n
[optional body]
```

### Response Format
```
HTTP/VERSION STATUS_CODE STATUS_TEXT\r\n
Header1: Value1\r\n
Header2: Value2\r\n
Content-Length: 1234\r\n
\r\n
[response body]
```

### Common Headers
```
Host: example.com
User-Agent: curl/7.64.1
Accept: text/html,application/xhtml+xml
Accept-Encoding: gzip, deflate
Connection: keep-alive
Content-Type: text/html; charset=utf-8
Content-Length: 1234
Server: nginx/1.18.0
Date: Mon, 14 May 2026 10:30:00 GMT
Cache-Control: max-age=3600
```

### Status Codes
```
1xx: Informational
  100 Continue
  101 Switching Protocols

2xx: Success
  200 OK
  201 Created
  204 No Content

3xx: Redirection
  301 Moved Permanently
  302 Found
  304 Not Modified

4xx: Client Error
  400 Bad Request
  401 Unauthorized
  403 Forbidden
  404 Not Found

5xx: Server Error
  500 Internal Server Error
  502 Bad Gateway
  503 Service Unavailable
```

---

## 🔍 Debugging Commands

### GDB
```bash
# Start debugger
gdb ./program

# Set breakpoint
(gdb) break main
(gdb) break file.c:10
(gdb) break function_name

# Run program
(gdb) run [args]
(gdb) run < input.txt

# Step through code
(gdb) next          # Next line
(gdb) step          # Step into function
(gdb) continue      # Continue execution
(gdb) finish        # Run until return

# Inspect variables
(gdb) print variable
(gdb) print *pointer
(gdb) print array[0]
(gdb) print &variable

# Stack trace
(gdb) backtrace
(gdb) frame 0
(gdb) up
(gdb) down

# Quit
(gdb) quit
```

### Valgrind
```bash
# Memory leak check
valgrind --leak-check=full ./program

# Detailed report
valgrind --leak-check=full --show-leak-kinds=all ./program

# Track origins
valgrind --track-origins=yes ./program

# Save to file
valgrind --log-file=valgrind.log ./program
```

### strace
```bash
# Trace all syscalls
strace ./program

# Trace specific syscalls
strace -e trace=socket,bind,listen ./program

# Save output
strace -o trace.log ./program

# Show time
strace -t ./program

# Count syscalls
strace -c ./program
```

---

## 📊 Performance Testing

### Apache Bench
```bash
# Basic test
ab -n 1000 -c 10 http://localhost:8080/

# With timeout
ab -n 1000 -c 10 -s 30 http://localhost:8080/

# POST request
ab -n 1000 -c 10 -p data.txt http://localhost:8080/

# Options:
# -n: Total requests
# -c: Concurrent requests
# -t: Timeout (seconds)
# -p: POST data file
# -H: Custom header
```

### wrk
```bash
# Basic test
wrk -t4 -c100 -d30s http://localhost:8080/

# With script
wrk -t4 -c100 -d30s -s script.lua http://localhost:8080/

# Options:
# -t: Number of threads
# -c: Number of connections
# -d: Duration
# -s: Lua script
```

### curl
```bash
# Simple request
curl http://localhost:8080/

# Show headers
curl -i http://localhost:8080/

# Verbose
curl -v http://localhost:8080/

# POST request
curl -X POST -d "data" http://localhost:8080/

# Custom header
curl -H "X-Custom: value" http://localhost:8080/

# Save output
curl -o file.html http://localhost:8080/
```

---

## 🛠️ Compilation Flags

```bash
# Basic compilation
gcc -o program program.c

# With warnings
gcc -Wall -Wextra -o program program.c

# With debug symbols
gcc -g -o program program.c

# With optimization
gcc -O2 -o program program.c

# With all features
gcc -Wall -Wextra -g -O2 -o program program.c

# Link libraries
gcc -o program program.c -lpthread -lm

# Define macro
gcc -DDEBUG -o program program.c

# Include directory
gcc -I/path/to/include -o program program.c

# Library directory
gcc -L/path/to/lib -o program program.c -lmylib
```

---

## 📝 Common Patterns

### Non-blocking Read
```c
ssize_t n = read(fd, buf, size);
if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No data available, try again later
    } else {
        // Error
        perror("read");
    }
} else if (n == 0) {
    // Connection closed
} else {
    // Data received
}
```

### Non-blocking Write
```c
ssize_t n = write(fd, buf, size);
if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Buffer full, try again later
    } else {
        // Error
        perror("write");
    }
} else if (n < size) {
    // Partial write, need to write remaining
}
```

### Safe Signal Handling
```c
#include <signal.h>

void signal_handler(int sig) {
    // Only use async-signal-safe functions
    write(STDOUT_FILENO, "Signal received\n", 16);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    
    // Main loop
}
```

### Connection Timeout
```c
struct timeval tv;
tv.tv_sec = 5;      // 5 seconds
tv.tv_usec = 0;

setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
```

---

## 🚀 Quick Start Template

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 8080
#define BACKLOG 10
#define BUFFER_SIZE 4096

int main() {
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }
    
    // Set socket options
    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    // Bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return 1;
    }
    
    // Listen
    if (listen(server_fd, BACKLOG) == -1) {
        perror("listen");
        return 1;
    }
    
    printf("Server listening on port %d\n", PORT);
    
    // Accept connections
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }
        
        // Read request
        char buffer[BUFFER_SIZE];
        ssize_t n = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("Received: %s\n", buffer);
        }
        
        // Send response
        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        write(client_fd, response, strlen(response));
        
        close(client_fd);
    }
    
    close(server_fd);
    return 0;
}
```

---

## 📚 Useful Macros

```c
// Error checking
#define CHECK(expr, msg) \
    if (!(expr)) { perror(msg); exit(1); }

// Memory allocation
#define MALLOC(type, count) \
    (type *)malloc(sizeof(type) * (count))

#define CALLOC(type, count) \
    (type *)calloc((count), sizeof(type))

// Min/Max
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Array size
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Offset of member in struct
#define OFFSET_OF(type, member) \
    ((size_t)&((type *)0)->member)
```

---

## 🎯 Next Steps

1. **Bookmark this page** for quick reference
2. **Study the patterns** - understand each one
3. **Practice the commands** - run them yourself
4. **Start coding** - Phase 1 awaits!

Good luck! 🚀
