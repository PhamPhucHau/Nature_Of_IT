# 🚀 Phase 6: Quick Start Guide

Get up and running with Phase 6 (Concurrency Control) in 5 minutes.

---

## Prerequisites

- GCC compiler
- Make build tool
- Valgrind (optional, for memory checking)
- GDB (optional, for debugging)

---

## Build

```bash
cd Database/Oracle/C/phase6
make
```

**Output:**
```
Compiling src/lock.c...
Compiling src/mvcc.c...
Compiling src/deadlock.c...
Compiling src/isolation.c...
Compiling src/main.c...
Linking bin/concurrency_test...
Build complete: bin/concurrency_test
```

---

## Run Tests

```bash
make run
```

**Expected Output:**
```
╔════════════════════════════════════════════════════════════╗
║      Concurrency Control Test Suite - Phase 6              ║
╚════════════════════════════════════════════════════════════╝

=== Test: Lock Manager Create/Destroy ===
✓ PASS: Lock manager created
✓ PASS: Lock count is 0

... (more tests) ...

╔════════════════════════════════════════════════════════════╗
║                      TEST SUMMARY                           ║
╚════════════════════════════════════════════════════════════╝
Total Tests: 32
Passed: 32
Failed: 0

✓ All tests passed!
```

---

## Debug with GDB

```bash
make gdb
```

**In GDB:**
```
(gdb) break lock_manager_acquire_lock
(gdb) run
(gdb) print mgr->lock_count
(gdb) continue
(gdb) quit
```

---

## Check Memory Leaks

```bash
make valgrind
```

**Expected Output:**
```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap alloc: X bytes in Y blocks
==12345==   total heap freed: X bytes in Y blocks
==12345==   total heap usage: Y allocs, Y frees, X bytes allocated
==12345== 
==12345== All heap blocks were freed -- no leaks are possible
```

---

## Clean Build Artifacts

```bash
make clean
```

---

## File Structure

```
phase6/
├── include/
│   ├── lock.h           # Lock manager API
│   ├── mvcc.h           # MVCC API
│   ├── deadlock.h       # Deadlock detection API
│   └── isolation.h      # Isolation levels API
├── src/
│   ├── lock.c           # Lock manager implementation
│   ├── mvcc.c           # MVCC implementation
│   ├── deadlock.c       # Deadlock detection implementation
│   ├── isolation.c      # Isolation levels implementation
│   └── main.c           # Test program
├── Makefile             # Build configuration
├── README.md            # Full documentation
└── QUICK_START.md       # This file
```

---

## Key Concepts

### Lock Manager
Manages locks on resources:
```c
LockManager *mgr = lock_manager_create();
lock_manager_acquire_lock(mgr, txn_id, resource_id, LOCK_SHARED);
lock_manager_release_lock(mgr, txn_id, resource_id);
lock_manager_destroy(mgr);
```

### MVCC
Multi-version concurrency control:
```c
MVCCManager *mgr = mvcc_manager_create();
uint64_t v = mvcc_manager_create_version(mgr, record_id, txn_id, "data");
mvcc_manager_commit_version(mgr, v);
mvcc_manager_destroy(mgr);
```

### Deadlock Detection
Detect and resolve deadlocks:
```c
DeadlockDetector *detector = deadlock_detector_create(VICTIM_YOUNGEST);
deadlock_detector_add_edge(detector, txn1, txn2, resource_id);
if (deadlock_detector_detect(detector)) {
    deadlock_detector_resolve(detector);
}
deadlock_detector_destroy(detector);
```

### Isolation Levels
Control transaction isolation:
```c
IsolationManager *mgr = isolation_manager_create(ISOLATION_SERIALIZABLE);
int allows_dirty = isolation_manager_allows_dirty_read(mgr);
isolation_manager_destroy(mgr);
```

---

## Test Coverage

- ✅ Lock Manager (8 tests)
  - Create/Destroy
  - Acquire shared/exclusive locks
  - Lock compatibility
  - Lock release
  - Get lock type

- ✅ MVCC (7 tests)
  - Create/Destroy
  - Create/read/commit/abort versions
  - Version chains
  - Garbage collection

- ✅ Deadlock Detection (7 tests)
  - Create/Destroy
  - Add/remove edges
  - Cycle detection
  - Victim selection
  - Deadlock resolution

- ✅ Isolation Levels (7 tests)
  - Create/Destroy
  - All isolation levels
  - Anomaly detection
  - Level changes

- ✅ Integration Tests (2 tests)
  - Lock + MVCC
  - Deadlock + Isolation

**Total: 32 test cases**

---

## Common Commands

| Command | Purpose |
|---------|---------|
| `make` | Build the project |
| `make run` | Build and run tests |
| `make debug` | Build with debug symbols |
| `make valgrind` | Check for memory leaks |
| `make gdb` | Debug with GDB |
| `make clean` | Remove build artifacts |
| `make help` | Show help message |

---

## Troubleshooting

### Build fails
```bash
# Check compiler
gcc --version

# Check make
make --version

# Try clean build
make clean
make
```

### Tests fail
```bash
# Run with debug output
make debug
./bin/concurrency_test

# Check with valgrind
make valgrind
```

### Memory leaks
```bash
# Run with valgrind
make valgrind

# Look for "definitely lost" or "indirectly lost"
# Check destroy() calls
```

---

## Next Steps

1. ✅ Understand lock management
2. ✅ Understand MVCC
3. ✅ Understand deadlock detection
4. ✅ Understand isolation levels
5. Move to Phase 7: Client-Server

---

## Learning Resources

- **Database Internals** by Alex Petrov
- **Designing Data-Intensive Applications** by Martin Kleppmann
- PostgreSQL concurrency documentation
- Lock compatibility matrices
- Wait-for graph algorithms

---

**Happy Learning! 📚**
