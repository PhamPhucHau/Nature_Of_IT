# 🌳 Phase 2: B+ Tree Index

## Goal
Implement a B+ tree for efficient data retrieval. B+ trees are the foundation of database indexing, enabling fast searches, range queries, and sorted iteration.

---

## What You'll Learn

### Core Concepts
- **B+ Tree Structure** - Multi-level tree with sorted keys
- **Node Types** - Internal nodes and leaf nodes
- **Page Management** - Fixed-size blocks of data
- **Splitting** - Handling node overflow
- **Merging** - Handling node underflow
- **Search Algorithms** - Efficient key lookup
- **Range Queries** - Finding keys in a range

### Practical Skills
- Tree data structure implementation
- Pointer manipulation
- Memory management for trees
- Recursive algorithms
- Performance optimization

---

## What You'll Build

### Features
- ✅ B+ tree node creation and management
- ✅ Insert operation with automatic splitting
- ✅ Search operation (exact match)
- ✅ Range search (find all keys in range)
- ✅ Delete operation with merging
- ✅ Tree traversal and iteration
- ✅ Statistics and diagnostics

### Data Structure
```
B+ Tree (Order 3, Max 2 keys per node)

                    [30]
                   /    \
              [10, 20]  [40, 50]
              /  |  \    /  |  \
            [5] [15][25][35][45][55]
            
Leaf nodes contain actual data
Internal nodes contain only keys for routing
```

---

## File Structure

```
phase2/
├── include/
│   ├── btree.h          # B+ tree API
│   ├── node.h           # Node structure
│   └── page.h           # Page management
├── src/
│   ├── btree.c          # B+ tree implementation
│   ├── node.c           # Node implementation
│   ├── page.c           # Page implementation
│   └── main.c           # Test program
├── tests/
│   └── test_btree.c     # Unit tests
├── Makefile             # Build configuration
└── README.md            # This file
```

---

## Building and Running

### Build
```bash
cd Database/Oracle/C/phase2
make
```

### Run
```bash
./bin/btree_test
```

### Run with Debugging
```bash
# With debug symbols
make debug

# With memory checking
valgrind --leak-check=full ./bin/btree_test

# With GDB
gdb ./bin/btree_test
```

### Clean
```bash
make clean
```

---

## Key Concepts Explained

### 1. B+ Tree Properties
- **Order (m)**: Maximum number of children per node
- **Keys per node**: Between m/2 and m-1 (except root)
- **Leaf nodes**: Contain actual data
- **Internal nodes**: Contain only keys for routing
- **Balanced**: All leaf nodes at same depth

### 2. Node Types

#### Internal Node
```
[Key1] [Key2] [Key3]
  |      |      |      |
 Child0 Child1 Child2 Child3
```

#### Leaf Node
```
[Key1, Value1] [Key2, Value2] [Key3, Value3]
```

### 3. Insertion Process
1. Find leaf node where key should go
2. Insert key-value pair
3. If node overflows, split it
4. Propagate split up the tree

### 4. Splitting
```
Before Split (4 keys, max 3):
[10, 20, 30, 40]

After Split:
Left:  [10, 20]
Right: [30, 40]
Parent gets key 30
```

### 5. Search Algorithm
```
1. Start at root
2. Find child pointer based on key
3. Recursively search child
4. Return value if found in leaf
```

---

## Testing

### Unit Tests
```bash
make run
```

### Manual Testing
```bash
./bin/btree_test
```

### Memory Checking
```bash
valgrind --leak-check=full ./bin/btree_test
```

### Performance Testing
```bash
# Insert 1000 keys
# Search 1000 keys
# Measure time and memory
```

---

## Debugging Tips

### 1. Print Tree Structure
```c
btree_print(tree);
```

### 2. Enable Debug Output
```bash
make debug
```

### 3. Use GDB
```bash
gdb ./bin/btree_test
(gdb) break btree_insert
(gdb) run
(gdb) print tree
```

### 4. Check Memory
```bash
valgrind --leak-check=full ./bin/btree_test
```

---

## Common Issues

### Issue: "Segmentation fault"
**Cause**: Null pointer or invalid memory access
**Solution**: 
1. Use GDB to find the line
2. Check pointer initialization
3. Verify node allocation

### Issue: "Tree not balanced"
**Cause**: Splitting not working correctly
**Solution**:
1. Check split logic
2. Verify key distribution
3. Test with small examples

### Issue: "Search returns wrong result"
**Cause**: Incorrect key comparison or traversal
**Solution**:
1. Add debug output
2. Trace search path
3. Check key ordering

---

## Performance Expectations

### Time Complexity
- **Search**: O(log n) where n = number of keys
- **Insert**: O(log n) + O(m) for splitting
- **Delete**: O(log n) + O(m) for merging
- **Range Query**: O(log n + k) where k = results

### Space Complexity
- **Storage**: O(n) where n = number of keys
- **Per Node**: ~1 KB (for order 100)
- **Total**: ~10 KB for 1000 keys

### Expected Performance
- **Search**: ~100,000 ops/sec
- **Insert**: ~10,000 ops/sec
- **Delete**: ~10,000 ops/sec
- **Range Query**: ~50,000 ops/sec

---

## Next Steps

After completing Phase 2:
1. ✅ Understand B+ tree structure
2. ✅ Understand insertion and splitting
3. ✅ Understand search algorithms
4. ✅ Move to Phase 3: SQL Parser

---

## Learning Resources

### B+ Tree Algorithms
- `man btree` - B+ tree documentation
- Wikipedia: B+ tree
- Database textbooks

### Tree Traversal
- In-order traversal
- Level-order traversal
- Range queries

### Debugging
- `man gdb` - GNU Debugger
- `man valgrind` - Memory checker

---

## Success Criteria

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] No memory leaks (valgrind clean)
- [ ] Insert/search operations work
- [ ] Page splitting works correctly
- [ ] Tree remains balanced
- [ ] Performance benchmarks pass
- [ ] Code is well-commented

---

**Happy Learning! 📚**
