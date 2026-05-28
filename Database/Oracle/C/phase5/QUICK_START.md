# 🚀 Phase 5: Quick Start Guide

Get up and running with Phase 5 (Transactions & WAL) in 5 minutes.

---

## Prerequisites

- GCC compiler
- Make build tool
- Valgrind (optional, for memory checking)
- GDB (optional, for debugging)

---

## Build

```bash
cd Database/Oracle/C/phase5
make
```

**Output:**
```
Compiling src/transaction.c...
Compiling src/wal.c...
Compiling src/checkpoint.c...
Compiling src/log_manager.c...
Compiling src/recovery.c...
Compiling src/main.c...
Linking bin/transaction_test...
Build complete: bin/transaction_test
```

---

## Run Tests

```bash
make run
```

**Expected Output:**
```
╔════════════════════════════════════════════════════════════╗
║    Transaction & WAL Test Suite - Phase 5                  ║
╚════════════════════════════════════════════════════════════╝

=== Test: Transaction Create/Destroy ===
✓ PASS: Transaction created
✓ PASS: Transaction ID is 1
✓ PASS: Transaction is active
✓ PASS: Operation count is 0

... (more tests) ...

╔════════════════════════════════════════════════════════════╗
║                      TEST SUMMARY                           ║
╚════════════════════════════════════════════════════════════╝
Total Tests: 23
Passed: 23
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
(gdb) break transaction_commit
(gdb) run
(gdb) print txn->state
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
phase5/
├── include/
│   ├── transaction.h      # Transaction API
│   ├── wal.h              # WAL API
│   ├── checkpoint.h       # Checkpoint API
│   ├── log_manager.h      # Log Manager API
│   └── recovery.h         # Recovery API
├── src/
│   ├── transaction.c      # Transaction implementation
│   ├── wal.c              # WAL implementation
│   ├── checkpoint.c       # Checkpoint implementation
│   ├── log_manager.c      # Log Manager implementation
│   ├── recovery.c         # Recovery implementation
│   └── main.c             # Test program
├── Makefile               # Build configuration
├── README.md              # Full documentation
└── QUICK_START.md         # This file
```

---

## Key Concepts

### Transactions
Atomic units of work with ACID properties:
```c
Transaction *txn = transaction_create(1);
transaction_add_operation(txn, OP_INSERT, 1, 0, "", "value");
transaction_commit(txn);
transaction_destroy(txn);
```

### Write Ahead Logging
Log operations before writing to disk:
```c
WriteAheadLog *wal = wal_create("wal.log");
uint64_t lsn = wal_append_entry(wal, LOG_WRITE, 1, 1, 0, "old", "new");
wal_flush(wal);
wal_destroy(wal);
```

### Checkpoints
Consistency points for faster recovery:
```c
CheckpointManager *cp_mgr = checkpoint_manager_create("cp.log");
checkpoint_manager_create_checkpoint(cp_mgr, 1, 100);
checkpoint_manager_destroy(cp_mgr);
```

### Recovery
Recover from crashes:
```c
RecoveryManager *rec_mgr = recovery_manager_create(wal, cp_mgr);
recovery_manager_recover(rec_mgr);
recovery_manager_destroy(rec_mgr);
```

---

## Test Coverage

- ✅ Transaction creation/destruction
- ✅ Transaction operations
- ✅ Transaction commit/abort
- ✅ WAL append/flush
- ✅ Checkpoint creation
- ✅ Log manager operations
- ✅ Recovery phases (analysis, redo, undo)
- ✅ Full recovery process
- ✅ Integration tests

**Total: 23 test cases**

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
./bin/transaction_test

# Check with valgrind
make valgrind
```

### Memory leaks
```bash
# Run with valgrind
make valgrind

# Look for "definitely lost" or "indirectly lost"
# Check transaction_destroy() and wal_destroy() calls
```

---

## Next Steps

1. ✅ Understand transaction management
2. ✅ Understand Write Ahead Logging
3. ✅ Understand crash recovery
4. Move to Phase 6: Concurrency Control

---

## Learning Resources

- **Database Internals** by Alex Petrov
- **Designing Data-Intensive Applications** by Martin Kleppmann
- PostgreSQL WAL documentation
- ARIES recovery algorithm

---

**Happy Learning! 📚**
