# ⚡ Quick Start - Phase 2: B+ Tree

Get up and running in 5 minutes!

---

## 🚀 Build and Run

### Step 1: Navigate to Phase 2
```bash
cd Database/Oracle/C/phase2
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
║     B+ Tree - Phase 2 Test Suite                          ║
║     Database Learning Project                             ║
╚════════════════════════════════════════════════════════════╝

=== Test: Create and Destroy ===
✓ PASS: Tree created
✓ PASS: Tree is empty
Tree destroyed successfully

=== Test: Single Insert ===
✓ PASS: Insert returns 0
✓ PASS: Count is 1

... (more tests) ...

╔════════════════════════════════════════════════════════════╗
║                    TEST SUMMARY                            ║
╚════════════════════════════════════════════════════════════╝
Passed: 30+
Failed: 0
Total:  30+

✓ All tests passed!
```

---

## 🔍 Debugging

### View Debug Output
```bash
make debug
./bin/btree_test
```

### Check for Memory Leaks
```bash
make valgrind
```

### Debug with GDB
```bash
make gdb
```

Then in GDB:
```gdb
(gdb) break btree_insert
(gdb) run
(gdb) print tree
(gdb) next
(gdb) quit
```

---

## 📚 Understanding the Code

### File Organization

```
include/
  └── btree.h         - B+ tree API

src/
  ├── btree.c         - B+ tree implementation
  └── main.c          - Tests and examples
```

### Key Functions

```c
// Create tree
BTree *tree = btree_create();

// Insert key-value pair
btree_insert(tree, 10, "Alice");

// Search for key
char value[256];
btree_search(tree, 10, value);
printf("Value: %s\n", value);  // Output: Value: Alice

// Range search
btree_range_search(tree, 10, 30, callback, NULL);

// Get statistics
printf("Entries: %u\n", btree_count(tree));
printf("Height: %u\n", btree_height(tree));

// Print tree structure
btree_print(tree);

// Clean up
btree_destroy(tree);
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
./bin/btree_test
```

### With Memory Checking
```bash
make valgrind
```

---

## 🛠️ Build Targets

```bash
make              # Build
make debug        # Build with debug symbols
make run          # Build and run
make valgrind     # Run with memory checking
make gdb          # Run with debugger
make clean        # Remove build artifacts
make help         # Show help
```

---

## 📖 Documentation

- **README.md** - Complete project guide
- **QUICK_START.md** - This file
- **DEBUGGING_GUIDE.md** - Debugging techniques (to be created)

---

## 🎯 What You'll Learn

1. **B+ Tree Structure** - Multi-level tree with sorted keys
2. **Node Types** - Internal nodes and leaf nodes
3. **Insertion** - Adding keys with automatic splitting
4. **Search** - Efficient key lookup
5. **Range Queries** - Finding keys in a range
6. **Tree Balancing** - Maintaining tree properties

---

## 💡 Tips

### Tip 1: Understand the Structure
```
B+ Tree (Order 4, Max 3 keys per node)

                    [30]
                   /    \
              [10, 20]  [40, 50]
              /  |  \    /  |  \
            [5] [15][25][35][45][55]
```

### Tip 2: Use Debug Mode
```bash
make debug
```
This shows exactly what's happening during operations.

### Tip 3: Print Tree Structure
```c
btree_print(tree);
```
This visualizes the tree structure.

### Tip 4: Study the Tests
`main.c` has 10 test cases showing how to use the API.

### Tip 5: Use GDB
```bash
make gdb
(gdb) break btree_insert
(gdb) run
```
This lets you step through code.

---

## 🐛 Common Issues

### Issue: "make: command not found"
**Solution**: Install build tools
```bash
sudo apt-get install build-essential
```

### Issue: "gcc: command not found"
**Solution**: Install compiler
```bash
sudo apt-get install gcc
```

### Issue: Tests fail
**Solution**: Check debug output
```bash
make debug
./bin/btree_test
```

### Issue: Memory leak detected
**Solution**: Check valgrind output
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
7. ✅ Move to Phase 3

---

**You're ready! Start with `make run` 🚀**
