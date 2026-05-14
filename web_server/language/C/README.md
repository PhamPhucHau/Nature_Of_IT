# 🚀 Building an NGINX-like Web Server in C

Welcome! This is a comprehensive learning scenario for building a high-performance web server in C, inspired by NGINX architecture.

---

## 📖 What You'll Learn

By completing this project, you'll understand:

- **Socket Programming**: TCP/IP, non-blocking I/O, connection handling
- **Event-Driven Architecture**: epoll/select, event loops, scalability
- **HTTP Protocol**: Request parsing, response building, headers
- **Process Management**: Master-worker pattern, signal handling, IPC
- **Performance Optimization**: Memory pooling, connection pooling, benchmarking
- **Advanced Features**: Reverse proxy, load balancing, logging

---

## 🎯 Quick Start

### 1. Setup Your Environment
```bash
# Read setup guide
cat SETUP.md

# Install required tools (Ubuntu/Debian)
sudo apt-get install build-essential gdb valgrind

# Create project structure
mkdir -p phase{1..7} bin tests tools docs
```

### 2. Choose Your Learning Path
- **Beginner**: Start with Phase 1 (Basic Socket)
- **Intermediate**: Start with Phase 3 (Non-Blocking I/O)
- **Advanced**: Start with Phase 5 (Master-Worker)

### 3. Read the Learning Scenario
```bash
cat LEARNING_SCENARIO.md
```

### 4. Start Coding
```bash
# Phase 1: Basic Socket Server
cd phase1
cat README.md
vim 01_basic_socket.c
make
./bin/phase1_server
```

---

## 📚 Documentation

### Essential Guides
- **[LEARNING_SCENARIO.md](LEARNING_SCENARIO.md)** - Complete 7-phase learning path
- **[SETUP.md](SETUP.md)** - Environment setup and tools
- **[RESOURCES.md](RESOURCES.md)** - Books, tutorials, and references
- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - C functions and patterns

### Phase-Specific Guides
Each phase has its own README:
- `phase1/README.md` - Basic Socket Server
- `phase2/README.md` - HTTP Protocol
- `phase3/README.md` - Non-Blocking I/O
- `phase4/README.md` - Event-Driven Architecture
- `phase5/README.md` - Master-Worker Pattern
- `phase6/README.md` - Configuration & Optimization
- `phase7/README.md` - Advanced Features

---

## 🏗️ Project Structure

```
web_server/language/C/
├── README.md                      (this file)
├── LEARNING_SCENARIO.md           (7-phase learning path)
├── SETUP.md                       (environment setup)
├── RESOURCES.md                   (learning resources)
├── QUICK_REFERENCE.md             (C functions & patterns)
│
├── phase1/                        (Basic Socket)
│   ├── README.md
│   ├── 01_basic_socket.c
│   └── Makefile
│
├── phase2/                        (HTTP Protocol)
│   ├── README.md
│   ├── http_parser.c
│   ├── http_parser.h
│   ├── http_response.c
│   ├── http_response.h
│   ├── 02_http_server.c
│   └── Makefile
│
├── phase3/                        (Non-Blocking I/O)
│   ├── README.md
│   ├── connection.c
│   ├── connection.h
│   ├── 03_select_server.c
│   └── Makefile
│
├── phase4/                        (Event-Driven)
│   ├── README.md
│   ├── event_loop.c
│   ├── event_loop.h
│   ├── 04_epoll_server.c
│   └── Makefile
│
├── phase5/                        (Master-Worker)
│   ├── README.md
│   ├── process_manager.c
│   ├── process_manager.h
│   ├── 05_master_worker_server.c
│   └── Makefile
│
├── phase6/                        (Configuration)
│   ├── README.md
│   ├── config.c
│   ├── config.h
│   ├── memory_pool.c
│   ├── memory_pool.h
│   ├── 06_optimized_server.c
│   └── Makefile
│
├── phase7/                        (Advanced Features)
│   ├── README.md
│   ├── proxy.c
│   ├── proxy.h
│   ├── load_balancer.c
│   ├── load_balancer.h
│   ├── logger.c
│   ├── logger.h
│   ├── 07_full_server.c
│   └── Makefile
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

## 🎓 Learning Phases

### Phase 1: Basic Socket Server (Week 1-2)
**Goal**: Create a simple TCP server that accepts connections

- Create socket, bind, listen, accept
- Send HTTP response
- Handle single connection

**Key Concepts**: TCP/IP, socket API, basic networking

---

### Phase 2: HTTP Protocol (Week 2-3)
**Goal**: Parse HTTP requests and build proper responses

- Parse HTTP request line and headers
- Build HTTP response with status codes
- Serve static files
- Handle different content types

**Key Concepts**: HTTP protocol, request/response format, headers

---

### Phase 3: Non-Blocking I/O (Week 3-4)
**Goal**: Handle multiple connections efficiently with select()

- Set sockets to non-blocking mode
- Use select() for multiplexing
- Manage connection state
- Handle partial reads/writes

**Key Concepts**: Non-blocking I/O, multiplexing, state machines

---

### Phase 4: Event-Driven Architecture (Week 4-5)
**Goal**: Implement epoll for high-performance event handling

- Replace select() with epoll()
- Implement event loop
- Handle EPOLLIN, EPOLLOUT events
- Scale to thousands of connections

**Key Concepts**: epoll, event-driven programming, scalability

---

### Phase 5: Master-Worker Pattern (Week 5-6)
**Goal**: Implement NGINX-like master-worker architecture

- Create master process
- Fork worker processes
- Handle signals (SIGTERM, SIGHUP)
- Graceful shutdown and reload

**Key Concepts**: Process management, signals, IPC

---

### Phase 6: Configuration & Optimization (Week 6-7)
**Goal**: Add configuration and performance tuning

- Parse configuration file
- Implement memory pooling
- Optimize buffer management
- Benchmark performance

**Key Concepts**: Configuration, memory management, optimization

---

### Phase 7: Advanced Features (Week 7-8)
**Goal**: Add production-ready features

- Implement reverse proxy
- Add load balancing
- Implement logging system
- Add performance metrics

**Key Concepts**: Proxy patterns, load balancing, logging

---

## 🚀 Getting Started

### Step 1: Read the Overview
Start with the parent directory's `Overview.md` to understand NGINX basics.

### Step 2: Setup Environment
Follow `SETUP.md` to install required tools and create project structure.

### Step 3: Choose Your Starting Point

**If you're new to C networking:**
1. Read `RESOURCES.md` - Learning Resources section
2. Study `QUICK_REFERENCE.md` - Essential functions
3. Start with Phase 1

**If you have some C experience:**
1. Skim `QUICK_REFERENCE.md`
2. Start with Phase 2 or 3

**If you're experienced:**
1. Review `LEARNING_SCENARIO.md`
2. Start with Phase 4 or 5

### Step 4: Start Coding
```bash
# Phase 1 example
cd phase1
cat README.md
vim 01_basic_socket.c
make
./bin/phase1_server

# In another terminal
curl http://localhost:8080/
```

### Step 5: Test and Debug
```bash
# Test with curl
curl -v http://localhost:8080/

# Benchmark with ab
ab -n 100 -c 10 http://localhost:8080/

# Debug with gdb
gdb ./bin/phase1_server

# Check memory with valgrind
valgrind --leak-check=full ./bin/phase1_server
```

---

## 💡 Tips for Success

### 1. Start Simple
Don't optimize prematurely. Get it working first, then optimize.

### 2. Test Incrementally
Test each phase before moving to the next. Don't skip testing.

### 3. Use Debugging Tools
- **gdb**: Debug crashes and logic errors
- **valgrind**: Find memory leaks
- **strace**: Understand system calls

### 4. Read NGINX Source
Study real NGINX code to see how professionals do it.

### 5. Benchmark Often
Measure performance at each phase to understand improvements.

### 6. Handle Errors Properly
Proper error handling is crucial in systems programming.

### 7. Document Your Learning
Write comments explaining what you learned in each phase.

---

## 🧪 Testing Strategy

### Unit Tests
- Test individual functions (HTTP parser, event loop)
- Use simple test cases
- Verify correctness

### Integration Tests
- Test multiple components together
- Test request/response cycle
- Test error handling

### Performance Tests
- Measure throughput (requests/sec)
- Measure latency (response time)
- Measure resource usage (memory, CPU)

### Load Tests
- Use Apache Bench (ab)
- Use wrk for modern load testing
- Test with increasing load

---

## 📊 Expected Milestones

| Phase | Connections | Features | Performance |
|-------|-------------|----------|-------------|
| 1 | 1 | TCP socket | N/A |
| 2 | 1 | HTTP parsing | N/A |
| 3 | 100s | select() | ~1000 req/s |
| 4 | 1000s | epoll | ~10000 req/s |
| 5 | 1000s | Master-worker | ~10000 req/s |
| 6 | 10000s | Config, pooling | ~50000 req/s |
| 7 | 10000s+ | Proxy, LB | ~50000+ req/s |

---

## 🔗 Quick Links

### Documentation
- [LEARNING_SCENARIO.md](LEARNING_SCENARIO.md) - Complete learning path
- [SETUP.md](SETUP.md) - Environment setup
- [RESOURCES.md](RESOURCES.md) - Learning resources
- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - C functions

### External Resources
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [Linux man pages](https://man7.org/)
- [NGINX Source Code](https://github.com/nginx/nginx)
- [POSIX Standard](https://pubs.opengroup.org/)

### Tools
- [GDB Debugger](https://www.gnu.org/software/gdb/)
- [Valgrind](https://valgrind.org/)
- [strace](https://strace.io/)
- [Apache Bench](https://httpd.apache.org/docs/2.4/programs/ab.html)

---

## ❓ FAQ

### Q: Do I need to know C well?
**A**: Basic C knowledge is helpful, but you'll learn as you go. Start with Phase 1 and take it slow.

### Q: How long will this take?
**A**: 8 weeks if you dedicate 10-15 hours per week. You can go faster or slower depending on your pace.

### Q: Can I skip phases?
**A**: Not recommended. Each phase builds on the previous one. However, if you have experience, you can start from Phase 3 or 4.

### Q: What if I get stuck?
**A**: 
1. Read the phase README
2. Check QUICK_REFERENCE.md
3. Look at RESOURCES.md for tutorials
4. Use debugging tools (gdb, strace)
5. Study NGINX source code

### Q: Can I use this for production?
**A**: No, this is for learning. Real NGINX is much more complex and battle-tested. Use this to understand concepts.

### Q: What about Windows?
**A**: Use WSL2 (Windows Subsystem for Linux) or MinGW. The code is POSIX-compliant and should work on any Unix-like system.

---

## 🎯 Next Steps

1. **Read SETUP.md** - Set up your environment
2. **Read LEARNING_SCENARIO.md** - Understand the full path
3. **Read QUICK_REFERENCE.md** - Learn essential functions
4. **Start Phase 1** - Create your first socket server
5. **Test and debug** - Use the tools
6. **Move to Phase 2** - Add HTTP support
7. **Continue** - Follow the learning path

---

## 📝 Project Checklist

- [ ] Environment setup complete
- [ ] Phase 1: Basic socket server working
- [ ] Phase 2: HTTP parsing and responses
- [ ] Phase 3: Multiple connections with select()
- [ ] Phase 4: Event-driven with epoll
- [ ] Phase 5: Master-worker architecture
- [ ] Phase 6: Configuration and optimization
- [ ] Phase 7: Advanced features
- [ ] All tests passing
- [ ] Performance benchmarks documented
- [ ] Code reviewed and documented

---

## 🤝 Contributing

Found an issue or have suggestions? Feel free to improve this learning scenario!

---

## 📄 License

This learning scenario is provided as-is for educational purposes.

---

## 🎉 Good Luck!

You're about to embark on an exciting journey into systems programming. This is challenging but rewarding work. Take your time, test thoroughly, and don't hesitate to debug.

Remember: **The best way to learn is by doing.** So start coding! 🚀

---

## 📞 Support

If you have questions:
1. Check the phase README
2. Review QUICK_REFERENCE.md
3. Search RESOURCES.md
4. Debug with gdb/valgrind
5. Study NGINX source code

Happy learning! 📚
