# Git from C - Learning Path - Detailed Guide

## Week 1: Git Fundamentals

### Day 1-2: Git Architecture

#### Topics
1. **Git Directory Structure**
   - .git/objects - Object database
   - .git/refs - Branch references
   - .git/HEAD - Current branch reference
   - .git/index - Staging area
   - .git/config - Repository configuration

2. **Object Database**
   - Objects directory structure
   - SHA-1 hash as filename
   - Compressed object storage

#### Exercises
- Create .git directory structure
- Understand object storage layout
- Explore existing Git repositories

### Day 3-4: Git Object Types

#### Topics
1. **Blob Objects**
   - File content storage
   - SHA-1 hash of content
   - Size and type header

2. **Tree Objects**
   - Directory structure
   - References to blobs and subtrees
   - Mode, name, SHA-1 format

3. **Commit Objects**
   - Snapshot metadata
   - Parent commits
   - Tree reference
   - Commit message

#### Exercises
- Create sample blobs
- Build tree structures
- Create commit objects

### Day 5-7: SHA-1 Hashing

#### Topics
1. **SHA-1 Algorithm**
   - Message digest
   - 160-bit hash
   - Block processing

2. **Git Object Hashing**
   - Type and size prefix
   - Null byte separator
   - Content hashing

#### Exercises
- Implement SHA-1
- Test with sample data
- Verify Git object hashes

---

## Week 2: Core Data Structures

### Day 1-2: Object Store

#### Topics
1. **Object Storage Design**
   - Directory structure (objects/XX/)
   - SHA-1 prefix splitting
   - Object lookup

2. **Object Operations**
   - Write object
   - Read object
   - Verify object

#### Exercises
- Implement object store
- Test read/write operations
- Handle edge cases

### Day 3-4: Index File

#### Topics
1. **Index Format**
   - Header structure
   - Entry format
   - Checksum

2. **Index Operations**
   - Read index
   - Write index
   - Update entries

#### Exercises
- Parse index file
- Build index from scratch
- Handle index updates

### Day 5-7: Repository

#### Topics
1. **Repository Structure**
   - Initialization
   - Configuration
   - State management

2. **HEAD Management**
   - Detached HEAD
   - Branch references
   - Symbolic references

#### Exercises
- Implement repository init
- Manage HEAD state
- Handle branch references

---

## Week 3: Git Commands

### Day 1-2: git init

#### Topics
1. **Repository Initialization**
   - Create .git directory
   - Initialize objects directory
   - Create HEAD file
   - Set up default branch

#### Exercises
- Implement git init
- Test with different paths
- Handle errors

### Day 3-4: git add

#### Topics
1. **Staging Changes**
   - Read file content
   - Create blob object
   - Update index
   - Handle file removal

#### Exercises
- Implement git add
- Test with multiple files
- Handle large files

### Day 5-7: git commit

#### Topics
1. **Creating Commits**
   - Write tree object
   - Create commit object
   - Update HEAD
   - Update parent references

#### Exercises
- Implement git commit
- Test with messages
- Handle multiple parents

---

## Week 4: Advanced Commands

### Day 1-2: git status

#### Topics
1. **Status Calculation**
   - Compare index to working tree
   - Compare index to HEAD
   - Untracked files

#### Exercises
- Implement git status
- Handle different file states
- Color output

### Day 3-4: git log

#### Topics
1. **History Display**
   - Traverse commit history
   - Format output
   - Handle branches

#### Exercises
- Implement git log
- Test with complex history
- Add options

### Day 5-7: git branch

#### Topics
1. **Branch Management**
   - List branches
   - Create branches
   - Delete branches
   - Checkout branches

#### Exercises
- Implement git branch
- Test branch operations
- Handle edge cases

---

## Week 5: Testing & Optimization

### Day 1-2: Testing

#### Topics
1. **Test Strategy**
   - Unit tests
   - Integration tests
   - Comparison with Git

#### Exercises
- Write test suite
- Compare outputs
- Fix bugs

### Day 3-4: Performance

#### Topics
1. **Optimization**
   - Object caching
   - Index optimization
   - Memory management

#### Exercises
- Profile code
- Optimize hot paths
- Measure improvements

### Day 5-6: Final Project

#### Topics
1. **Complete Implementation**
   - All commands working
   - Error handling
   - Documentation

#### Exercises
- Final integration
- Documentation
- Demo

---

## Additional Resources

### Code Examples
- [Git Source Code](https://github.com/git/git)
- [Simple Git](https://github.com/rust-lang/simple-git)
- [Git Tutorial](https://github.com/git/git/tree/master/Documentation)

### Tools
- `git cat-file` - Inspect Git objects
- `git hash-object` - Calculate SHA-1
- `objdump` - Analyze binaries
- `gdb` - Debug C code

### Practice Projects
1. Build a file versioning system
2. Create a backup tool
3. Implement a sync utility

---

## Assessment

### Phase 1 (Week 1)
- [ ] Understand Git architecture
- [ ] Know object types
- [ ] Implement SHA-1

### Phase 2 (Week 2)
- [ ] Implement object store
- [ ] Build index management
- [ ] Create repository

### Phase 3 (Week 3)
- [ ] Implement git init
- [ ] Implement git add
- [ ] Implement git commit

### Phase 4 (Week 4)
- [ ] Implement git status
- [ ] Implement git log
- [ ] Implement git branch

### Phase 5 (Week 5)
- [ ] Pass all tests
- [ ] Document code
- [ ] Optimize performance