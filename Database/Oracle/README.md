# 🗄️ Mini Oracle Database System in C++

A comprehensive learning project for building a database management system (DBMS) similar to Oracle, from scratch in C++.

---

## 📖 Project Overview

This project guides you through building a complete database system with:

- **Storage Engine** - Pages, serialization, file I/O
- **B+ Tree Indexing** - Fast data retrieval
- **SQL Parser** - Parse and execute SQL commands
- **Query Executor** - Execute parsed queries
- **Buffer Pool** - Cache management with LRU eviction
- **Transaction System** - ACID properties, WAL, rollback
- **Concurrency Control** - Locks, MVCC, deadlock detection
- **Client-Server** - TCP networking, binary protocol

---

## 🎯 Learning Objectives

By completing this project, you will understand:

- ✅ How databases store data on disk
- ✅ Why indexes speed up queries
- ✅ How B+ trees work internally
- ✅ How SQL is parsed and executed
- ✅ How buffer pools manage memory
- ✅ How transactions maintain ACID properties
- ✅ How concurrency is controlled
- ✅ How crash recovery works
- ✅ How client-server databases work
- ✅ How systems like Oracle, PostgreSQL, MySQL operate

---

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────┐
│         Client Application              │
└────────────────┬────────────────────────┘
                 │
         ┌───────▼────────┐
         │  SQL Parser    │
         │  (Lexer/AST)   │
         └───────┬────────┘
                 │
         ┌───────▼────────────┐
         │  Query Planner     │
         │  & Optimizer       │
         └───────┬────────────┘
                 │
         ┌───────▼────────────┐
         │ Execution Engine   │
         │ (Executor)         │
         └───────┬────────────┘
                 │
         ┌───────▼────────────┐
         │  Storage Engine    │
         │  (Pages, B+ Tree)  │
         └───────┬────────────┘
                 │
         ┌───────▼────────────┐
         │  Buffer Pool       │
         │  (Cache Mgmt)      │
         └───────┬────────────┘
                 │
         ┌───────▼────────────┐
         │  Transaction Log   │
         │  (WAL)             │
         └───────┬────────────┘
                 │
         ┌───────▼────────────┐
         │  Disk Storage      │
         │  (Files)           │
         └────────────────────┘
```

---

## 📊 The 7 Learning Phases

### Phase 1: Key-Value Store (Week 1-2)
**Goal**: Build a simple key-value store with file persistence

- Create basic key-value data structure
- Implement serialization
- File I/O operations
- Simple put/get operations

**Learn**: File I/O, binary formats, data serialization

---

### Phase 2: B+ Tree Index (Week 2-3)
**Goal**: Implement B+ tree for efficient data retrieval

- B+ tree node structure
- Insert and search operations
- Page splitting and merging
- Index management

**Learn**: Tree structures, page management, indexing

---

### Phase 3: SQL Parser (Week 3-4)
**Goal**: Parse SQL commands into executable form

- Lexer (tokenization)
- Parser (syntax analysis)
- AST (Abstract Syntax Tree)
- Support: CREATE TABLE, INSERT, SELECT, DELETE

**Learn**: Compiler design, parsing, AST construction

---

### Phase 4: Buffer Pool (Week 4-5)
**Goal**: Implement cache management for pages

- Page cache
- LRU eviction policy
- Dirty page tracking
- Page replacement strategies

**Learn**: Cache management, memory optimization, eviction policies

---

### Phase 5: Transactions (Week 5-6)
**Goal**: Add transaction support with ACID properties

- Write-Ahead Logging (WAL)
- Transaction commit/rollback
- Crash recovery
- Checkpoints

**Learn**: Transaction processing, logging, recovery

---

### Phase 6: Concurrency Control (Week 6-7)
**Goal**: Handle multiple concurrent transactions

- Lock manager
- MVCC (Multi-Version Concurrency Control)
- Deadlock detection
- Isolation levels

**Learn**: Concurrency control, locking, MVCC

---

### Phase 7: Client-Server (Week 7-8)
**Goal**: Build networked database server

- TCP server
- Binary protocol
- Connection pooling
- Client library

**Learn**: Networking, protocols, distributed systems

---

## 📁 Project Structure

```
Database/Oracle/C_Plus_Plus/
├── START_HERE.md              ← Quick start guide
├── README.md                  ← This file
├── SETUP.md                   ← Environment setup
├── LEARNING_SCENARIO.md       ← Full learning path
├── QUICK_REFERENCE.md         ← C++ patterns
├── RESOURCES.md               ← Learning resources
├── ROADMAP.md                 ← Visual roadmap
├── INDEX.md                   ← Navigation
│
├── phase1/                    ← Key-Value Store
│   ├── README.md
│   ├── src/
│   │   ├── storage.cpp
│   │   ├── serializer.cpp
│   │   └── main.cpp
│   ├── include/
│   │   ├── storage.h
│   │   └── serializer.h
│   ├── tests/
│   │   └── test_storage.cpp
│   └── CMakeLists.txt
│
├── phase2/                    ← B+ Tree Index
│   ├── README.md
│   ├── src/
│   │   ├── btree.cpp
│   │   ├── page.cpp
│   │   └── main.cpp
│   ├── include/
│   │   ├── btree.h
│   │   └── page.h
│   ├── tests/
│   │   └── test_btree.cpp
│   └── CMakeLists.txt
│
├── phase3/                    ← SQL Parser
│   ├── README.md
│   ├── src/
│   │   ├── lexer.cpp
│   │   ├── parser.cpp
│   │   ├── ast.cpp
│   │   └── main.cpp
│   ├── include/
│   │   ├── lexer.h
│   │   ├── parser.h
│   │   └── ast.h
│   ├── tests/
│   │   └── test_parser.cpp
│   └── CMakeLists.txt
│
├── phase4/                    ← Buffer Pool
│   ├── README.md
│   ├── src/
│   │   ├── buffer_pool.cpp
│   │   ├── lru_cache.cpp
│   │   └── main.cpp
│   ├── include/
│   │   ├── buffer_pool.h
│   │   └── lru_cache.h
│   ├── tests/
│   │   └── test_buffer_pool.cpp
│   └── CMakeLists.txt
│
├── phase5/                    ← Transactions
│   ├── README.md
│   ├── src/
│   │   ├── transaction.cpp
│   │   ├── wal.cpp
│   │   ├── recovery.cpp
│   │   └── main.cpp
│   ├── include/
│   │   ├── transaction.h
│   │   ├── wal.h
│   │   └── recovery.h
│   ├── tests/
│   │   └── test_transaction.cpp
│   └── CMakeLists.txt
│
├── phase6/                    ← Concurrency
│   ├── README.md
│   ├── src/
│   │   ├── lock_manager.cpp
│   │   ├── mvcc.cpp
│   │   ├── deadlock.cpp
│   │   └── main.cpp
│   ├── include/
│   │   ├── lock_manager.h
│   │   ├── mvcc.h
│   │   └── deadlock.h
│   ├── tests/
│   │   └── test_concurrency.cpp
│   └── CMakeLists.txt
│
├── phase7/                    ← Client-Server
│   ├── README.md
│   ├── src/
│   │   ├── server.cpp
│   │   ├── client.cpp
│   │   ├── protocol.cpp
│   │   └── main.cpp
│   ├── include/
│   │   ├── server.h
│   │   ├── client.h
│   │   └── protocol.h
│   ├── tests/
│   │   └── test_server.cpp
│   └── CMakeLists.txt
│
├── tests/                     ← Integration tests
│   ├── integration_test.cpp
│   └── CMakeLists.txt
│
├── tools/                     ← Utilities
│   ├── benchmark.cpp
│   ├── profiler.cpp
│   └── CMakeLists.txt
│
├── docs/                      ← Documentation
│   ├── architecture.md
│   ├── api_reference.md
│   └── troubleshooting.md
│
├── CMakeLists.txt             ← Root build file
└── build/                     ← Build output
```

---

## 🚀 Quick Start

### 1. Setup Environment
```bash
# Follow SETUP.md
cat SETUP.md
```

### 2. Create Project Structure
```bash
mkdir -p Database/Oracle/C_Plus_Plus/{phase1..phase7}/{src,include,tests}
mkdir -p Database/Oracle/C_Plus_Plus/{tests,tools,docs,build}
```

### 3. Start Phase 1
```bash
cd Database/Oracle/C_Plus_Plus/phase1
cat README.md
# Start implementing key-value store
```

### 4. Build and Test
```bash
cd Database/Oracle/C_Plus_Plus/build
cmake ..
make
./phase1_test
```

---

## 🎓 What You'll Learn

### Database Concepts
- ✅ Storage engines
- ✅ Indexing strategies
- ✅ Query processing
- ✅ Transaction management
- ✅ Concurrency control
- ✅ Crash recovery
- ✅ Buffer management
- ✅ Query optimization

### C++ Skills
- ✅ Advanced OOP
- ✅ Memory management
- ✅ Template programming
- ✅ File I/O
- ✅ Networking
- ✅ Multithreading
- ✅ Performance optimization

### Systems Programming
- ✅ File systems
- ✅ Memory management
- ✅ Concurrency primitives
- ✅ Debugging techniques
- ✅ Performance profiling

---

## 📚 Key Concepts

### B+ Trees
The heart of database indexing. Enables efficient search, insert, and delete operations.

### Pages
Fixed-size blocks of data (typically 4KB). Databases organize data into pages for efficient I/O.

### Buffer Pool
Cache that keeps frequently accessed pages in memory to reduce disk I/O.

### WAL (Write-Ahead Logging)
Ensures durability by writing changes to a log before applying them to the database.

### MVCC (Multi-Version Concurrency Control)
Allows multiple transactions to read and write concurrently without blocking.

### ACID Properties
- **Atomicity**: All or nothing
- **Consistency**: Valid state transitions
- **Isolation**: Concurrent transactions don't interfere
- **Durability**: Committed data survives crashes

---

## 🧪 Testing Strategy

### Unit Tests
- Test individual components (B+ tree, buffer pool, etc.)
- Use Google Test or Catch2
- Verify correctness

### Integration Tests
- Test multiple components together
- Test end-to-end workflows
- Verify interactions

### Performance Tests
- Benchmark throughput
- Measure latency
- Profile memory usage

### Stress Tests
- Concurrent operations
- Large datasets
- Recovery scenarios

---

## 📈 Expected Milestones

| Phase | Milestone | Features | Performance |
|-------|-----------|----------|-------------|
| 1 | Key-Value Store | Put/Get | ~1000 ops/s |
| 2 | B+ Tree | Insert/Search | ~10000 ops/s |
| 3 | SQL Parser | Parse SQL | N/A |
| 4 | Buffer Pool | Cache Mgmt | ~50000 ops/s |
| 5 | Transactions | ACID | ~10000 txn/s |
| 6 | Concurrency | MVCC | ~50000 ops/s |
| 7 | Client-Server | Networking | ~10000 req/s |

---

## 💡 Tips for Success

1. **Start Simple** - Don't optimize prematurely
2. **Test Incrementally** - Test after each small change
3. **Use Debugging Tools** - gdb and valgrind are essential
4. **Study Real Databases** - Read SQLite/PostgreSQL source
5. **Benchmark Often** - Measure performance at each phase
6. **Document Your Learning** - Write comments explaining concepts
7. **Take Breaks** - Database systems are complex

---

## 🔗 Quick Links

### Documentation
- [START_HERE.md](START_HERE.md) - Quick start
- [SETUP.md](SETUP.md) - Environment setup
- [LEARNING_SCENARIO.md](LEARNING_SCENARIO.md) - Full path
- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - C++ patterns
- [RESOURCES.md](RESOURCES.md) - Learning resources
- [ROADMAP.md](ROADMAP.md) - Visual roadmap
- [INDEX.md](INDEX.md) - Navigation

### External Resources
- [SQLite Source](https://www.sqlite.org/)
- [PostgreSQL Source](https://www.postgresql.org/)
- [CMU Database Course](https://15445.courses.cs.cmu.edu/)
- [Build Your Own Database](https://build-your-own.org/database/)

---

## ❓ FAQ

### Q: Do I need to know C++ well?
**A**: Intermediate C++ knowledge is helpful. You'll learn advanced concepts as you go.

### Q: How long will this take?
**A**: 8 weeks if you dedicate 12-20 hours per week. You can go faster or slower.

### Q: Can I skip phases?
**A**: Not recommended. Each phase builds on the previous one.

### Q: What if I get stuck?
**A**: 
1. Read the phase README
2. Check QUICK_REFERENCE.md
3. Look at RESOURCES.md for tutorials
4. Use debugging tools (gdb, valgrind)
5. Study SQLite/PostgreSQL source

### Q: Can I use this for production?
**A**: No, this is for learning. Real databases are much more complex and battle-tested.

### Q: What about Windows?
**A**: Use WSL2 or MinGW. The code is portable and should work on any platform.

---

## 🎉 Next Steps

1. **Read START_HERE.md** - Get oriented
2. **Follow SETUP.md** - Set up environment
3. **Read LEARNING_SCENARIO.md** - Understand the path
4. **Start Phase 1** - Create key-value store
5. **Keep QUICK_REFERENCE.md handy** - Reference while coding
6. **Progress through phases** - Build incrementally
7. **Celebrate your success** - You're learning systems programming!

---

## 📝 Project Info

- **Language**: C++17 or later
- **Build System**: CMake
- **Testing**: Google Test / Catch2
- **Duration**: 8 weeks (100-150 hours)
- **Difficulty**: Advanced
- **Prerequisites**: Intermediate C++, basic database knowledge

---

## 🤝 Contributing

Found an issue or have suggestions? Feel free to improve this learning scenario!

---

## 📄 License

This learning scenario is provided as-is for educational purposes.

---

## 🎓 Learning Philosophy

This project follows these principles:

- **Learn by Doing** - Write code, not just read
- **Build Incrementally** - Start simple, add complexity
- **Understand Concepts** - Know the "why", not just the "how"
- **Test Thoroughly** - Verify each phase works
- **Study Real Code** - Learn from SQLite/PostgreSQL
- **Debug Effectively** - Use proper tools
- **Optimize Gradually** - Get it working first

---

**Happy Learning! 📚**

*Now go build something amazing!* 🎉
