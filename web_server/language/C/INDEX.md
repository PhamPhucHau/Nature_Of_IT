# 📑 Complete Index - NGINX-like Web Server in C

## 🎯 Start Here

**New to this project?** Start with this order:

1. **[README.md](README.md)** ← Start here! Overview and quick start
2. **[SETUP.md](SETUP.md)** ← Set up your environment
3. **[LEARNING_SCENARIO.md](LEARNING_SCENARIO.md)** ← Understand the full learning path
4. **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** ← Keep this handy while coding
5. **[RESOURCES.md](RESOURCES.md)** ← When you need to learn more

---

## 📚 Documentation Files

### Main Guides
| File | Purpose | Read When |
|------|---------|-----------|
| [README.md](README.md) | Project overview and quick start | First thing |
| [LEARNING_SCENARIO.md](LEARNING_SCENARIO.md) | Complete 7-phase learning path | Planning your journey |
| [SETUP.md](SETUP.md) | Environment setup and tools | Before coding |
| [RESOURCES.md](RESOURCES.md) | Books, tutorials, references | When learning concepts |
| [QUICK_REFERENCE.md](QUICK_REFERENCE.md) | C functions and patterns | While coding |
| [INDEX.md](INDEX.md) | This file - navigation guide | When lost |

---

## 🏗️ Phase Guides

Each phase has its own README with specific instructions:

### Phase 1: Basic Socket Server
- **Duration**: Week 1-2
- **Goal**: Create a simple TCP server
- **Files**: `phase1/README.md`, `phase1/01_basic_socket.c`
- **Key Concepts**: TCP/IP, socket API, basic networking

### Phase 2: HTTP Protocol
- **Duration**: Week 2-3
- **Goal**: Parse HTTP requests and build responses
- **Files**: `phase2/README.md`, `phase2/02_http_server.c`
- **Key Concepts**: HTTP protocol, request/response format

### Phase 3: Non-Blocking I/O
- **Duration**: Week 3-4
- **Goal**: Handle multiple connections with select()
- **Files**: `phase3/README.md`, `phase3/03_select_server.c`
- **Key Concepts**: Non-blocking I/O, multiplexing

### Phase 4: Event-Driven Architecture
- **Duration**: Week 4-5
- **Goal**: Implement epoll for high performance
- **Files**: `phase4/README.md`, `phase4/04_epoll_server.c`
- **Key Concepts**: epoll, event-driven programming

### Phase 5: Master-Worker Pattern
- **Duration**: Week 5-6
- **Goal**: Implement NGINX-like architecture
- **Files**: `phase5/README.md`, `phase5/05_master_worker_server.c`
- **Key Concepts**: Process management, signals

### Phase 6: Configuration & Optimization
- **Duration**: Week 6-7
- **Goal**: Add configuration and performance tuning
- **Files**: `phase6/README.md`, `phase6/06_optimized_server.c`
- **Key Concepts**: Configuration, memory management

### Phase 7: Advanced Features
- **Duration**: Week 7-8
- **Goal**: Add production-ready features
- **Files**: `phase7/README.md`, `phase7/07_full_server.c`
- **Key Concepts**: Proxy, load balancing, logging

---

## 🔍 Quick Navigation

### By Topic

#### Socket Programming
- [QUICK_REFERENCE.md - Socket Creation & Setup](QUICK_REFERENCE.md#socket-creation--setup)
- [QUICK_REFERENCE.md - Socket Options](QUICK_REFERENCE.md#socket-options)
- [QUICK_REFERENCE.md - I/O Operations](QUICK_REFERENCE.md#io-operations)
- [RESOURCES.md - Socket Programming](RESOURCES.md#tutorials--guides)

#### I/O Multiplexing
- [QUICK_REFERENCE.md - select()](QUICK_REFERENCE.md#select)
- [QUICK_REFERENCE.md - epoll()](QUICK_REFERENCE.md#epoll)
- [LEARNING_SCENARIO.md - Phase 3](LEARNING_SCENARIO.md#phase-3-non-blocking-io-weeks-3-4)
- [LEARNING_SCENARIO.md - Phase 4](LEARNING_SCENARIO.md#phase-4-event-driven-architecture-weeks-4-5)

#### HTTP Protocol
- [QUICK_REFERENCE.md - HTTP Protocol](QUICK_REFERENCE.md#-http-protocol-quick-reference)
- [LEARNING_SCENARIO.md - Phase 2](LEARNING_SCENARIO.md#phase-2-http-protocol-weeks-2-3)

#### Process Management
- [QUICK_REFERENCE.md - Process Management](QUICK_REFERENCE.md#process-management)
- [QUICK_REFERENCE.md - Signal Handling](QUICK_REFERENCE.md#signal-handling)
- [LEARNING_SCENARIO.md - Phase 5](LEARNING_SCENARIO.md#phase-5-worker-processes-weeks-5-6)

#### Debugging
- [SETUP.md - Debugging Tools](SETUP.md#debugging-tools)
- [QUICK_REFERENCE.md - Debugging Commands](QUICK_REFERENCE.md#-debugging-commands)

#### Performance Testing
- [SETUP.md - Testing Tools](SETUP.md#testing-tools)
- [QUICK_REFERENCE.md - Performance Testing](QUICK_REFERENCE.md#-performance-testing)

---

## 📖 Learning Resources

### Books
- **Unix Network Programming** (Stevens & Rago) - [RESOURCES.md](RESOURCES.md#1-unix-network-programming-stevens--rago)
- **The Linux Programming Interface** (Kerrisk) - [RESOURCES.md](RESOURCES.md#2-the-linux-programming-interface-michael-kerrisk)
- **Advanced Programming in UNIX** (Stevens & Rago) - [RESOURCES.md](RESOURCES.md#3-advanced-programming-in-the-unix-environment-stevens--rago)

### Online Resources
- **Beej's Guide** - [RESOURCES.md](RESOURCES.md#tutorials--guides)
- **Linux man pages** - [RESOURCES.md](RESOURCES.md#official-documentation)
- **NGINX Source Code** - [RESOURCES.md](RESOURCES.md#-nginx-source-code)

### Tools & Utilities
- **GDB** - [SETUP.md](SETUP.md#gdb-gnu-debugger) | [QUICK_REFERENCE.md](QUICK_REFERENCE.md#gdb)
- **Valgrind** - [SETUP.md](SETUP.md#valgrind-memory-checker) | [QUICK_REFERENCE.md](QUICK_REFERENCE.md#valgrind)
- **strace** - [SETUP.md](SETUP.md#strace-system-call-tracer) | [QUICK_REFERENCE.md](QUICK_REFERENCE.md#strace)
- **Apache Bench** - [SETUP.md](SETUP.md#apache-bench-ab) | [QUICK_REFERENCE.md](QUICK_REFERENCE.md#apache-bench)

---

## 🛠️ Common Tasks

### Setup & Installation
1. Read [SETUP.md](SETUP.md)
2. Install tools: `sudo apt-get install build-essential gdb valgrind`
3. Create directories: `mkdir -p phase{1..7} bin tests tools docs`

### Start a New Phase
1. Read the phase README (e.g., `phase1/README.md`)
2. Review [QUICK_REFERENCE.md](QUICK_REFERENCE.md) for relevant functions
3. Create the main file (e.g., `phase1/01_basic_socket.c`)
4. Create a Makefile
5. Compile and test

### Debug a Problem
1. Use [QUICK_REFERENCE.md](QUICK_REFERENCE.md) to find relevant functions
2. Use gdb: `gdb ./bin/server`
3. Use valgrind: `valgrind --leak-check=full ./bin/server`
4. Use strace: `strace ./bin/server`
5. Check [SETUP.md - Common Issues](SETUP.md#common-issues--solutions)

### Test Performance
1. Start server: `./bin/server`
2. In another terminal, use ab: `ab -n 1000 -c 10 http://localhost:8080/`
3. Or use wrk: `wrk -t4 -c100 -d30s http://localhost:8080/`
4. Compare results across phases

### Learn a Concept
1. Check [QUICK_REFERENCE.md](QUICK_REFERENCE.md) for quick overview
2. Read relevant section in [RESOURCES.md](RESOURCES.md)
3. Study NGINX source code
4. Implement in your code

---

## 📊 Project Structure

```
web_server/language/C/
├── README.md                      ← Start here
├── INDEX.md                       ← This file
├── LEARNING_SCENARIO.md           ← Full learning path
├── SETUP.md                       ← Environment setup
├── RESOURCES.md                   ← Learning resources
├── QUICK_REFERENCE.md             ← C functions & patterns
│
├── phase1/                        ← Basic Socket
│   ├── README.md
│   ├── 01_basic_socket.c
│   └── Makefile
│
├── phase2/                        ← HTTP Protocol
│   ├── README.md
│   ├── http_parser.c
│   ├── http_parser.h
│   ├── http_response.c
│   ├── http_response.h
│   ├── 02_http_server.c
│   └── Makefile
│
├── phase3/                        ← Non-Blocking I/O
│   ├── README.md
│   ├── connection.c
│   ├── connection.h
│   ├── 03_select_server.c
│   └── Makefile
│
├── phase4/                        ← Event-Driven
│   ├── README.md
│   ├── event_loop.c
│   ├── event_loop.h
│   ├── 04_epoll_server.c
│   └── Makefile
│
├── phase5/                        ← Master-Worker
│   ├── README.md
│   ├── process_manager.c
│   ├── process_manager.h
│   ├── 05_master_worker_server.c
│   └── Makefile
│
├── phase6/                        ← Configuration
│   ├── README.md
│   ├── config.c
│   ├── config.h
│   ├── memory_pool.c
│   ├── memory_pool.h
│   ├── 06_optimized_server.c
│   └── Makefile
│
├── phase7/                        ← Advanced Features
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
├── tests/                         ← Testing
│   ├── test_http_parser.c
│   ├── test_event_loop.c
│   └── Makefile
│
├── tools/                         ← Utilities
│   ├── benchmark.sh
│   ├── load_test.sh
│   └── debug.sh
│
└── docs/                          ← Documentation
    ├── architecture.md
    ├── api_reference.md
    └── troubleshooting.md
```

---

## 🎯 Learning Paths

### Path 1: Complete Beginner
**Time**: 8 weeks, 10-15 hours/week

1. Week 1-2: Phase 1 (Basic Socket)
2. Week 2-3: Phase 2 (HTTP Protocol)
3. Week 3-4: Phase 3 (Non-Blocking I/O)
4. Week 4-5: Phase 4 (Event-Driven)
5. Week 5-6: Phase 5 (Master-Worker)
6. Week 6-7: Phase 6 (Configuration)
7. Week 7-8: Phase 7 (Advanced Features)

### Path 2: Experienced Programmer
**Time**: 4 weeks, 15-20 hours/week

1. Week 1: Phase 1-2 (Socket + HTTP)
2. Week 2: Phase 3-4 (I/O + Event-Driven)
3. Week 3: Phase 5-6 (Master-Worker + Config)
4. Week 4: Phase 7 (Advanced Features)

### Path 3: Systems Programming Expert
**Time**: 2 weeks, 20+ hours/week

1. Week 1: Phase 1-4 (Fundamentals)
2. Week 2: Phase 5-7 (Advanced)

---

## ✅ Completion Checklist

### Setup
- [ ] Tools installed (gcc, make, gdb, valgrind)
- [ ] Project structure created
- [ ] IDE configured (VS Code, CLion, etc.)

### Phase 1
- [ ] Basic socket server compiles
- [ ] Server accepts connections
- [ ] Server sends HTTP response
- [ ] Tested with curl

### Phase 2
- [ ] HTTP parser works
- [ ] HTTP response builder works
- [ ] Static files served
- [ ] Multiple content types supported

### Phase 3
- [ ] select() multiplexing works
- [ ] Non-blocking I/O implemented
- [ ] Multiple connections handled
- [ ] Benchmarked performance

### Phase 4
- [ ] epoll() implemented
- [ ] Event loop works
- [ ] Thousands of connections handled
- [ ] Performance improved vs Phase 3

### Phase 5
- [ ] Master process works
- [ ] Worker processes created
- [ ] Signals handled properly
- [ ] Graceful shutdown works

### Phase 6
- [ ] Configuration file parsing works
- [ ] Memory pooling implemented
- [ ] Performance optimized
- [ ] Benchmarks documented

### Phase 7
- [ ] Reverse proxy works
- [ ] Load balancing implemented
- [ ] Logging system works
- [ ] Performance metrics collected

---

## 🆘 Troubleshooting

### Common Issues
- **"Address already in use"** → [SETUP.md](SETUP.md#issue-address-already-in-use)
- **"Permission denied"** → [SETUP.md](SETUP.md#issue-permission-denied-on-port--1024)
- **"Segmentation fault"** → [SETUP.md](SETUP.md#issue-segmentation-fault)
- **"Memory leak detected"** → [SETUP.md](SETUP.md#issue-memory-leak-detected)

### Debugging Help
- [SETUP.md - Debugging Tools](SETUP.md#debugging-tools)
- [QUICK_REFERENCE.md - Debugging Commands](QUICK_REFERENCE.md#-debugging-commands)
- [RESOURCES.md - Communities](RESOURCES.md#communities)

---

## 🔗 External Resources

### Official Documentation
- [Linux man pages](https://man7.org/)
- [POSIX Standard](https://pubs.opengroup.org/)
- [GCC Documentation](https://gcc.gnu.org/onlinedocs/)

### Tutorials
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [GeeksforGeeks - Socket Programming](https://www.geeksforgeeks.org/socket-programming-cc/)
- [Linux Socket Programming](https://www.geeksforgeeks.org/socket-programming-cc/)

### Source Code
- [NGINX GitHub](https://github.com/nginx/nginx)
- [Linux Kernel](https://github.com/torvalds/linux)

### Tools
- [GDB](https://www.gnu.org/software/gdb/)
- [Valgrind](https://valgrind.org/)
- [strace](https://strace.io/)
- [perf](https://perf.wiki.kernel.org/)

---

## 📞 Getting Help

### If You're Stuck
1. **Check the phase README** - Most answers are there
2. **Review QUICK_REFERENCE.md** - Find the function you need
3. **Search RESOURCES.md** - Learn the concept
4. **Debug with tools** - Use gdb, valgrind, strace
5. **Study NGINX source** - See how professionals do it

### Common Questions
- **"How do I compile?"** → [SETUP.md](SETUP.md#compilation-flags-explained)
- **"How do I debug?"** → [QUICK_REFERENCE.md](QUICK_REFERENCE.md#-debugging-commands)
- **"How do I test?"** → [QUICK_REFERENCE.md](QUICK_REFERENCE.md#-performance-testing)
- **"What's the next step?"** → [README.md](README.md#-next-steps)

---

## 🎉 You're Ready!

Now that you understand the structure, here's what to do:

1. **Read [README.md](README.md)** - Get the overview
2. **Follow [SETUP.md](SETUP.md)** - Set up your environment
3. **Study [LEARNING_SCENARIO.md](LEARNING_SCENARIO.md)** - Understand the path
4. **Keep [QUICK_REFERENCE.md](QUICK_REFERENCE.md) handy** - Reference while coding
5. **Start Phase 1** - Create your first socket server

Good luck! 🚀

---

## 📝 Document Versions

- **Created**: May 14, 2026
- **Last Updated**: May 14, 2026
- **Status**: Ready for learning

---

## 🙏 Acknowledgments

This learning scenario is inspired by:
- NGINX architecture and design
- Unix Network Programming (Stevens & Rago)
- The Linux Programming Interface (Kerrisk)
- Real-world systems programming practices

---

**Happy Learning! 📚**
