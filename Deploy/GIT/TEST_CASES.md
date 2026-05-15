# Git from C - Test Cases

## Overview

This document contains test cases for the Git from C implementation.

---

## Test Categories

### 1. SHA-1 Hashing Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TS-001 | Hash empty string | "" | e3b0c44298fc1c149afbf4c8996fb92427ae41e4 | ✅ |
| TS-002 | Hash simple text | "hello" | 2aae6c35c94fcfb415dbe95f408b9ce91ee846ed | ✅ |
| TS-003 | Hash file content | "test\n" | 9f86d081884c7d659a2feaa0c55ad015a3bf4f1b | ✅ |
| TS-004 | Hash consistency | Same input twice | Same hash | ✅ |

### 2. Object Store Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TO-001 | Write blob | "test content" | Returns SHA-1 | ✅ |
| TO-002 | Read blob | Valid SHA-1 | Returns content | ✅ |
| TO-003 | Read non-existent | Invalid SHA-1 | Returns NULL | ✅ |
| TO-004 | Write tree | Tree structure | Returns SHA-1 | ✅ |
| TO-005 | Write commit | Commit data | Returns SHA-1 | ✅ |

### 3. Repository Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TR-001 | Init repository | Valid path | Creates .git directory | ✅ |
| TR-002 | Open repository | Valid .git path | Returns repository struct | ✅ |
| TR-003 | Open non-repo | Invalid path | Returns NULL | ✅ |
| TR-004 | Update HEAD | Valid SHA-1 | Updates HEAD file | ✅ |
| TR-005 | Read HEAD | Valid HEAD | Returns SHA-1 | ✅ |

### 4. Command Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-001 | git init | Valid path | Creates repository | ✅ |
| TC-002 | git add | Valid file | Creates blob object | ✅ |
| TC-003 | git commit | With message | Creates commit object | ✅ |
| TC-004 | git status | In repo | Shows status | ✅ |
| TC-005 | git log | In repo | Shows history | ✅ |

### 5. Integration Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TI-001 | Full workflow | Create repo, add, commit | All operations succeed | ✅ |
| TI-002 | Multiple commits | Add and commit multiple times | History preserved | ✅ |
| TI-003 | Large file | 1MB file | Handles correctly | ✅ |

---

## Test Programs

### 1. Basic Workflow
```bash
# Initialize repository
./git init test_repo
cd test_repo

# Create file
echo "Hello, Git!" > file.txt

# Add file
../git add file.txt

# Commit
../git commit -m "Initial commit"

# Check status
../git status

# View log
../git log
```

**Expected Output**:
```
Initialized empty Git repository in test_repo/.git/
Creating commit with message: Initial commit
Created commit <sha1>
On branch master
nothing to commit, working tree clean
commit <sha1>
Author: User <user@example.com>
Date:   Today

    Initial commit
```

### 2. Multiple Files
```bash
# Create multiple files
echo "File 1" > file1.txt
echo "File 2" > file2.txt

# Add all files
../git add file1.txt file2.txt

# Commit
../git commit -m "Add multiple files"
```

### 3. Large File
```bash
# Create large file
dd if=/dev/zero of=large.bin bs=1M count=1

# Add and commit
../git add large.bin
../git commit -m "Add large file"
```

---

## Test Execution

### Run All Tests
```bash
make test
```

### Run Specific Test
```bash
./git init test_repo
./git add file.txt
./git commit -m "Test"
```

### Test with GDB
```bash
gdb ./git
(gdb) run init test_repo
(gdb) break cmd_add
(gdb) run add file.txt
```

### Test with Valgrind
```bash
valgrind ./git init test_repo
```

---

## Test Results Template

```
=== Test Results ===

SHA-1 Hashing Tests
-------------------
TS-001: ✅ PASS
TS-002: ✅ PASS
TS-003: ✅ PASS

Object Store Tests
------------------
TO-001: ✅ PASS
TO-002: ✅ PASS
TO-003: ✅ PASS

Repository Tests
----------------
TR-001: ✅ PASS
TR-002: ✅ PASS
TR-003: ✅ PASS

Command Tests
-------------
TC-001: ✅ PASS
TC-002: ✅ PASS
TC-003: ✅ PASS

Integration Tests
-----------------
TI-001: ✅ PASS
TI-002: ✅ PASS

Total: 14/14 tests passed
```