# 💾 Phase 5: Transactions & Write Ahead Logging

## Goal
Implement transaction management with Write Ahead Logging (WAL) for ACID properties and crash recovery. This phase teaches transaction processing concepts: atomicity, consistency, isolation, durability, and recovery.

---

## What You'll Learn

### Core Concepts
- **Transactions** - Atomic units of work
- **ACID Properties** - Atomicity, Consistency, Isolation, Durability
- **Write Ahead Logging** - Log before write principle
- **Crash Recovery** - Recovering from failures
- **Undo/Redo** - Recovery mechanisms
- **Checkpoints** - Consistency points

### Practical Skills
- Transaction management
- Log file handling
- Recovery algorithms
- Crash simulation and recovery
- Durability guarantees

---

## What You'll Build

### Features
- ✅ Transaction Manager - Manage transactions
- ✅ Write Ahead Logging - Log operations before commit
- ✅ Undo/Redo Logs - Recovery information
- ✅ Checkpoints - Consistency points
- ✅ Crash Recovery - Recover from failures
- ✅ Durability - Guarantee data persistence
- ✅ Atomicity - All or nothing

### Architecture
```
Application
    ↓
Transaction Manager
├── Active Transactions
├── Log Manager
│   ├── WAL (Write Ahead Log)
│   ├── Undo Logs
│   └── Redo Logs
├── Checkpoint Manager
└── Recovery Manager
    ↓
Buffer Pool
    ↓
Disk Storage
```

---

## File Structure

```
phase5/
├── include/
│   ├── transaction.h     # Transaction structure
│   ├── log_manager.h     # Log management
│   ├── wal.h             # Write Ahead Logging
│   ├── checkpoint.h      # Checkpoint management
│   └── recovery.h        # Recovery operations
├── src/
│   ├── transaction.c     # Transaction implementation
│   ├── log_manager.c     # Log manager implementation
│   ├── wal.c             # WAL implementation
│   ├── checkpoint.c      # Checkpoint implementation
│   ├── recovery.c        # Recovery implementation
│   └── main.c            # Test program
├── Makefile              # Build configuration
└── README.md             # This file
```

---

## Building and Running

### Build
```bash
cd Database/Oracle/C/phase5
make
```

### Run
```bash
./bin/transaction_test
```

### Run with Debugging
```bash
# With debug symbols
make debug

# With memory checking
valgrind --leak-check=full ./bin/transaction_test

# With GDB
gdb ./bin/transaction_test
```

### Clean
```bash
make clean
```

---

## Key Concepts Explained

### 1. Transactions
Atomic units of work that either complete fully or not at all:
```
BEGIN TRANSACTION
  UPDATE account SET balance = balance - 100 WHERE id = 1
  UPDATE account SET balance = balance + 100 WHERE id = 2
COMMIT
```

### 2. ACID Properties
```
Atomicity:    All or nothing
Consistency:  Valid state to valid state
Isolation:    Concurrent transactions don't interfere
Durability:   Committed data survives failures
```

### 3. Write Ahead Logging
Log operations before writing to disk:
```
1. Write operation to log
2. Flush log to disk
3. Write data to buffer pool
4. Flush data to disk
5. Mark log entry as committed
```

### 4. Crash Recovery
Recover from failures using logs:
```
Redo Phase:   Replay committed transactions
Undo Phase:   Rollback uncommitted transactions
```

### 5. Checkpoints
Consistency points to reduce recovery time:
```
Checkpoint:
  1. Flush all dirty pages
  2. Write checkpoint record
  3. Flush checkpoint to log
```

---

## Testing

### Unit Tests
```bash
make run
```

### Manual Testing
```bash
./bin/transaction_test
```

### Memory Checking
```bash
valgrind --leak-check=full ./bin/transaction_test
```

### Crash Simulation
```bash
# Simulate crash during transaction
# Verify recovery works correctly
```

---

## Debugging Tips

### 1. Print Transaction State
```c
transaction_print_state(txn);
```

### 2. Print Log Entries
```c
log_manager_print_entries(log_mgr);
```

### 3. Enable Debug Output
```bash
make debug
```

### 4. Use GDB
```bash
gdb ./bin/transaction_test
(gdb) break transaction_commit
(gdb) run
(gdb) print txn->state
```

---

## Common Issues

### Issue: "Transaction not committed"
**Cause**: Transaction state not updated
**Solution**: 
1. Check transaction state
2. Verify commit logic
3. Check log entries

### Issue: "Recovery failed"
**Cause**: Log file corrupted or incomplete
**Solution**:
1. Check log file integrity
2. Verify checkpoint records
3. Check redo/undo logs

### Issue: "Memory leak"
**Cause**: Transactions not freed properly
**Solution**:
1. Use valgrind to find leak
2. Check transaction_destroy() calls
3. Verify cleanup

---

## Performance Expectations

### Time Complexity
- **Begin Transaction**: O(1)
- **Commit Transaction**: O(n) where n = log entries
- **Rollback Transaction**: O(n)
- **Recovery**: O(m) where m = log size

### Space Complexity
- **Transaction**: O(1)
- **Log**: O(n) where n = operations
- **Checkpoint**: O(1)

### Expected Performance
- **Commit Latency**: 1-10ms (disk dependent)
- **Recovery Time**: 100ms-1s (log size dependent)
- **Throughput**: 1000-10000 transactions/sec

---

## Next Steps

After completing Phase 5:
1. ✅ Understand transaction management
2. ✅ Understand Write Ahead Logging
3. ✅ Understand crash recovery
4. move to Phase 6: Concurrency Control

---

## Learning Resources

### Transaction Processing
- "Database Internals" by Alex Petrov
- "Architecture of a Database System"
- PostgreSQL WAL documentation

### ACID Properties
- "Designing Data-Intensive Applications"
- Database theory textbooks
- Transaction processing papers

### Recovery Algorithms
- ARIES algorithm
- Shadow paging
- Copy-on-write

---

## Success Criteria

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] No memory leaks (valgrind clean)
- [ ] Transactions work correctly
- [ ] WAL functions properly
- [ ] Crash recovery works
- [ ] Checkpoints work
- [ ] Error messages helpful
- [ ] Code is well-commented

---

**Happy Learning! 📚**
