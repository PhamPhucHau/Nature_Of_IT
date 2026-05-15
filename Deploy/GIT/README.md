# Git from C - Learning Path & Implementation

## Overview

This project implements a simplified Git version control system from scratch in C. The implementation covers:
- Object storage (blobs, trees, commits)
- SHA-1 hashing
- Index file management
- Branch management
- Basic Git commands (init, add, commit, log, status)

---

## Learning Path

### Phase 1: Git Fundamentals (Week 1)

#### Topics
1. **Git Architecture**
   - Object database (objects/ directory)
   - Index file (.git/index)
   - HEAD reference
   - Branches

2. **Git Object Types**
   - Blob: File content
   - Tree: Directory structure
   - Commit: Snapshot with metadata

3. **SHA-1 Hashing**
   - Object identification
   - Content-addressable storage

#### Resources
- [Pro Git Book](https://git-scm.com/book/en/v2)
- [Git Internals](https://git-scm.com/book/en/v2/Git-Internals-Git-Objects)

### Phase 2: Core Data Structures (Week 2)

#### Implementation Steps

1. **Object Storage**
   - Create .git/objects directory
   - Implement SHA-1 hashing
   - Store and retrieve objects

2. **Index Management**
   - Parse and write index file
   - Track staged changes

3. **Repository Structure**
   - Initialize repository
   - Manage HEAD and branches

### Phase 3: Git Commands (Week 3)

#### Topics
1. **Basic Commands**
   - git init
   - git add
   - git commit
   - git status
   - git log

2. **Advanced Commands**
   - git branch
   - git checkout
   - git diff

### Phase 4: Advanced Features (Week 4)

#### Topics
1. **Merge and Rebase**
2. **Remote Operations**
3. **Pack Files**
4. **Hooks**

---

## Project Structure

```
GitFromC/
├── README.md                 # This file
├── LEARNING_PATH.md          # Detailed learning guide
├── ARCHITECTURE.md           # Git architecture documentation
├── IMPLEMENTATION.md         # Implementation guide
├── makefile
├── src/
│   ├── core/
│   │   ├── object_store.c
│   │   ├── object_store.h
│   │   ├── sha1.c
│   │   └── sha1.h
│   ├── index/
│   │   ├── index.c
│   │   └── index.h
│   ├── repository/
│   │   ├── repository.c
│   │   └── repository.h
│   ├── commands/
│   │   ├── init.c
│   │   ├── add.c
│   │   ├── commit.c
│   │   ├── status.c
│   │   └── log.c
│   └── main.c
├── test/
│   ├── test_object_store.c
│   ├── test_index.c
│   └── test_repository.c
└── examples/
    └── sample_project/
```

---

## Quick Start

### Prerequisites
- C programming knowledge
- Understanding of data structures
- GCC or Clang compiler

### Build
```bash
cd GitFromC
make
```

### Run
```bash
# Initialize a repository
./git init

# Add files
./git add file.txt

# Commit changes
./git commit -m "Initial commit"

# View history
./git log
```

---

## Learning Resources

### Books
1. **"Pro Git"** - Scott Chacon
2. **"Git Internals"** - Peff
3. **"The Git Book"** - GitHub

### Online Resources
1. [Git Source Code](https://github.com/git/git)
2. [Git Internals](https://git-scm.com/book/en/v2/Git-Internals-Git-Objects)
3. [How Git Works](https://github.com/git/git/blob/master/Documentation/technical/)

---

## Implementation Checklist

### Phase 1: Core Structure
- [ ] Object storage implementation
- [ ] SHA-1 hashing
- [ ] Repository initialization

### Phase 2: Index Management
- [ ] Index file parsing
- [ ] Staging changes
- [ ] Index updates

### Phase 3: Commands
- [ ] git init
- [ ] git add
- [ ] git commit
- [ ] git status
- [ ] git log

### Phase 4: Testing
- [ ] Unit tests
- [ ] Integration tests
- [ ] Performance tests

---

## Next Steps

1. Start with Phase 1 learning materials
2. Implement object storage
3. Build SHA-1 hashing
4. Create index management
5. Implement basic commands
6. Add advanced features