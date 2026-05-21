# 🗄️ Phase 1: Key-Value Store

## Goal
Build a simple key-value store with file persistence. This is the foundation for understanding how databases store and retrieve data.

---

## What You'll Learn

### Core Concepts
- **Binary File Formats** - How to store data in binary format
- **Data Serialization** - Converting data structures to bytes
- **File I/O in C** - Reading and writing files
- **Memory Management** - Allocating and freeing memory safely
- **Data Structures** - Hash tables and linked lists

### Practical Skills
- File operations (open, read, write, close)
- Binary data handling
- Error handling
- Memory debugging with valgrind
- Testing with simple assertions

---

## What You'll Build

### Features
- ✅ `put(key, value)` - Store a key-value pair
- ✅ `get(key)` - Retrieve a value by key
- ✅ `delete(key)` - Remove a key-value pair
- ✅ `save()` - Persist data to disk
- ✅ `load()` - Load data from disk
- ✅ `list()` - List all key-value pairs

### Data Structure
```
Key-Value Store
├── Hash Table (in-memory)
│   ├── Entry 1: key="user:1", value="Alice"
│   ├── Entry 2: key="user:2", value="Bob"
│   └── Entry 3: key="user:3", value="Charlie"
└── File Storage (persistent)
    └── Binary file with serialized entries
```

---

## File Structure

```
phase1/
├── include/
│   ├── storage.h          # Storage API
│   └── serializer.h       # Serialization functions
├── src/
│   ├── storage.c          # Storage implementation
│   ├── serializer.c       # Serializer implementation
│   └── main.c             # Test program
├── tests/
│   └── test_storage.c     # Unit tests
├── Makefile               # Build configuration
└── README.md              # This file
```

---

## Building and Running

### Build
```bash
cd web_server/language/C/phase1
make
```

### Run
```bash
./storage_test
```

### Run with Debugging
```bash
# With debug symbols
make debug

# With memory checking
valgrind --leak-check=full ./storage_test

# With GDB
gdb ./storage_test
```

### Clean
```bash
make clean
```

---

## Key Concepts Explained

### 1. Binary File Format
Instead of storing data as text, we store it as binary:
- More compact
- Faster to read/write
- Preserves data types

### 2. Serialization
Converting in-memory data to bytes:
```
In Memory:          Binary Format:
key = "user:1"  →   [4 bytes: length][4 bytes: "user:1"]
value = "Alice" →   [5 bytes: length][5 bytes: "Alice"]
```

### 3. Hash Table
Fast lookup structure:
```
Hash Function: hash("user:1") = 5
Bucket 5: [key="user:1", value="Alice"]
```

### 4. File I/O
Reading and writing to disk:
```
Memory ←→ File
put()  →  write to file
get()  ←  read from file
```

---

## Testing

### Unit Tests
```bash
make test
```

### Manual Testing
```bash
# Start the program
./storage_test

# It will:
# 1. Create a key-value store
# 2. Add some test data
# 3. Save to disk
# 4. Load from disk
# 5. Verify data integrity
```

### Memory Checking
```bash
valgrind --leak-check=full --show-leak-kinds=all ./storage_test
```

---

## Debugging Tips

### 1. Print Debug Information
The code includes debug macros:
```c
DEBUG_PRINT("Adding key: %s\n", key);
DEBUG_PRINT("Value: %s\n", value);
```

### 2. Enable Debug Mode
```bash
make debug
```

### 3. Use GDB
```bash
gdb ./storage_test
(gdb) break storage_put
(gdb) run
(gdb) print key
(gdb) next
```

### 4. Check Memory Leaks
```bash
valgrind --leak-check=full ./storage_test
```

### 5. Trace System Calls
```bash
strace ./storage_test
```

---

## Common Issues

### Issue: "Segmentation fault"
**Cause**: Accessing invalid memory
**Solution**: 
1. Use GDB to find the line
2. Check pointer initialization
3. Verify array bounds

### Issue: "Memory leak detected"
**Cause**: Not freeing allocated memory
**Solution**:
1. Use valgrind to find the leak
2. Add corresponding free() calls
3. Check error paths

### Issue: "File not found"
**Cause**: Wrong file path
**Solution**:
1. Check current directory
2. Use absolute paths
3. Create directory if needed

---

## Performance Expectations

### Throughput
- **Put operation**: ~1,000 ops/sec
- **Get operation**: ~10,000 ops/sec (hash table lookup)
- **Save operation**: ~100 KB/sec (file I/O)

### Memory Usage
- **Per entry**: ~100 bytes (key + value + overhead)
- **Hash table**: ~1 KB (for 100 entries)
- **Total**: ~10 KB for 100 entries

---

## Next Steps

After completing Phase 1:
1. ✅ Understand file I/O
2. ✅ Understand serialization
3. ✅ Understand hash tables
4. ✅ Move to Phase 2: B+ Tree Index

---

## Learning Resources

### File I/O in C
- `man fopen` - Open file
- `man fwrite` - Write to file
- `man fread` - Read from file
- `man fclose` - Close file

### Memory Management
- `man malloc` - Allocate memory
- `man free` - Free memory
- `man memcpy` - Copy memory

### Debugging
- `man gdb` - GNU Debugger
- `man valgrind` - Memory checker
- `man strace` - System call tracer

---

## Success Criteria

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] No memory leaks (valgrind clean)
- [ ] Can save and load data
- [ ] Handles errors gracefully
- [ ] Code is well-commented

---

**Happy Learning! 📚**
