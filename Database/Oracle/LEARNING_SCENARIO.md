# 🗄️ Complete Learning Scenario: Mini Oracle Database in C++

A comprehensive 7-phase learning path for building a database management system from scratch.

---

## 📚 Overview

This scenario guides you through building a complete database system with:

- **Phase 1**: Key-Value Store (File I/O, Serialization)
- **Phase 2**: B+ Tree Index (Indexing, Page Management)
- **Phase 3**: SQL Parser (Lexer, Parser, AST)
- **Phase 4**: Buffer Pool (Cache Management, LRU)
- **Phase 5**: Transactions (WAL, ACID, Recovery)
- **Phase 6**: Concurrency (Locks, MVCC, Deadlock)
- **Phase 7**: Client-Server (Networking, Protocol)

---

## 🎯 Learning Objectives

By completing this scenario, you will understand:

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

## 📊 Project Phases

### Phase 1: Key-Value Store (Week 1-2)

**Goal**: Build a simple key-value store with file persistence

#### What You'll Build
- Basic key-value data structure
- Serialization/deserialization
- File I/O operations
- Simple put/get operations

#### Key Concepts
- Binary file formats
- Data serialization
- File I/O in C++
- Memory management

#### Files to Create
```
phase1/
├── include/
│   ├── storage.h
│   └── serializer.h
├── src/
│   ├── storage.cpp
│   ├── serializer.cpp
│   └── main.cpp
├── tests/
│   └── test_storage.cpp
└── CMakeLists.txt
```

#### Implementation Tasks
- [ ] Create Storage class with put/get methods
- [ ] Implement Serializer for binary format
- [ ] Write data to disk
- [ ] Read data from disk
- [ ] Handle errors gracefully
- [ ] Write unit tests

#### Learning Resources
- File I/O in C++
- Binary formats
- Serialization patterns
- Memory management

#### Success Criteria
- [ ] Compiles without errors
- [ ] put/get operations work
- [ ] Data persists to disk
- [ ] Data can be read back
- [ ] All tests pass

---

### Phase 2: B+ Tree Index (Week 2-3)

**Goal**: Implement B+ tree for efficient data retrieval

#### What You'll Build
- B+ tree node structure
- Insert and search operations
- Page splitting and merging
- Index management

#### Key Concepts
- Tree data structures
- B+ tree properties
- Page management
- Node splitting
- Search algorithms

#### Files to Create
```
phase2/
├── include/
│   ├── btree.h
│   ├── page.h
│   └── node.h
├── src/
│   ├── btree.cpp
│   ├── page.cpp
│   ├── node.cpp
│   └── main.cpp
├── tests/
│   └── test_btree.cpp
└── CMakeLists.txt
```

#### Implementation Tasks
- [ ] Define B+ tree node structure
- [ ] Implement page management
- [ ] Implement insert operation
- [ ] Implement search operation
- [ ] Implement page splitting
- [ ] Implement page merging
- [ ] Write comprehensive tests

#### Learning Resources
- B+ tree algorithms
- Tree balancing
- Page management
- Index structures

#### Success Criteria
- [ ] Compiles without errors
- [ ] Insert/search operations work
- [ ] Page splitting works correctly
- [ ] Tree remains balanced
- [ ] Performance benchmarks pass
- [ ] All tests pass

---

### Phase 3: SQL Parser (Week 3-4)

**Goal**: Parse SQL commands into executable form

#### What You'll Build
- Lexer (tokenization)
- Parser (syntax analysis)
- AST (Abstract Syntax Tree)
- Support: CREATE TABLE, INSERT, SELECT, DELETE

#### Key Concepts
- Lexical analysis
- Syntax analysis
- Abstract syntax trees
- Grammar definition
- Error handling

#### Files to Create
```
phase3/
├── include/
│   ├── lexer.h
│   ├── parser.h
│   ├── ast.h
│   └── token.h
├── src/
│   ├── lexer.cpp
│   ├── parser.cpp
│   ├── ast.cpp
│   └── main.cpp
├── tests/
│   └── test_parser.cpp
└── CMakeLists.txt
```

#### Implementation Tasks
- [ ] Define token types
- [ ] Implement lexer
- [ ] Implement parser
- [ ] Define AST node types
- [ ] Support CREATE TABLE
- [ ] Support INSERT
- [ ] Support SELECT
- [ ] Support DELETE
- [ ] Handle errors
- [ ] Write comprehensive tests

#### Learning Resources
- Compiler design
- Lexical analysis
- Syntax analysis
- Grammar theory
- AST construction

#### Success Criteria
- [ ] Compiles without errors
- [ ] Lexer tokenizes correctly
- [ ] Parser builds valid AST
- [ ] All SQL commands parsed
- [ ] Error messages helpful
- [ ] All tests pass

---

### Phase 4: Buffer Pool (Week 4-5)

**Goal**: Implement cache management for pages

#### What You'll Build
- Page cache
- LRU eviction policy
- Dirty page tracking
- Page replacement strategies

#### Key Concepts
- Cache management
- LRU algorithm
- Page replacement
- Dirty page tracking
- Memory optimization

#### Files to Create
```
phase4/
├── include/
│   ├── buffer_pool.h
│   ├── lru_cache.h
│   ├── page_frame.h
│   └── replacement_policy.h
├── src/
│   ├── buffer_pool.cpp
│   ├── lru_cache.cpp
│   ├── page_frame.cpp
│   └── main.cpp
├── tests/
│   └── test_buffer_pool.cpp
└── CMakeLists.txt
```

#### Implementation Tasks
- [ ] Define page frame structure
- [ ] Implement LRU cache
- [ ] Implement buffer pool
- [ ] Track dirty pages
- [ ] Implement page eviction
- [ ] Handle page replacement
- [ ] Write comprehensive tests
- [ ] Benchmark performance

#### Learning Resources
- Cache management
- LRU algorithm
- Page replacement policies
- Memory optimization
- Performance tuning

#### Success Criteria
- [ ] Compiles without errors
- [ ] LRU eviction works
- [ ] Dirty pages tracked
- [ ] Page replacement works
- [ ] Performance improved
- [ ] All tests pass

---

### Phase 5: Transactions (Week 5-6)

**Goal**: Add transaction support with ACID properties

#### What You'll Build
- Write-Ahead Logging (WAL)
- Transaction commit/rollback
- Crash recovery
- Checkpoints

#### Key Concepts
- Transaction processing
- ACID properties
- Write-ahead logging
- Crash recovery
- Checkpoints
- Undo/redo logs

#### Files to Create
```
phase5/
├── include/
│   ├── transaction.h
│   ├── wal.h
│   ├── recovery.h
│   ├── log_record.h
│   └── checkpoint.h
├── src/
│   ├── transaction.cpp
│   ├── wal.cpp
│   ├── recovery.cpp
│   ├── log_record.cpp
│   └── main.cpp
├── tests/
│   └── test_transaction.cpp
└── CMakeLists.txt
```

#### Implementation Tasks
- [ ] Define transaction structure
- [ ] Implement WAL
- [ ] Implement commit operation
- [ ] Implement rollback operation
- [ ] Implement crash recovery
- [ ] Implement checkpoints
- [ ] Handle undo/redo logs
- [ ] Write comprehensive tests

#### Learning Resources
- Transaction processing
- ACID properties
- Write-ahead logging
- Crash recovery
- Checkpoint mechanisms

#### Success Criteria
- [ ] Compiles without errors
- [ ] Transactions work
- [ ] Commit/rollback works
- [ ] Crash recovery works
- [ ] Checkpoints work
- [ ] All tests pass

---

### Phase 6: Concurrency Control (Week 6-7)

**Goal**: Handle multiple concurrent transactions

#### What You'll Build
- Lock manager
- MVCC (Multi-Version Concurrency Control)
- Deadlock detection
- Isolation levels

#### Key Concepts
- Concurrency control
- Locking mechanisms
- MVCC
- Deadlock detection
- Isolation levels
- Transaction scheduling

#### Files to Create
```
phase6/
├── include/
│   ├── lock_manager.h
│   ├── mvcc.h
│   ├── deadlock.h
│   ├── lock.h
│   └── version.h
├── src/
│   ├── lock_manager.cpp
│   ├── mvcc.cpp
│   ├── deadlock.cpp
│   ├── lock.cpp
│   └── main.cpp
├── tests/
│   └── test_concurrency.cpp
└── CMakeLists.txt
```

#### Implementation Tasks
- [ ] Define lock types
- [ ] Implement lock manager
- [ ] Implement MVCC
- [ ] Implement version management
- [ ] Implement deadlock detection
- [ ] Support isolation levels
- [ ] Handle lock conflicts
- [ ] Write comprehensive tests

#### Learning Resources
- Concurrency control
- Locking mechanisms
- MVCC algorithms
- Deadlock detection
- Isolation levels

#### Success Criteria
- [ ] Compiles without errors
- [ ] Locks work correctly
- [ ] MVCC works
- [ ] Deadlock detection works
- [ ] Isolation levels work
- [ ] All tests pass

---

### Phase 7: Client-Server (Week 7-8)

**Goal**: Build networked database server

#### What You'll Build
- TCP server
- Binary protocol
- Connection pooling
- Client library

#### Key Concepts
- Network programming
- Binary protocols
- Connection pooling
- Client-server architecture
- Message serialization

#### Files to Create
```
phase7/
├── include/
│   ├── server.h
│   ├── client.h
│   ├── protocol.h
│   ├── connection.h
│   └── message.h
├── src/
│   ├── server.cpp
│   ├── client.cpp
│   ├── protocol.cpp
│   ├── connection.cpp
│   └── main.cpp
├── tests/
│   └── test_server.cpp
└── CMakeLists.txt
```

#### Implementation Tasks
- [ ] Define binary protocol
- [ ] Implement TCP server
- [ ] Implement client library
- [ ] Implement connection pooling
- [ ] Implement message serialization
- [ ] Handle multiple clients
- [ ] Implement error handling
- [ ] Write comprehensive tests

#### Learning Resources
- Network programming
- Binary protocols
- Connection pooling
- Client-server architecture
- Message serialization

#### Success Criteria
- [ ] Compiles without errors
- [ ] Server starts and listens
- [ ] Client can connect
- [ ] Messages sent/received
- [ ] Connection pooling works
- [ ] All tests pass

---

## 🏗️ Architecture Progression

### Phase 1: Simple Storage
```
┌──────────────┐
│ Key-Value    │
│ Store        │
└──────┬───────┘
       │
    ┌──▼──┐
    │File │
    └─────┘
```

### Phase 2: Indexed Storage
```
┌──────────────┐
│ Key-Value    │
│ Store        │
└──────┬───────┘
       │
    ┌──▼──────┐
    │ B+ Tree │
    │ Index   │
    └──┬──────┘
       │
    ┌──▼──┐
    │File │
    └─────┘
```

### Phase 3: SQL Interface
```
┌──────────────┐
│ SQL Parser   │
└──────┬───────┘
       │
    ┌──▼──────────┐
    │ Executor    │
    └──────┬──────┘
           │
    ┌──────▼──────┐
    │ Key-Value   │
    │ Store       │
    └──────┬──────┘
           │
        ┌──▼──────┐
        │ B+ Tree │
        │ Index   │
        └──┬──────┘
           │
        ┌──▼──┐
        │File │
        └─────┘
```

### Phase 4: Buffered Storage
```
┌──────────────┐
│ SQL Parser   │
└──────┬───────┘
       │
    ┌──▼──────────┐
    │ Executor    │
    └──────┬──────┘
           │
    ┌──────▼──────────┐
    │ Buffer Pool     │
    │ (LRU Cache)     │
    └──────┬──────────┘
           │
    ┌──────▼──────┐
    │ Key-Value   │
    │ Store       │
    └──────┬──────┘
           │
        ┌──▼──────┐
        │ B+ Tree │
        │ Index   │
        └──┬──────┘
           │
        ┌──▼──┐
        │File │
        └─────┘
```

### Phase 5: Transactional Storage
```
┌──────────────┐
│ SQL Parser   │
└──────┬───────┘
       │
    ┌──▼──────────┐
    │ Executor    │
    └──────┬──────┘
           │
    ┌──────▼──────────┐
    │ Transaction     │
    │ Manager         │
    └──────┬──────────┘
           │
    ┌──────▼──────────┐
    │ Buffer Pool     │
    │ (LRU Cache)     │
    └──────┬──────────┘
           │
    ┌──────▼──────┐
    │ Key-Value   │
    │ Store       │
    └──────┬──────┘
           │
        ┌──▼──────┐
        │ B+ Tree │
        │ Index   │
        └──┬──────┘
           │
    ┌──────▼──────┐
    │ WAL (Log)   │
    └──────┬──────┘
           │
        ┌──▼──┐
        │File │
        └─────┘
```

### Phase 6: Concurrent Storage
```
┌──────────────┐
│ SQL Parser   │
└──────┬───────┘
       │
    ┌──▼──────────┐
    │ Executor    │
    └──────┬──────┘
           │
    ┌──────▼──────────┐
    │ Lock Manager    │
    │ & MVCC          │
    └──────┬──────────┘
           │
    ┌──────▼──────────┐
    │ Transaction     │
    │ Manager         │
    └──────┬──────────┘
           │
    ┌──────▼──────────┐
    │ Buffer Pool     │
    │ (LRU Cache)     │
    └──────┬──────────┘
           │
    ┌──────▼──────┐
    │ Key-Value   │
    │ Store       │
    └──────┬──────┘
           │
        ┌──▼──────┐
        │ B+ Tree │
        │ Index   │
        └──┬──────┘
           │
    ┌──────▼──────┐
    │ WAL (Log)   │
    └──────┬──────┘
           │
        ┌──▼──┐
        │File │
        └─────┘
```

### Phase 7: Networked Database
```
┌──────────────┐
│ Client 1     │
└──────┬───────┘
       │
    ┌──▼──────────┐
    │ TCP Server  │
    │ & Protocol  │
    └──────┬──────┘
           │
    ┌──────▼──────────┐
    │ SQL Parser      │
    └──────┬──────────┘
           │
    ┌──────▼──────────┐
    │ Executor        │
    └──────┬──────────┘
           │
    ┌──────▼──────────┐
    │ Lock Manager    │
    │ & MVCC          │
    └──────┬──────────┘
           │
    ┌──────▼──────────┐
    │ Transaction     │
    │ Manager         │
    └──────┬──────────┘
           │
    ┌──────▼──────────┐
    │ Buffer Pool     │
    │ (LRU Cache)     │
    └──────┬──────────┘
           │
    ┌──────▼──────┐
    │ Key-Value   │
    │ Store       │
    └──────┬──────┘
           │
        ┌──▼──────┐
        │ B+ Tree │
        │ Index   │
        └──┬──────┘
           │
    ┌──────▼──────┐
    │ WAL (Log)   │
    └──────┬──────┘
           │
        ┌──▼──┐
        │File │
        └─────┘
```

---

## 📈 Performance Progression

```
Throughput (ops/sec)
    ↑
    │                                    ╱─── Phase 7: 10,000 req/s
    │                                ╱───
    │                            ╱───
    │                        ╱───
    │                    ╱───
    │                ╱───
    │            ╱───
    │        ╱───
    │    ╱───
    │╱───
    └─────────────────────────────────────→ Phase
      1    2    3    4    5    6    7

Phase 1: ~1,000 ops/s (simple storage)
Phase 2: ~10,000 ops/s (indexed)
Phase 3: ~10,000 ops/s (parsed)
Phase 4: ~50,000 ops/s (buffered)
Phase 5: ~10,000 txn/s (transactional)
Phase 6: ~50,000 ops/s (concurrent)
Phase 7: ~10,000 req/s (networked)
```

---

## 🧪 Testing Strategy

### Unit Tests
- Test individual components
- Verify correctness
- Use Google Test or Catch2

### Integration Tests
- Test multiple components
- Test workflows
- Verify interactions

### Performance Tests
- Benchmark throughput
- Measure latency
- Profile memory

### Stress Tests
- Concurrent operations
- Large datasets
- Recovery scenarios

---

## 📚 Key Concepts by Phase

### Phase 1: File I/O & Serialization
- Binary file formats
- Data serialization
- Memory management
- Error handling

### Phase 2: Data Structures
- Tree algorithms
- Page management
- Search algorithms
- Balancing

### Phase 3: Compiler Design
- Lexical analysis
- Syntax analysis
- AST construction
- Error recovery

### Phase 4: Cache Management
- LRU algorithm
- Page replacement
- Memory optimization
- Performance tuning

### Phase 5: Transaction Processing
- ACID properties
- Write-ahead logging
- Crash recovery
- Checkpoints

### Phase 6: Concurrency Control
- Locking mechanisms
- MVCC
- Deadlock detection
- Isolation levels

### Phase 7: Networking
- TCP/IP
- Binary protocols
- Connection pooling
- Message serialization

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

## 🎯 Completion Checklist

- [ ] Phase 1: Key-Value Store complete
- [ ] Phase 2: B+ Tree Index complete
- [ ] Phase 3: SQL Parser complete
- [ ] Phase 4: Buffer Pool complete
- [ ] Phase 5: Transactions complete
- [ ] Phase 6: Concurrency complete
- [ ] Phase 7: Client-Server complete
- [ ] All tests passing
- [ ] Performance benchmarks documented
- [ ] Code reviewed and documented

---

## 🚀 Next Steps

1. Read **SETUP.md** - Set up environment
2. Read **Phase 1 README** - Start coding
3. Keep **QUICK_REFERENCE.md** handy
4. Progress through phases
5. Celebrate your success!

---

**Happy Learning! 📚**
