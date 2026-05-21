# Database Internals Notes
# Serialization, stdio.h, fopen(), fread(), fwrite()

---

# Table of Contents

1. What is Serialization
2. What is Deserialization
3. Why Serialization is Important in Databases
4. Types of Serialization
5. Serialization Examples in C
6. Database Page Layout
7. WAL Serialization
8. Network Serialization
9. Introduction to stdio.h
10. FILE Structure
11. fopen()
12. fread()
13. fwrite()
14. fclose()
15. fseek()
16. File Buffering
17. System Calls
18. File Descriptors
19. Kernel IO Flow
20. Why Databases Use Low-Level IO
21. How SQLite/PostgreSQL Handle IO
22. Mini Database Recommendations

---

# 1. What is Serialization

Serialization is the process of:

> Converting objects or structured data in memory into a format that can be stored or transmitted.

The output can be:

- Binary
- JSON
- XML
- Protobuf
- Byte streams

---

# Example

## In Memory

```c
typedef struct {
    int id;
    char name[32];
} User;
```

Object:

```text
id = 1
name = "Alice"
```

---

# Serialized Binary

```text
01 00 00 00 41 6C 69 63 65 ...
```

This binary data can be:
- stored in files
- sent through sockets
- written into database pages
- written into WAL logs

---

# 2. What is Deserialization

Deserialization is the reverse process:

```text
Bytes -> Object
```

Example:

```text
01 00 00 00 41 6C 69 63 65
```

becomes:

```text
User {
    id = 1,
    name = "Alice"
}
```

---

# 3. Why Serialization is Important in Databases

Databases constantly:
- store data on disk
- read data from disk
- write WAL logs
- send data through network
- cache pages

All of these require serialization.

---

# Database INSERT Flow

```text
SQL INSERT
    |
    v
Row Object
    |
Serialization
    |
Binary Data
    |
Disk Page
```

---

# 4. Types of Serialization

# Binary Serialization

Example:

```text
01 00 00 00
```

Advantages:
- Fast
- Compact
- Efficient

Disadvantages:
- Harder to debug

---

# JSON Serialization

Example:

```json
{
  "id": 1,
  "name": "Alice"
}
```

Advantages:
- Human-readable

Disadvantages:
- Larger
- Slower

---

# Protocol Buffers

Used heavily in:
- gRPC
- Distributed systems
- Google infrastructure

---

# 5. Serialization Examples in C

# Struct Definition

```c
typedef struct {
    int id;
    char name[32];
} User;
```

---

# Serialize to File

```c
#include <stdio.h>

FILE *fp = fopen("user.bin", "wb");

User u = {1, "Alice"};

fwrite(&u, sizeof(User), 1, fp);

fclose(fp);
```

---

# Deserialize from File

```c
#include <stdio.h>

FILE *fp = fopen("user.bin", "rb");

User u;

fread(&u, sizeof(User), 1, fp);

printf("%d %s\n", u.id, u.name);

fclose(fp);
```

---

# 6. Database Page Layout

Databases store data in pages.

Example:

```text
+----------------------+
| Page Header          |
+----------------------+
| Row 1                |
| Row 2                |
| Row 3                |
+----------------------+
```

Rows are serialized into binary.

---

# Example Internal Row Layout

```text
| Row Header |
| Column Count |
| Null Bitmap |
| Variable Offsets |
| Data |
```

---

# 7. WAL Serialization

WAL = Write Ahead Logging

Example query:

```sql
UPDATE users SET name='Bob' WHERE id=1;
```

Serialized WAL entry:

```text
TX_ID=100
TABLE=users
ROW=1
OLD=Alice
NEW=Bob
```

The WAL is written before actual data pages.

---

# 8. Network Serialization

Client sends:

```sql
SELECT * FROM users;
```

Database serializes:
- query packets
- result rows
- metadata

into binary network packets.

---

# 9. Introduction to stdio.h

```c
#include <stdio.h>
```

`stdio.h` means:

> Standard Input Output Header

Provides:
- file operations
- console IO
- formatted printing
- buffered IO

---

# Important stdio.h Functions

| Function | Purpose |
|---|---|
| fopen() | Open file |
| fread() | Read binary |
| fwrite() | Write binary |
| fclose() | Close file |
| fseek() | Move file pointer |
| ftell() | Current position |
| rewind() | Go to file start |

---

# 10. FILE Structure

Example:

```c
FILE *fp;
```

`FILE` is an abstraction around:
- file descriptor
- buffering
- file position
- error state

---

# Simplified FILE Structure

```c
typedef struct {
    int fd;
    char *buffer;
    int position;
    int eof;
    int error;
} FILE;
```

Actual implementations are much more complex.

---

# 11. fopen()

# Example

```c
FILE *fp = fopen("data.db", "rb");
```

---

# What fopen() Does Internally

1. Allocates FILE structure
2. Parses mode (`rb`)
3. Calls OS `open()` syscall
4. Creates internal buffer
5. Returns FILE pointer

---

# Underneath

```c
int fd = open("data.db", O_RDONLY);
```

---

# File Open Modes

| Mode | Meaning |
|---|---|
| r | Read text |
| w | Write text |
| a | Append |
| rb | Read binary |
| wb | Write binary |
| ab | Append binary |

---

# 12. fread()

# Example

```c
fread(&user, sizeof(User), 1, fp);
```

---

# Internal Steps

1. Check FILE buffer
2. If empty:
   - call `read()` syscall
3. Copy buffer into user memory

---

# Simplified Internal Logic

```c
size_t fread(void *ptr,
             size_t size,
             size_t count,
             FILE *fp) {

    if (buffer_empty(fp)) {

        read(fp->fd,
             fp->buffer,
             BUFFER_SIZE);
    }

    memcpy(ptr,
           fp->buffer + fp->position,
           size * count);

    fp->position += size * count;
}
```

---

# 13. fwrite()

# Example

```c
fwrite(&user, sizeof(User), 1, fp);
```

---

# Internal Steps

1. Copy data into FILE buffer
2. If buffer full:
   - call `write()` syscall

---

# Simplified Internal Logic

```c
size_t fwrite(void *ptr,
              size_t size,
              size_t count,
              FILE *fp) {

    memcpy(fp->buffer + fp->position,
           ptr,
           size * count);

    fp->position += size * count;

    if (buffer_full(fp)) {

        write(fp->fd,
              fp->buffer,
              fp->position);
    }
}
```

---

# 14. fclose()

# Example

```c
fclose(fp);
```

---

# Internally

1. Flush remaining buffer
2. Close file descriptor
3. Free FILE memory

---

# Why Important?

Prevents:
- data corruption
- memory leaks
- incomplete writes

---

# 15. fseek()

# Example

```c
fseek(fp, 4096, SEEK_SET);
```

Moves the file pointer.

---

# Underneath

```c
lseek(fd, 4096, SEEK_SET);
```

---

# Databases Use This For

Jumping directly to pages:

```c
offset = page_id * PAGE_SIZE;
```

---

# 16. File Buffering

Without buffering:

```text
1 byte write
-> syscall
-> kernel
-> disk
```

Very slow.

---

# With Buffering

```text
Write many bytes
-> buffer
-> single large write
```

Much faster.

---

# 17. System Calls

stdio functions eventually call OS system calls.

| stdio | System Call |
|---|---|
| fopen() | open() |
| fread() | read() |
| fwrite() | write() |
| fseek() | lseek() |

---

# Example

```c
write(fd, buffer, size);
```

The kernel then handles actual disk IO.

---

# 18. File Descriptors

Linux example:

```text
0 -> stdin
1 -> stdout
2 -> stderr
3 -> file
```

Kernel internally tracks:
- open files
- offsets
- permissions

---

# 19. Kernel IO Flow

Example:

```c
fwrite(&user, sizeof(User), 1, fp);
```

Actual flow:

```text
User Struct
   |
memcpy
   |
stdio buffer
   |
write syscall
   |
kernel page cache
   |
filesystem
   |
disk driver
   |
SSD/HDD
```

---

# 20. Why Databases Use Low-Level IO

Real databases often avoid heavy use of:

```c
fread()
fwrite()
```

because they need:
- precise flushing
- direct page management
- async IO
- reduced buffering overhead

---

# Databases Prefer

```c
open()
pread()
pwrite()
mmap()
fsync()
```

---

# 21. How SQLite/PostgreSQL Handle IO

## SQLite

Uses:
- OS abstraction layer
- pager system
- low-level file handling

## PostgreSQL

Uses:
- shared buffers
- WAL flushing
- direct file/page management

---

# 22. Mini Database Recommendations

# Beginner Stage

Use:

```c
fopen()
fread()
fwrite()
```

because they are easier to learn.

---

# Intermediate Stage

Move to:

```c
open()
pread()
pwrite()
```

to understand:
- kernel IO
- page management
- real DBMS architecture

---

# Advanced Stage

Learn:
- mmap()
- async IO
- direct IO
- WAL optimization
- buffer pool systems

---

# Important Final Concept

`fread()` and `fwrite()` do NOT directly access the disk.

Instead:

```text
Your Program
   |
stdio buffer
   |
kernel syscall
   |
kernel page cache
   |
filesystem
   |
disk
```

The operating system controls the real disk operations.

---

# Related Libraries

| Library | Purpose |
|---|---|
| stdio.h | File IO |
| stdlib.h | malloc/free |
| string.h | memcpy/strcmp |
| stdint.h | uint32_t |
| unistd.h | OS syscalls |
| fcntl.h | open flags |
| sys/mman.h | mmap |

---

# Final Advice

If you want to deeply understand databases:

1. Start with:
   - fopen()
   - fread()
   - fwrite()

2. Then learn:
   - open()
   - read()
   - write()

3. Finally study:
   - mmap()
   - WAL systems
   - page cache
   - buffer pools
   - async IO

That progression matches how real database systems evolved internally.

---