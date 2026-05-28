# 🚀 Phase 4: Buffer Pool - Quick Start (5 Minutes)

## What You'll Do

Build and run a buffer pool (page cache) that manages in-memory pages efficiently using LRU replacement.

## Prerequisites

- GCC compiler
- Make build tool
- Basic C knowledge

## Step 1: Build the Project (1 minute)

```bash
cd Database/Oracle/C/phase4
make
```

Expected output:
```
Compiling src/page.c...
Compiling src/lru.c...
Compiling src/disk_manager.c...
Compiling src/buffer_pool.c...
Compiling src/main.c...
Linking bin/buffer_pool_test...
Build complete: bin/buffer_pool_test
```

## Step 2: Run the Tests (1 minute)

```bash
make run
```

Expected output:
```
╔════════════════════════════════════════════════════════════╗
║         Buffer Pool Test Suite - Phase 4                    ║
╚════════════════════════════════════════════════════════════╝

=== Test: Page Create/Destroy ===
✓ PASS: Page created
✓ PASS: Page ID is 1
✓ PASS: Pin count is 0
✓ PASS: Page is clean

... (more tests) ...

╔════════════════════════════════════════════════════════════╗
║                      TEST SUMMARY                           ║
╚════════════════════════════════════════════════════════════╝
Total Tests: 15
Passed: 15
Failed: 0

✓ All tests passed!
```

## Step 3: Understand the Components (2 minutes)

### Page (page.h/page.c)
- Represents a single page in memory
- Contains page ID, data, dirty flag, pin count
- Pin/unpin operations for reference counting

### LRU (lru.h/lru.c)
- Doubly linked list for LRU replacement
- Tracks access order of pages
- Evicts least recently used page when needed

### Disk Manager (disk_manager.h/disk_manager.c)
- Handles disk I/O operations
- Reads/writes pages from/to disk
- Manages database file

### Buffer Pool (buffer_pool.h/buffer_pool.c)
- Main cache manager
- Coordinates page, LRU, and disk manager
- Tracks statistics (hit rate, evictions, flushes)

## Step 4: Try It Yourself (1 minute)

Edit `src/main.c` and add a new test:

```c
void test_my_buffer_pool() {
    printf("\n" YELLOW "=== My Test ===" RESET "\n");
    
    DiskManager *dm = disk_manager_create("my_db.bin");
    BufferPool *pool = buffer_pool_create(5, dm);
    
    // Allocate and fetch pages
    for (int i = 0; i < 3; i++) {
        uint32_t page_id = disk_manager_allocate_page(dm);
        Page *page = buffer_pool_fetch_page(pool, page_id);
        printf("Fetched page %u\n", page_id);
        buffer_pool_unpin_page(pool, page_id, 0);
    }
    
    // Print statistics
    buffer_pool_print_stats(pool);
    
    buffer_pool_destroy(pool);
    disk_manager_destroy(dm);
}
```

Then add to main():
```c
test_my_buffer_pool();
```

Rebuild and run:
```bash
make run
```

## Debugging Tips

### Print Buffer Pool State
```bash
buffer_pool_print_stats(pool);
```

### Print LRU List
```bash
lru_print(pool->lru_list);
```

### Run with GDB
```bash
make gdb
(gdb) break buffer_pool_fetch_page
(gdb) run
(gdb) print pool->free_count
```

### Check for Memory Leaks
```bash
make valgrind
```

## Common Issues

### Issue: "No free frames"
**Solution**: Buffer pool is full. Increase pool size or check for pinned pages.

### Issue: "Memory leak"
**Solution**: Run valgrind to find leak. Check destroy() calls.

### Issue: "Segmentation fault"
**Solution**: Use GDB to find line. Check pointer initialization.

## Next Steps

1. ✅ Understand buffer pool management
2. ✅ Understand LRU replacement
3. ✅ Understand page lifecycle
4. 📚 Read DEBUGGING_GUIDE.md for advanced debugging
5. 📚 Read COMPLETION_SUMMARY.md for implementation details
6. 🎯 Move to Phase 5: Transactions

## Key Files

- `include/page.h` - Page structure
- `include/lru.h` - LRU replacement policy
- `include/disk_manager.h` - Disk I/O operations
- `include/buffer_pool.h` - Buffer pool API
- `src/page.c` - Page implementation
- `src/lru.c` - LRU implementation
- `src/disk_manager.c` - Disk manager implementation
- `src/buffer_pool.c` - Buffer pool implementation
- `src/main.c` - Test program

## Learning Resources

- "Database Internals" by Alex Petrov
- "Architecture of a Database System"
- PostgreSQL Buffer Manager source code

---

**Happy Learning! 📚**
