# 🐛 Debugging Guide for Phase 1: Key-Value Store

This guide explains how to debug the key-value store implementation and understand what's happening at each step.

---

## 📋 Table of Contents

1. [Debug Output](#debug-output)
2. [Using GDB](#using-gdb)
3. [Memory Checking with Valgrind](#memory-checking-with-valgrind)
4. [System Call Tracing](#system-call-tracing)
5. [Common Issues](#common-issues)
6. [Performance Profiling](#performance-profiling)

---

## Debug Output

### Enabling Debug Output

The code includes debug macros that print detailed information about what's happening:

```c
DEBUG_PRINT("Adding key: %s\n", key);
DEBUG_PRINT("Value: %s\n", value);
```

To enable debug output, compile with debug mode:

```bash
make debug
```

### Understanding Debug Output

When you run the program with debug enabled, you'll see output like:

```
[DEBUG] storage.c:123: Creating storage with filename: test.db
[DEBUG] storage.c:145: Storage created successfully
[DEBUG] storage.c:200: Putting key='user:1', value='Alice'
[DEBUG] storage.c:220: Hashing key: user:1
[DEBUG] storage.c:230: Hash index: 5
[DEBUG] storage.c:250: Creating new entry
[DEBUG] storage.c:280: Entry created successfully, total entries: 1
```

### Debug Macros Explained

```c
/* Print debug information */
DEBUG_PRINT("Message: %s\n", variable);

/* Print error information */
ERROR_PRINT("Error: %s\n", error_message);
```

---

## Using GDB

### Starting GDB

```bash
# Build with debug symbols
make debug

# Start GDB
make gdb
```

Or manually:

```bash
gdb ./bin/storage_test
```

### Common GDB Commands

#### Setting Breakpoints

```gdb
# Break at function
(gdb) break storage_put
(gdb) break storage_get

# Break at line
(gdb) break storage.c:200

# Break at condition
(gdb) break storage.c:200 if strcmp(key, "user:1") == 0
```

#### Running and Stepping

```gdb
# Run program
(gdb) run

# Run with arguments
(gdb) run arg1 arg2

# Step to next line
(gdb) next

# Step into function
(gdb) step

# Continue execution
(gdb) continue

# Finish current function
(gdb) finish
```

#### Inspecting Variables

```gdb
# Print variable
(gdb) print key
(gdb) print value
(gdb) print store->entry_count

# Print with format
(gdb) print /x key        # Hexadecimal
(gdb) print /d key        # Decimal
(gdb) print /s key        # String

# Print memory
(gdb) x/10x store->table  # Print 10 hex values
(gdb) x/s key             # Print string at address
```

#### Stack Inspection

```gdb
# Show call stack
(gdb) backtrace

# Show frame info
(gdb) frame 0
(gdb) frame 1

# Move up/down stack
(gdb) up
(gdb) down

# Show local variables
(gdb) info locals

# Show arguments
(gdb) info args
```

### Example GDB Session

```bash
$ make gdb
$ (gdb) break storage_put
Breakpoint 1 at 0x1234: file storage.c, line 200.

$ (gdb) run
Starting program: ./bin/storage_test

Breakpoint 1, storage_put (store=0x..., key=0x..., value=0x...) at storage.c:200
200     if (!store || !key || !value) {

$ (gdb) print key
$1 = 0x... "user:1"

$ (gdb) print value
$2 = 0x... "Alice"

$ (gdb) next
201         ERROR_PRINT("Invalid arguments...");

$ (gdb) continue
Continuing.
...
```

---

## Memory Checking with Valgrind

### Running with Valgrind

```bash
# Check for memory leaks
make valgrind

# Or manually
valgrind --leak-check=full ./bin/storage_test
```

### Understanding Valgrind Output

```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap alloc: 1,234 bytes in 56 blocks
==12345==   total heap freed: 1,234 bytes in 56 blocks
==12345==   total heap waste: 0 bytes
==12345== 
==12345== LEAK SUMMARY:
==12345==    definitely lost: 0 bytes in 0 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==      possibly lost: 0 bytes in 0 blocks
==12345==    still reachable: 0 bytes in 0 blocks
==12345==         suppressed: 0 bytes in 0 blocks
```

### Interpreting Results

- **definitely lost**: Memory that was allocated but never freed (BUG!)
- **indirectly lost**: Memory lost due to lost pointers
- **possibly lost**: Memory that might be lost
- **still reachable**: Memory that's still referenced (usually OK)

### Common Memory Issues

#### Memory Leak Example

```c
// WRONG: Memory not freed
char *key = malloc(100);
strcpy(key, "user:1");
// key is never freed!
```

**Fix:**

```c
// CORRECT: Memory freed
char *key = malloc(100);
strcpy(key, "user:1");
// ... use key ...
free(key);
```

#### Use-After-Free Example

```c
// WRONG: Using memory after free
char *value = malloc(100);
strcpy(value, "Alice");
free(value);
printf("%s\n", value);  // BUG: value is freed!
```

**Fix:**

```c
// CORRECT: Don't use after free
char *value = malloc(100);
strcpy(value, "Alice");
printf("%s\n", value);
free(value);
```

---

## System Call Tracing

### Running with strace

```bash
# Trace system calls
make strace

# Or manually
strace -o strace.log ./bin/storage_test
cat strace.log
```

### Understanding strace Output

```
open("test.db", O_WRONLY|O_CREAT|O_TRUNC, 0666) = 3
write(3, "\x4b\x56\x42\x00", 4)                = 4
write(3, "\x01\x00\x00\x00", 4)                = 4
write(3, "\x03\x00\x00\x00", 4)                = 4
...
close(3)                                        = 0
```

This shows:
1. File opened for writing
2. Magic number written (0x4b5642 = "KVB")
3. Version written (0x01)
4. Entry count written (0x03)
5. File closed

---

## Common Issues

### Issue 1: Segmentation Fault

**Symptom:**
```
Segmentation fault (core dumped)
```

**Debugging:**

```bash
# Run with GDB
make gdb
(gdb) run
# Program crashes
(gdb) backtrace
# Shows where it crashed
```

**Common Causes:**
- Null pointer dereference
- Array out of bounds
- Use-after-free

**Example:**

```c
// WRONG: Null pointer dereference
Storage *store = NULL;
storage_put(store, "key", "value");  // CRASH!

// CORRECT: Check for NULL
if (store == NULL) {
    fprintf(stderr, "Storage is NULL\n");
    return -1;
}
```

### Issue 2: Memory Leak

**Symptom:**
```
definitely lost: 1,234 bytes in 56 blocks
```

**Debugging:**

```bash
# Run with valgrind
make valgrind
# Look for "definitely lost"
```

**Common Causes:**
- Forgot to free allocated memory
- Exception path doesn't free memory

**Example:**

```c
// WRONG: Memory leak on error
char *key = malloc(100);
if (strlen(key) > MAX_KEY_LENGTH) {
    return -1;  // key not freed!
}
free(key);

// CORRECT: Free before returning
char *key = malloc(100);
if (strlen(key) > MAX_KEY_LENGTH) {
    free(key);
    return -1;
}
free(key);
```

### Issue 3: File Not Found

**Symptom:**
```
[ERROR] Failed to open file for reading: test.db
```

**Debugging:**

```bash
# Check current directory
pwd

# List files
ls -la

# Check file permissions
ls -la test.db
```

**Solution:**
- Make sure you're in the right directory
- Check file permissions
- Use absolute paths

### Issue 4: Incorrect Data

**Symptom:**
```
Expected: Alice
Actual:   Bob
```

**Debugging:**

```bash
# Enable debug output
make debug
./bin/storage_test

# Look for where data changed
# Use GDB to step through code
make gdb
(gdb) break storage_put
(gdb) run
```

---

## Performance Profiling

### Using time Command

```bash
# Measure execution time
time ./bin/storage_test

# Output:
# real    0m0.123s
# user    0m0.100s
# sys     0m0.023s
```

### Using gprof

```bash
# Compile with profiling
gcc -pg -o storage_test storage.c serializer.c main.c

# Run program
./storage_test

# Generate report
gprof storage_test gmon.out > report.txt
cat report.txt
```

### Profiling Output Example

```
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  ms/call  ms/call  name    
 50.0      0.050    0.050   10000     0.005     0.005  hash_function
 30.0      0.080    0.030    5000     0.006     0.010  storage_put
 20.0      0.100    0.020    5000     0.004     0.004  storage_get
```

This shows:
- `hash_function` takes 50% of time
- `storage_put` takes 30% of time
- `storage_get` takes 20% of time

---

## Debugging Checklist

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] No memory leaks (valgrind clean)
- [ ] No segmentation faults
- [ ] Correct output format
- [ ] Error handling works
- [ ] Performance acceptable
- [ ] Code is well-commented

---

## Tips and Tricks

### 1. Add Print Statements

```c
printf("DEBUG: key=%s, value=%s\n", key, value);
```

### 2. Use Assertions

```c
assert(store != NULL);
assert(key != NULL);
assert(strlen(key) < MAX_KEY_LENGTH);
```

### 3. Check Return Values

```c
int result = storage_put(store, key, value);
if (result != 0) {
    fprintf(stderr, "storage_put failed\n");
}
```

### 4. Use Debugger Breakpoints

```gdb
(gdb) break storage.c:200 if strcmp(key, "user:1") == 0
```

### 5. Inspect Memory

```gdb
(gdb) x/100x store->table
```

---

## Resources

- [GDB Manual](https://sourceware.org/gdb/documentation/)
- [Valgrind Manual](https://valgrind.org/docs/manual/)
- [strace Manual](https://man7.org/linux/man-pages/man1/strace.1.html)
- [C Debugging](https://www.gnu.org/software/gdb/documentation/)

---

**Happy Debugging! 🐛**
