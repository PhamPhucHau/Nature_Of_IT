# ⚡ Quick Start Guide - Phase 1

Get up and running in 5 minutes!

---

## 🚀 Build and Run

### Step 1: Navigate to Phase 1
```bash
cd web_server/language/C/phase1
```

### Step 2: Build
```bash
make
```

### Step 3: Run Tests
```bash
make run
```

You should see:
```
╔════════════════════════════════════════════════════════════╗
║     Key-Value Store - Phase 1 Test Suite                  ║
║     Database Learning Project                             ║
╚════════════════════════════════════════════════════════════╝

=== Test: Create and Destroy ===
✓ PASS: Storage created
✓ PASS: Storage is empty
Storage destroyed successfully

=== Test: Put and Get ===
✓ PASS: Get returns correct value
✓ PASS: Get user:2
✓ PASS: Get user:3
✓ PASS: Get non-existent key returns NULL
✓ PASS: Entry count is 3

... (more tests) ...

╔════════════════════════════════════════════════════════════╗
║                    TEST SUMMARY                            ║
╚════════════════════════════════════════════════════════════╝
Passed: 30
Failed: 0
Total:  30

✓ All tests passed!
```

---

## 🔍 Debugging

### View Debug Output
```bash
make debug
./bin/storage_test
```

You'll see detailed debug information:
```
[DEBUG] storage.c:123: Creating storage with filename: test.db
[DEBUG] storage.c:145: Storage created successfully
[DEBUG] storage.c:200: Putting key='user:1', value='Alice'
[DEBUG] storage.c:220: Hashing key: user:1
[DEBUG] storage.c:230: Hash index: 5
...
```

### Check for Memory Leaks
```bash
make valgrind
```

Should show:
```
==12345== LEAK SUMMARY:
==12345==    definitely lost: 0 bytes in 0 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==      possibly lost: 0 bytes in 0 blocks
==12345==    still reachable: 0 bytes in 0 blocks
```

### Debug with GDB
```bash
make gdb
```

Then in GDB:
```gdb
(gdb) break storage_put
(gdb) run
(gdb) print key
(gdb) next
(gdb) quit
```

---

## 📚 Understanding the Code

### File Organization

```
include/
  ├── storage.h       - Storage API (what you can do)
  └── serializer.h    - Serialization API

src/
  ├── storage.c       - Storage implementation (how it works)
  ├── serializer.c    - Serializer implementation
  └── main.c          - Tests and examples
```

### Key Functions

```c
// Create storage
Storage *store = storage_create("data.db");

// Store data
storage_put(store, "user:1", "Alice");

// Retrieve data
const char *value = storage_get(store, "user:1");
printf("Value: %s\n", value);  // Output: Value: Alice

// Save to disk
storage_save(store);

// Load from disk
storage_load(store);

// Clean up
storage_destroy(store);
```

---

## 🧪 Running Tests

### All Tests
```bash
make run
```

### With Debug Output
```bash
make debug
./bin/storage_test
```

### With Memory Checking
```bash
make valgrind
```

### With System Call Tracing
```bash
make strace
cat strace.log
```

---

## 🛠️ Build Targets

```bash
make              # Build
make debug        # Build with debug symbols
make run          # Build and run
make valgrind     # Run with memory checking
make gdb          # Run with debugger
make strace       # Run with system call tracing
make clean        # Remove build artifacts
make help         # Show help
```

---

## 📖 Documentation

- **README.md** - Complete project guide
- **DEBUGGING_GUIDE.md** - Debugging techniques
- **COMPLETION_SUMMARY.md** - What was implemented
- **QUICK_START.md** - This file

---

## 🎯 What You'll Learn

1. **Hash Tables** - Fast key-value lookup
2. **File I/O** - Reading and writing files
3. **Serialization** - Converting data to bytes
4. **Memory Management** - Allocating and freeing memory
5. **Debugging** - Finding and fixing bugs
6. **Testing** - Writing and running tests

---

## 💡 Tips

### Tip 1: Read the Comments
Every function has detailed comments explaining what it does.

### Tip 2: Use Debug Mode
```bash
make debug
```
This shows exactly what's happening.

### Tip 3: Check Memory
```bash
make valgrind
```
This finds memory leaks.

### Tip 4: Study the Tests
`main.c` has 9 test cases showing how to use the API.

### Tip 5: Use GDB
```bash
make gdb
(gdb) break storage_put
(gdb) run
```
This lets you step through code.

---

## 🐛 Common Issues

### Issue: "make: command not found"
**Solution**: Install build tools
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# macOS
brew install make
```

### Issue: "gcc: command not found"
**Solution**: Install compiler
```bash
# Ubuntu/Debian
sudo apt-get install gcc

# macOS
xcode-select --install
```

### Issue: Tests fail
**Solution**: Check debug output
```bash
make debug
./bin/storage_test
```

### Issue: Memory leak detected
**Solution**: Read DEBUGGING_GUIDE.md
```bash
make valgrind
```

---

## 🚀 Next Steps

1. ✅ Build and run: `make run`
2. ✅ Read README.md
3. ✅ Study the code
4. ✅ Debug with GDB: `make gdb`
5. ✅ Check memory: `make valgrind`
6. ✅ Modify and experiment
7. ✅ Move to Phase 2

---

## 📞 Need Help?

1. **Read README.md** - Project overview
2. **Read DEBUGGING_GUIDE.md** - Debugging help
3. **Check comments** - Code is well-commented
4. **Study main.c** - Has usage examples
5. **Use GDB** - `make gdb`

---

**You're ready! Start with `make run` 🚀**
