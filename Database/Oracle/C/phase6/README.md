# 🔒 Phase 6: Concurrency Control

## Goal
Implement concurrency control mechanisms for safe concurrent transaction execution. This phase teaches concurrency control concepts: locking, MVCC, deadlock detection, and isolation levels.

---

## What You'll Learn

### Core Concepts
- **Locks** - Synchronization primitives
- **Lock Types** - Shared and exclusive locks
- **Lock Compatibility** - Which locks can coexist
- **MVCC** - Multi-Version Concurrency Control
- **Deadlock** - Circular wait conditions
- **Isolation Levels** - Degree of isolation
- **Read Anomalies** - Dirty, non-repeatable, phantom reads

### Practical Skills
- Lock management
- Version control
- Deadlock detection and resolution
- Isolation level enforcement
- Concurrency optimization

---

## What You'll Build

### Features
- ✅ Lock Manager - Manage shared/exclusive locks
- ✅ MVCC - Multi-version concurrency control
- ✅ Deadlock Detector - Detect circular dependencies
- ✅ Isolation Levels - READ_UNCOMMITTED, READ_COMMITTED, REPEATABLE_READ, SERIALIZABLE
- ✅ Conflict Resolution - Automatic deadlock resolution
- ✅ Version Management - Version chains and garbage collection
- ✅ Anomaly Detection - Detect read anomalies

### Architecture
```
Application
    ↓
Concurrency Control Manager
├── Lock Manager
│   ├── Shared Locks
│   ├── Exclusive Locks
│   └── Lock Compatibility
├── MVCC Manager
│   ├── Version Chains
│   ├── Read Sets
│   └── Garbage Collection
├── Deadlock Detector
│   ├── Wait-For Graph
│   ├── Cycle Detection
│   └── Victim Selection
└── Isolation Manager
    ├── Isolation Levels
    ├── Anomaly Detection
    └── Conflict Prevention
    ↓
Transaction Manager
    ↓
Buffer Pool
    ↓
Disk Storage
```

---

## File Structure

```
phase6/
├── include/
│   ├── lock.h           # Lock manager
│   ├── mvcc.h           # Multi-version concurrency control
│   ├── deadlock.h       # Deadlock detection
│   └── isolation.h      # Isolation levels
├── src/
│   ├── lock.c           # Lock manager implementation
│   ├── mvcc.c           # MVCC implementation
│   ├── deadlock.c       # Deadlock detection implementation
│   ├── isolation.c      # Isolation levels implementation
│   └── main.c           # Test program
├── Makefile             # Build configuration
└── README.md            # This file
```

---

## Building and Running

### Build
```bash
cd Database/Oracle/C/phase6
make
```

### Run
```bash
./bin/concurrency_test
```

### Run with Debugging
```bash
# With debug symbols
make debug

# With memory checking
valgrind --leak-check=full ./bin/concurrency_test

# With GDB
gdb ./bin/concurrency_test
```

### Clean
```bash
make clean
```

---

## Key Concepts Explained

### 1. Lock Manager
Manages locks on resources:
```c
LockManager *mgr = lock_manager_create();
lock_manager_acquire_lock(mgr, txn_id, resource_id, LOCK_SHARED);
lock_manager_release_lock(mgr, txn_id, resource_id);
lock_manager_destroy(mgr);
```

**Lock Types:**
- **SHARED**: Multiple readers, no writers
- **EXCLUSIVE**: Single writer, no readers

**Lock Compatibility Matrix:**
```
        | SHARED | EXCLUSIVE
--------|--------|----------
SHARED  |   ✓    |    ✗
EXCLUSIVE|   ✗    |    ✗
```

### 2. MVCC (Multi-Version Concurrency Control)
Allows concurrent reads and writes:
```c
MVCCManager *mgr = mvcc_manager_create();
uint64_t v1 = mvcc_manager_create_version(mgr, record_id, txn_id, "data");
mvcc_manager_commit_version(mgr, v1);
Version *v = mvcc_manager_read_version(mgr, record_id, txn_id);
mvcc_manager_destroy(mgr);
```

**Benefits:**
- Non-blocking reads
- Concurrent writes
- Snapshot isolation
- No dirty reads

### 3. Deadlock Detection
Detects circular wait conditions:
```c
DeadlockDetector *detector = deadlock_detector_create(VICTIM_YOUNGEST);
deadlock_detector_add_edge(detector, txn1, txn2, resource_id);
if (deadlock_detector_detect(detector)) {
    uint32_t victim = deadlock_detector_resolve(detector);
}
deadlock_detector_destroy(detector);
```

**Victim Selection Policies:**
- **YOUNGEST**: Abort transaction with highest ID
- **OLDEST**: Abort transaction with lowest ID
- **LEAST_LOCKS**: Abort transaction with fewest locks

### 4. Isolation Levels
Control visibility of uncommitted data:
```c
IsolationManager *mgr = isolation_manager_create(ISOLATION_SERIALIZABLE);
int allows_dirty = isolation_manager_allows_dirty_read(mgr);
isolation_manager_destroy(mgr);
```

**Isolation Levels:**
```
Level                  | Dirty | Non-Rep | Phantom
-----------------------|-------|---------|--------
READ_UNCOMMITTED       |  ✓    |    ✓    |   ✓
READ_COMMITTED         |  ✗    |    ✓    |   ✓
REPEATABLE_READ        |  ✗    |    ✗    |   ✓
SERIALIZABLE           |  ✗    |    ✗    |   ✗
```

---

## Testing

### Unit Tests
```bash
make run
```

### Manual Testing
```bash
./bin/concurrency_test
```

### Memory Checking
```bash
valgrind --leak-check=full ./bin/concurrency_test
```

### Debugging
```bash
gdb ./bin/concurrency_test
(gdb) break lock_manager_acquire_lock
(gdb) run
(gdb) print mgr->lock_count
```

---

## Common Issues

### Issue: "Lock timeout"
**Cause**: Transaction waiting too long for lock
**Solution**:
1. Check for deadlocks
2. Verify lock release
3. Increase timeout

### Issue: "Deadlock detected"
**Cause**: Circular wait condition
**Solution**:
1. Resolve deadlock automatically
2. Abort victim transaction
3. Retry transaction

### Issue: "Anomaly detected"
**Cause**: Read anomaly at current isolation level
**Solution**:
1. Increase isolation level
2. Use locks to prevent anomaly
3. Retry transaction

### Issue: "Memory leak"
**Cause**: Versions not freed properly
**Solution**:
1. Use valgrind to find leak
2. Check garbage collection
3. Verify destroy calls

---

## Performance Expectations

### Time Complexity
| Operation | Complexity | Notes |
|-----------|-----------|-------|
| Acquire Lock | O(n) | n = locks |
| Release Lock | O(n) | n = locks |
| Create Version | O(1) | Constant time |
| Read Version | O(m) | m = versions |
| Detect Deadlock | O(e) | e = edges |
| Resolve Deadlock | O(e) | e = edges |

### Space Complexity
| Component | Complexity | Notes |
|-----------|-----------|-------|
| Lock Manager | O(n) | n = locks |
| MVCC Manager | O(m) | m = versions |
| Deadlock Detector | O(e) | e = edges |
| Isolation Manager | O(1) | Constant |

### Expected Performance
- **Lock Acquisition**: 1-10µs
- **Lock Release**: 1-10µs
- **Version Creation**: 1-5µs
- **Deadlock Detection**: 10-100µs
- **Throughput**: 10000-100000 ops/sec

---

## Next Steps

After completing Phase 6:
1. ✅ Understand lock management
2. ✅ Understand MVCC
3. ✅ Understand deadlock detection
4. ✅ Understand isolation levels
5. Move to Phase 7: Client-Server

---

## Learning Resources

### Concurrency Control
- "Database Internals" by Alex Petrov
- "Designing Data-Intensive Applications"
- PostgreSQL concurrency documentation

### Lock Management
- Lock compatibility matrices
- Two-phase locking protocol
- Strict two-phase locking

### MVCC
- Snapshot isolation
- Version chains
- Garbage collection

### Deadlock Detection
- Wait-for graphs
- Cycle detection algorithms
- Victim selection policies

---

## Success Criteria

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] No memory leaks (valgrind clean)
- [ ] Lock manager works correctly
- [ ] MVCC functions properly
- [ ] Deadlock detection works
- [ ] Isolation levels enforced
- [ ] Error messages helpful
- [ ] Code is well-commented

---

**Happy Learning! 📚**
