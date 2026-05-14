# 📚 Learning Resources

## 📖 Essential Books

### 1. Unix Network Programming (Stevens & Rago)
- **Volume 1**: Networking APIs - Sockets and XTI
- **Volume 2**: Interprocess Communications
- **Why**: The bible for socket programming
- **Chapters to focus on**:
  - Ch 3: Sockets Introduction
  - Ch 4: Elementary TCP Sockets
  - Ch 6: I/O Multiplexing (select, poll, epoll)
  - Ch 12: IPv4 and IPv6 Interoperability

### 2. The Linux Programming Interface (Michael Kerrisk)
- **Why**: Comprehensive guide to Linux system programming
- **Chapters to focus on**:
  - Ch 56: Sockets: Introduction
  - Ch 57: Sockets: Unix Domain
  - Ch 58: Sockets: TCP/IP
  - Ch 61: Fundamentals of Process
  - Ch 63: Process Termination

### 3. Advanced Programming in the UNIX Environment (Stevens & Rago)
- **Why**: Deep dive into UNIX/Linux internals
- **Chapters to focus on**:
  - Ch 8: Process Control
  - Ch 10: Signals
  - Ch 14: Advanced I/O

---

## 🌐 Online Resources

### Official Documentation
- **Linux man pages**: https://man7.org/
  - `man socket` - Socket creation
  - `man epoll` - Event polling
  - `man fork` - Process creation
  - `man signal` - Signal handling

- **POSIX Standard**: https://pubs.opengroup.org/onlinepubs/9699919799/
  - Complete POSIX specification
  - Socket API reference

### Tutorials & Guides

#### Socket Programming
- **Beej's Guide to Network Programming**: https://beej.us/guide/bgnet/
  - Beginner-friendly introduction
  - Practical examples
  - IPv4 and IPv6

- **Linux Socket Programming**: https://www.geeksforgeeks.org/socket-programming-cc/
  - Step-by-step tutorials
  - Code examples

#### Event-Driven Programming
- **epoll Tutorial**: https://man7.org/linux/man-pages/man7/epoll.7.html
  - Official epoll documentation
  - Usage patterns

- **Event-Driven Architecture**: https://en.wikipedia.org/wiki/Event-driven_architecture
  - Conceptual overview
  - Design patterns

#### Process Management
- **Fork and Exec**: https://www.geeksforgeeks.org/fork-in-c/
  - Process creation
  - Parent-child relationships

- **Signal Handling**: https://www.gnu.org/software/libc/manual/html_node/Signal-Handling.html
  - Signal basics
  - Safe signal handling

### Video Tutorials
- **YouTube - Socket Programming in C**: Various channels
- **Coursera - Systems Programming**: University courses
- **edX - Linux System Programming**: Comprehensive courses

---

## 🔍 NGINX Source Code

### Where to Find It
- **GitHub**: https://github.com/nginx/nginx
- **Official**: https://nginx.org/download/

### Key Files to Study
```
nginx/
├── src/
│   ├── core/
│   │   ├── ngx_connection.c      # Connection handling
│   │   ├── ngx_cycle.c           # Main event loop
│   │   └── ngx_process.c         # Process management
│   ├── event/
│   │   ├── ngx_event.c           # Event core
│   │   ├── modules/
│   │   │   ├── ngx_epoll_module.c    # epoll implementation
│   │   │   └── ngx_select_module.c   # select implementation
│   ├── http/
│   │   ├── ngx_http_request.c    # HTTP request handling
│   │   └── ngx_http_parse.c      # HTTP parsing
│   └── os/
│       └── unix/
│           ├── ngx_socket.c      # Socket operations
│           └── ngx_process.c     # Process operations
```

### How to Study NGINX Code
1. **Start with main()**: `src/core/nginx.c`
2. **Follow the flow**: Trace execution path
3. **Focus on one module**: Study event loop first
4. **Compare with your code**: See how they do it
5. **Read comments**: NGINX has good inline documentation

---

## 🛠️ Development Tools

### Debuggers
- **GDB (GNU Debugger)**
  - https://www.gnu.org/software/gdb/
  - Tutorial: https://www.gnu.org/software/gdb/documentation/
  - Cheat sheet: https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf

- **LLDB (LLVM Debugger)**
  - https://lldb.llvm.org/
  - Better for macOS

### Memory Tools
- **Valgrind**
  - https://valgrind.org/
  - Memory leak detection
  - Invalid memory access detection

- **AddressSanitizer**
  - https://github.com/google/sanitizers
  - Compile-time instrumentation
  - Faster than Valgrind

### Profilers
- **perf (Linux Performance Events)**
  - https://perf.wiki.kernel.org/
  - CPU profiling
  - Flamegraph generation

- **Flamegraph**
  - https://github.com/brendangregg/FlameGraph
  - Visualize performance data

### System Call Tracing
- **strace**
  - https://strace.io/
  - Trace system calls
  - Understand kernel interactions

- **ltrace**
  - Trace library calls

---

## 📊 HTTP Protocol Reference

### HTTP/1.1 Specification
- **RFC 7230-7235**: https://tools.ietf.org/html/rfc7230
  - Message Syntax and Routing
  - Semantics and Content
  - Conditional Requests
  - Range Requests
  - Caching
  - Authentication

### HTTP Request Format
```
GET /path HTTP/1.1
Host: example.com
User-Agent: curl/7.64.1
Accept: */*

[optional body]
```

### HTTP Response Format
```
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 1234
Connection: keep-alive

[response body]
```

### Common Status Codes
- 200 OK
- 301 Moved Permanently
- 400 Bad Request
- 404 Not Found
- 500 Internal Server Error

---

## 🎓 Learning Path

### Week 1-2: Fundamentals
- [ ] Read Beej's Guide to Network Programming
- [ ] Study socket API basics
- [ ] Understand TCP/IP model
- [ ] Complete Phase 1

### Week 3: HTTP Protocol
- [ ] Read HTTP/1.1 RFC (sections 3-5)
- [ ] Understand request/response format
- [ ] Study HTTP headers
- [ ] Complete Phase 2

### Week 4: I/O Multiplexing
- [ ] Read UNP Chapter 6
- [ ] Understand select(), poll(), epoll()
- [ ] Study non-blocking I/O
- [ ] Complete Phase 3

### Week 5: Event-Driven Architecture
- [ ] Study epoll in detail
- [ ] Read NGINX event loop code
- [ ] Understand event callbacks
- [ ] Complete Phase 4

### Week 6: Process Management
- [ ] Read UNP Chapter 12
- [ ] Study fork(), exec(), signals
- [ ] Understand master-worker pattern
- [ ] Complete Phase 5

### Week 7: Optimization
- [ ] Study memory management
- [ ] Learn performance profiling
- [ ] Understand connection pooling
- [ ] Complete Phase 6

### Week 8: Advanced Features
- [ ] Study reverse proxy patterns
- [ ] Learn load balancing algorithms
- [ ] Understand logging systems
- [ ] Complete Phase 7

---

## 💻 Code Examples

### Basic Socket Server
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);
    
    int client_fd = accept(server_fd, NULL, NULL);
    
    char buffer[1024] = {0};
    read(client_fd, buffer, 1024);
    printf("Received: %s\n", buffer);
    
    close(client_fd);
    close(server_fd);
    
    return 0;
}
```

### Non-Blocking Socket
```c
#include <fcntl.h>

// Set socket to non-blocking
int flags = fcntl(sock, F_GETFL, 0);
fcntl(sock, F_SETFL, flags | O_NONBLOCK);

// Handle EAGAIN
ssize_t n = read(sock, buf, size);
if (n == -1 && errno == EAGAIN) {
    // No data available, try again later
}
```

### epoll Event Loop
```c
#include <sys/epoll.h>

int epfd = epoll_create1(0);

struct epoll_event ev, events[MAX_EVENTS];
ev.events = EPOLLIN;
ev.data.fd = server_fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

while (1) {
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    for (int i = 0; i < nfds; i++) {
        if (events[i].data.fd == server_fd) {
            // New connection
        } else {
            // Data available
        }
    }
}
```

---

## 🔗 Useful Links

### Documentation
- Linux man pages: https://man7.org/
- POSIX specification: https://pubs.opengroup.org/
- GCC documentation: https://gcc.gnu.org/onlinedocs/

### Communities
- Stack Overflow: https://stackoverflow.com/questions/tagged/c
- Linux Kernel Mailing List: https://lkml.org/
- NGINX Community: https://nginx.org/en/community.html

### Tools & Utilities
- GDB: https://www.gnu.org/software/gdb/
- Valgrind: https://valgrind.org/
- perf: https://perf.wiki.kernel.org/
- strace: https://strace.io/

### Performance Testing
- Apache Bench: https://httpd.apache.org/docs/2.4/programs/ab.html
- wrk: https://github.com/wg/wrk
- hey: https://github.com/rakyll/hey

---

## 📝 Recommended Reading Order

1. **Start**: Beej's Guide to Network Programming
2. **Then**: UNP Chapter 3-4 (Socket basics)
3. **Then**: UNP Chapter 6 (I/O Multiplexing)
4. **Then**: NGINX source code (event loop)
5. **Then**: Linux Programming Interface (Process management)
6. **Finally**: Advanced topics (optimization, security)

---

## 🎯 Practice Exercises

### Phase 1 Exercises
- [ ] Create a socket and bind to port 8080
- [ ] Accept a connection and read data
- [ ] Send a response back to client
- [ ] Handle multiple sequential connections

### Phase 2 Exercises
- [ ] Parse HTTP request line
- [ ] Extract headers from request
- [ ] Build HTTP response with headers
- [ ] Serve a static file

### Phase 3 Exercises
- [ ] Use select() to monitor multiple sockets
- [ ] Handle non-blocking reads/writes
- [ ] Implement connection timeout
- [ ] Handle partial reads/writes

### Phase 4 Exercises
- [ ] Replace select() with epoll()
- [ ] Implement event callback system
- [ ] Handle EPOLLIN and EPOLLOUT
- [ ] Benchmark epoll vs select()

### Phase 5 Exercises
- [ ] Create master process
- [ ] Fork worker processes
- [ ] Handle signals (SIGTERM, SIGHUP)
- [ ] Implement graceful shutdown

### Phase 6 Exercises
- [ ] Parse configuration file
- [ ] Implement memory pooling
- [ ] Optimize buffer management
- [ ] Benchmark performance

### Phase 7 Exercises
- [ ] Implement reverse proxy
- [ ] Add round-robin load balancing
- [ ] Implement access logging
- [ ] Add performance metrics

---

## 🚀 Next Steps

1. **Install tools** (see SETUP.md)
2. **Read Beej's Guide** (start with basics)
3. **Study NGINX source** (understand architecture)
4. **Start Phase 1** (write code)
5. **Debug and test** (use tools)
6. **Benchmark** (measure performance)
7. **Iterate** (improve and optimize)

Happy learning! 📚
