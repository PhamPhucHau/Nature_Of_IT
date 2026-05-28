# 🗄️ Phase 4: Buffer Pool Manager

## Goal
Implement a buffer pool (page cache) for efficient memory management. This phase teaches cache management concepts: page replacement policies, dirty page handling, and memory optimization.

---

## What You'll Learn

### Core Concepts
- **Buffer Pool** - In-memory cache for disk pages
- **Page Management** - Loading and evicting pages
- **LRU Replacement** - Least Recently Used eviction policy
- **Dirty Pages** - Pages modified in memory
- **Pin/Unpin** - Reference counting for pages
- **Flush** - Writing dirty pages to disk

### Practical Skills
- Cache management algorithms
- LRU data structure implementation
- Page lifecycle management
- Memory optimization
- Performance tuning

---

## What You'll Build

### Features
- ✅ Buffer Pool - In-memory page cache
- ✅ Page Manager - Load/evict pages
- ✅ LRU Replacement - Eviction policy
- ✅ Dirty Page Tracking - Track modified pages
- ✅ Pin/Unpin - Reference counting
- ✅ Flush - Write pages to disk
- ✅ Statistics - Performance metrics

### Architecture
```
Application
    ↓
Buffer Pool Manager
    ├── Page Table (hash map)
    ├── LRU List (doubly linked list)
    ├── Free List (available frames)
    └── Disk Manager
        ↓
    Disk Storage
```

---

## File Structure

```
phase4/
├── include/
│   ├── page.h           # Page structure
│   ├── buffer_pool.h    # Buffer pool API
│   ├── lru.h            # LRU replacement policy
│   └── disk_manager.h   # Disk I/O operations
├── src/
│   ├── page.c           # Page implementation
│   ├── buffer_pool.c    # Buffer pool implementation
│   ├── lru.c            # LRU implementation
│   ├── disk_manager.c   # Disk manager implementation
│   └── main.c           # Test program
├── tests/
│   └── test_buffer_pool.c # Unit tests
├── Makefile             # Build configuration
└── README.md            # This file
```

---

## Building and Running

### Build
```bash
cd Database/Oracle/C/phase4
make
```

### Run
```bash
./bin/buffer_pool_test
```

### Run with Debugging
```bash
# With debug symbols
make debug

# With memory checking
valgrind --leak-check=full ./bin/buffer_pool_test

# With GDB
gdb ./bin/buffer_pool_test
```

### Clean
```bash
make clean
```

---

## Key Concepts Explained

### 1. Buffer Pool
In-memory cache that stores frequently accessed disk pages:
```
Buffer Pool (RAM)
├── Frame 0: Page 1
├── Frame 1: Page 5
├── Frame 2: Page 3
└── Frame 3: (empty)

Disk
├── Page 0
├── Page 1
├── Page 2
├── Page 3
├── Page 4
└── Page 5
```

### 2. Page Structure
```c
typedef struct {
    uint32_t page_id;           // Unique page identifier
    char data[PAGE_SIZE];       // Page data (4KB)
    int is_dirty;               // Modified flag
    uint32_t pin_count;         // Reference count
    uint32_t access_time;       // Last access time
} Page;
```

### 3. LRU Replacement
When buffer pool is full, evict least recently used page:
```
Access order: A, B, C, A, D, B, E
LRU order:    C, D, E (C is least recently used)
```

### 4. Pin/Unpin
Reference counting to prevent eviction:
```
pin_count = 0   → Can be evicted
pin_count > 0   → Cannot be evicted
```

### 5. Dirty Pages
Track pages modified in memory:
```
Page modified → is_dirty = 1
Flush to disk → is_dirty = 0
```

---

## Testing

### Unit Tests
```bash
make run
```

### Manual Testing
```bash
./bin/buffer_pool_test
```

### Memory Checking
```bash
valgrind --leak-check=full ./bin/buffer_pool_test
```

### Performance Testing
```bash
# Measure cache hit rate
# Measure eviction count
# Measure flush count
```

---

## Debugging Tips

### 1. Print Buffer Pool State
```c
buffer_pool_print_stats(pool);
```

### 2. Print Page Information
```c
page_print(page);
```

### 3. Enable Debug Output
```bash
make debug
```

### 4. Use GDB
```bash
gdb ./bin/buffer_pool_test
(gdb) break buffer_pool_fetch_page
(gdb) run
(gdb) print pool->page_count
```

---

## Common Issues

### Issue: "No free frames"
**Cause**: Buffer pool is full and no pages can be evicted
**Solution**: 
1. Increase buffer pool size
2. Check for pages with pin_count > 0
3. Verify flush operations

### Issue: "Memory leak"
**Cause**: Pages not freed properly
**Solution**:
1. Use valgrind to find leak
2. Check buffer_pool_destroy() calls
3. Verify page cleanup

### Issue: "Segmentation fault"
**Cause**: Null pointer or invalid memory
**Solution**:
1. Use GDB to find line
2. Check pointer initialization
3. Verify bounds

---

## Performance Expectations

### Time Complexity
- **Fetch Page**: O(1) average (hash table lookup)
- **Evict Page**: O(1) (LRU list removal)
- **Flush Page**: O(1) (disk write)

### Space Complexity
- **Buffer Pool**: O(n) where n = number of frames

### Expected Performance
- **Cache Hit Rate**: 80-95% for typical workloads
- **Eviction Rate**: 5-20% of accesses
- **Flush Rate**: 1-5% of accesses

---

## Next Steps

After completing Phase 4:
1. ✅ Understand buffer pool management
2. ✅ Understand LRU replacement
3. ✅ Understand page lifecycle
4. Move to Phase 5: Transactions (WAL, ACID)

---

## Learning Resources

### Buffer Pool Management
- "Database Internals" by Alex Petrov
- "Architecture of a Database System"
- PostgreSQL Buffer Manager source code

### LRU Cache
- "Designing Data-Intensive Applications"
- LeetCode LRU Cache problem

### Disk I/O
- Operating Systems textbooks
- Linux file I/O documentation

---

## Success Criteria

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] No memory leaks (valgrind clean)
- [ ] Buffer pool fetches pages correctly
- [ ] LRU eviction works properly
- [ ] Dirty pages tracked correctly
- [ ] Pin/unpin reference counting works
- [ ] Flush operations work
- [ ] Error messages helpful
- [ ] Code is well-commented

---

**Happy Learning! 📚**
