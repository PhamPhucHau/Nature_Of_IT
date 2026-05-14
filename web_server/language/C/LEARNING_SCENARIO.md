# 🚀 Learning Scenario: Building an NGINX-like Web Server in C

## 📚 Overview
This scenario guides you through building a high-performance web server in C, learning the core concepts that make NGINX powerful: event-driven architecture, non-blocking I/O, and efficient connection handling.

---

## 🎯 Learning Objectives

By completing this scenario, you will understand:
- ✅ Socket programming (TCP/IP)
- ✅ Event-driven architecture (epoll/select)
- ✅ Non-blocking I/O operations
- ✅ HTTP protocol parsing
- ✅ Process/thread management
- ✅ Memory management in C
- ✅ Performance optimization techniques

---

## 📊 Project Phases

### Phase 1: Foundation (Weeks 1-2)
**Goal**: Build a basic single-threaded HTTP server

#### 1.1 - Basic Socket Server
- [ ] Create a TCP socket
- [ ] Bind to port 8080
- [ ] Listen for connections
- [ ] Accept client connections
- [ ] Send a simple HTTP response

**Files to create**:
- `phase1/01_basic_socket.c` - Hello world socket server
- `phase1/Makefile` - Build configuration

**Learning**: TCP/IP basics, socket API

---

### Phase 2: HTTP Protocol (Weeks 2-3)
**Goal**: Parse and respond to HTTP requests properly

#### 2.1 - HTTP Request Parser
- [ ] Read HTTP request from socket
- [ ] Parse request line (method, path, version)
- [ ] Parse headers
- [ ] Handle request body

#### 2.2 - HTTP Response Builder
- [ ] Build proper HTTP response headers
- [ ] Send status codes (200, 404, 500)
- [ ] Serve static files
- [ ] Set Content-Type headers

**Files to create**:
- `phase2/http_parser.c` - HTTP parsing logic
- `phase2/http_parser.h` - Header file
- `phase2/http_response.c` - Response building
- `phase2/02_http_server.c` - Main server with HTTP support

**Learning**: HTTP protocol, request/response format

---

### Phase 3: Non-Blocking I/O (Weeks 3-4)
**Goal**: Handle multiple connections efficiently

#### 3.1 - Select-based Multiplexing
- [ ] Use `select()` to monitor multiple sockets
- [ ] Set sockets to non-blocking mode
- [ ] Handle partial reads/writes
- [ ] Manage connection state

#### 3.2 - Connection State Machine
- [ ] Track connection states (reading, writing, closing)
- [ ] Buffer incomplete requests
- [ ] Handle timeouts

**Files to create**:
- `phase3/connection.c` - Connection state management
- `phase3/connection.h` - Connection structures
- `phase3/03_select_server.c` - Select-based multiplexing

**Learning**: Non-blocking I/O, multiplexing, state machines

---

### Phase 4: Event-Driven Architecture (Weeks 4-5)
**Goal**: Implement epoll for Linux (or kqueue for BSD)

#### 4.1 - Epoll Implementation
- [ ] Replace select() with epoll()
- [ ] Register/unregister file descriptors
- [ ] Handle EPOLLIN, EPOLLOUT events
- [ ] Implement event loop

#### 4.2 - Event Handler
- [ ] Create event callback system
- [ ] Handle read events
- [ ] Handle write events
- [ ] Handle error events

**Files to create**:
- `phase4/event_loop.c` - Event loop implementation
- `phase4/event_loop.h` - Event loop interface
- `phase4/04_epoll_server.c` - Epoll-based server

**Learning**: epoll/kqueue, event-driven programming, scalability

---

### Phase 5: Worker Processes (Weeks 5-6)
**Goal**: Implement master-worker architecture like NGINX

#### 5.1 - Master Process
- [ ] Create worker processes
- [ ] Monitor worker health
- [ ] Handle graceful shutdown
- [ ] Reload configuration

#### 5.2 - Worker Process
- [ ] Accept connections from shared socket
- [ ] Run event loop
- [ ] Handle signals

**Files to create**:
- `phase5/process_manager.c` - Process management
- `phase5/process_manager.h` - Process structures
- `phase5/05_master_worker_server.c` - Master-worker architecture

**Learning**: Process management, signal handling, IPC

---

### Phase 6: Configuration & Optimization (Weeks 6-7)
**Goal**: Add configuration and performance tuning

#### 6.1 - Configuration System
- [ ] Parse config file
- [ ] Set port, worker count, buffer sizes
- [ ] Logging configuration

#### 6.2 - Performance Optimization
- [ ] Connection pooling
- [ ] Buffer optimization
- [ ] Memory pooling
- [ ] Benchmarking

**Files to create**:
- `phase6/config.c` - Configuration parser
- `phase6/config.h` - Config structures
- `phase6/memory_pool.c` - Memory pooling
- `phase6/06_optimized_server.c` - Optimized server

**Learning**: Configuration management, performance tuning

---

### Phase 7: Advanced Features (Weeks 7-8)
**Goal**: Add production-ready features

#### 7.1 - Reverse Proxy
- [ ] Forward requests to backend servers
- [ ] Handle backend responses
- [ ] Connection pooling to backends

#### 7.2 - Load Balancing
- [ ] Round-robin load balancing
- [ ] Health checks
- [ ] Failover handling

#### 7.3 - Logging & Monitoring
- [ ] Access logs
- [ ] Error logs
- [ ] Performance metrics

**Files to create**:
- `phase7/proxy.c` - Reverse proxy logic
- `phase7/load_balancer.c` - Load balancing
- `phase7/logger.c` - Logging system
- `phase7/07_full_server.c` - Complete server

**Learning**: Proxy patterns, load balancing, logging

---

## 📁 Project Structure

```
web_server/language/C/
├── LEARNING_SCENARIO.md          (this file)
├── SETUP.md                       (environment setup)
├── RESOURCES.md                   (learning resources)
│
├── phase1/                        (Basic Socket)
│   ├── 01_basic_socket.c
│   ├── Makefile
│   └── README.md
│
├── phase2/                        (HTTP Protocol)
│   ├── http_parser.c
│   ├── http_parser.h
│   ├── http_response.c
│   ├── http_response.h
│   ├── 02_http_server.c
│   ├── Makefile
│   └── README.md
│
├── phase3/                        (Non-Blocking I/O)
│   ├── connection.c
│   ├── connection.h
│   ├── 03_select_server.c
│   ├── Makefile
│   └── README.md
│
├── phase4/                        (Event-Driven)
│   ├── event_loop.c
│   ├── event_loop.h
│   ├── 04_epoll_server.c
│   ├── Makefile
│   └── README.md
│
├── phase5/                        (Master-Worker)
│   ├── process_manager.c
│   ├── process_manager.h
│   ├── 05_master_worker_server.c
│   ├── Makefile
│   └── README.md
│
├── phase6/                        (Configuration)
│   ├── config.c
│   ├── config.h
│   ├── memory_pool.c
│   ├── memory_pool.h
│   ├── 06_optimized_server.c
│   ├── Makefile
│   └── README.md
│
├── phase7/                        (Advanced Features)
│   ├── proxy.c
│   ├── proxy.h
│   ├── load_balancer.c
│   ├── load_balancer.h
│   ├── logger.c
│   ├── logger.h
│   ├── 07_full_server.c
│   ├── Makefile
│   └── README.md
│
├── tests/                         (Testing)
│   ├── test_http_parser.c
│   ├── test_event_loop.c
│   └── Makefile
│
├── tools/                         (Utilities)
│   ├── benchmark.sh
│   ├── load_test.sh
│   └── debug.sh
│
└── docs/                          (Documentation)
    ├── architecture.md
    ├── api_reference.md
    └── troubleshooting.md
```

---

## 🎓 Key Concepts to Learn

### 1. Socket Programming
```c
// Create socket
int sock = socket(AF_INET, SOCK_STREAM, 0);

// Bind to port
struct sockaddr_in addr;
bind(sock, (struct sockaddr*)&addr, sizeof(addr));

// Listen for connections
listen(sock, BACKLOG);

// Accept connection
int client = accept(sock, NULL, NULL);
```

### 2. Non-Blocking I/O
```c
// Set non-blocking
fcntl(sock, F_SETFL, O_NONBLOCK);

// Handle EAGAIN/EWOULDBLOCK
if (read(sock, buf, size) == -1 && errno == EAGAIN) {
    // No data available, try again later
}
```

### 3. Event Loop (epoll)
```c
// Create epoll instance
int epfd = epoll_create1(0);

// Add socket to monitor
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = sock;
epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);

// Wait for events
struct epoll_event events[MAX_EVENTS];
int n = epoll_wait(epfd, events, MAX_EVENTS, timeout);
```

### 4. Process Management
```c
// Fork worker process
pid_t pid = fork();
if (pid == 0) {
    // Worker process
    worker_main();
} else {
    // Master process
    master_main();
}
```

---

## 📈 Progression Path

```
Phase 1: Single-threaded → Phase 2: HTTP parsing
    ↓
Phase 3: Non-blocking I/O → Phase 4: Event-driven
    ↓
Phase 5: Master-worker → Phase 6: Configuration
    ↓
Phase 7: Advanced features → Production-ready server
```

---

## 🧪 Testing Strategy

### Unit Tests
- HTTP parser correctness
- Event loop functionality
- Connection state management

### Integration Tests
- Multiple concurrent connections
- Request/response cycle
- Error handling

### Performance Tests
- Throughput (requests/sec)
- Latency (response time)
- Memory usage
- CPU usage

### Load Testing
- Apache Bench (ab)
- wrk
- Custom load test script

---

## 🎯 Milestones

| Phase | Milestone | Connections | Features |
|-------|-----------|-------------|----------|
| 1 | Basic Server | 1 | TCP socket |
| 2 | HTTP Server | 1 | HTTP parsing |
| 3 | Multi-conn | 100s | select() |
| 4 | Event-driven | 1000s | epoll |
| 5 | Master-worker | 1000s | Process mgmt |
| 6 | Optimized | 10000s | Config, pooling |
| 7 | Production | 10000s+ | Proxy, LB, logging |

---

## 💡 Tips for Success

1. **Start Simple**: Don't optimize prematurely. Get it working first.
2. **Test Incrementally**: Test each phase before moving to the next.
3. **Use Debugging Tools**: gdb, strace, valgrind are your friends.
4. **Read NGINX Source**: Study real NGINX code for inspiration.
5. **Benchmark Often**: Measure performance at each phase.
6. **Handle Errors**: Proper error handling is crucial in systems programming.
7. **Memory Safety**: Use valgrind to check for leaks and corruption.

---

## 📚 Recommended Resources

- **Books**:
  - "Unix Network Programming" by Stevens & Rago
  - "The Linux Programming Interface" by Michael Kerrisk
  - NGINX source code (github.com/nginx/nginx)

- **Online**:
  - man pages (man socket, man epoll, man fork)
  - Linux kernel documentation
  - NGINX documentation

- **Tools**:
  - gdb (debugger)
  - valgrind (memory checker)
  - strace (system call tracer)
  - perf (performance profiler)

---

## ✅ Completion Checklist

- [ ] Phase 1: Basic socket server working
- [ ] Phase 2: HTTP parsing and responses
- [ ] Phase 3: Multiple connections with select()
- [ ] Phase 4: Event-driven with epoll
- [ ] Phase 5: Master-worker architecture
- [ ] Phase 6: Configuration and optimization
- [ ] Phase 7: Advanced features (proxy, LB)
- [ ] All tests passing
- [ ] Performance benchmarks documented
- [ ] Code reviewed and documented

---

## 🚀 Next Steps

1. Start with **Phase 1**: Create `phase1/01_basic_socket.c`
2. Follow the README in each phase
3. Test thoroughly before moving to the next phase
4. Document your learning journey
5. Compare your implementation with NGINX source code

Good luck! 🎉
