# Git from C - Architecture Documentation

## Overview

This document describes the architecture of our simplified Git implementation.

---

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Git from C                             │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  Object      │  │   Index      │  │ Repository   │      │
│  │   Store      │  │   Manager    │  │   Manager    │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│         │                 │                  │               │
│         ▼                 ▼                  ▼               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  Objects     │  │  Index File  │  │  HEAD/Refs   │      │
│  │  (Blobs,     │  │  (Staging)   │  │  (Branches)  │      │
│  │   Trees,     │  │              │  │              │      │
│  │   Commits)   │  │              │  │              │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│                                      │                  │   │
│                              ┌───────┴──────────────────┘   │
│                              │                              │
│                      ┌───────▼────────┐                    │
│                      │   Commands     │                    │
│                      │   Layer        │                    │
│                      │                │                    │
│                      │ init, add,     │                    │
│                      │ commit, log,   │                    │
│                      │ status, etc.   │                    │
│                      └────────────────┘                    │
└─────────────────────────────────────────────────────────────┘
```

---

## Core Components

### 1. Object Store

**Purpose**: Store and retrieve Git objects

**Storage Structure**:
```
.git/objects/
├── XX/          # SHA-1 prefix (first 2 chars)
│   ├── YYYY...  # SHA-1 suffix (remaining 38 chars)
```

**Object Types**:
- **Blob**: File content
- **Tree**: Directory structure
- **Commit**: Snapshot with metadata

**Operations**:
- `write_object(type, content, len)` - Store object
- `read_object(sha1)` - Retrieve object
- `verify_object(sha1)` - Verify integrity

### 2. Index Manager

**Purpose**: Manage staging area

**File Format**:
```
Index File:
├── Header (12 bytes)
│   ├── Signature: "DIRC"
│   ├── Version
│   └── Entry count
├── Entries (variable)
│   ├── Entry 1
│   ├── Entry 2
│   └── ...
└── Checksum (SHA-1)
```

**Operations**:
- `read_index()` - Load index from file
- `write_index()` - Save index to file
- `add_entry()` - Add file to staging
- `remove_entry()` - Remove file from staging

### 3. Repository Manager

**Purpose**: Manage repository state

**Files**:
- `.git/HEAD` - Current branch reference
- `.git/refs/heads/` - Branch references
- `.git/refs/tags/` - Tag references
- `.git/config` - Repository configuration

**Operations**:
- `init_repository()` - Initialize repository
- `read_head()` - Get current HEAD
- `write_head()` - Update HEAD
- `resolve_ref()` - Resolve branch reference

---

## Data Structures

### Object Structure
```c
typedef enum {
    OBJ_COMMIT,
    OBJ_TREE,
    OBJ_BLOB,
    OBJ_TAG,
    OBJ_OFS_DELTA,
    OBJ_REF_DELTA
} ObjectType;

typedef struct {
    ObjectType type;
    char sha1[20];
    void *content;
    size_t size;
} Object;
```

### Index Entry Structure
```c
typedef struct {
    uint32_t ctime_seconds;
    uint32_t ctime_nanoseconds;
    uint32_t mtime_seconds;
    uint32_t mtime_nanoseconds;
    uint32_t dev;
    uint32_t ino;
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t size;
    char sha1[20];
    uint16_t flags;
    char name[];
} IndexEntry;
```

### Repository Structure
```c
typedef struct {
    char *path;
    char *objects_path;
    char *index_path;
    char *head_path;
    char *refs_heads_path;
    char *refs_tags_path;
    char *config_path;
} Repository;
```

---

## Object Store Implementation

### Object Writing
```c
int write_object(ObjectType type, const void *content, size_t size, char *sha1_out) {
    // Calculate SHA-1
    char header[100];
    snprintf(header, sizeof(header), "%s %zu", type_to_string(type), size);
    
    SHA1Context ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, (unsigned char *)header, strlen(header));
    SHA1Update(&ctx, (unsigned char *)content, size);
    SHA1Final(&ctx, sha1);
    
    // Create directory structure
    char dir[4];
    snprintf(dir, sizeof(dir), "%02x", sha1[0]);
    char *obj_path = object_path(sha1);
    
    // Write compressed object
    compress_and_write(obj_path, type, content, size);
    
    free(obj_path);
    return 0;
}
```

### Object Reading
```c
Object *read_object(const char *sha1) {
    char *obj_path = object_path(sha1);
    
    // Read and decompress
    unsigned char *content;
    size_t size;
    ObjectType type;
    
    read_and_decompress(obj_path, &content, &size, &type);
    
    Object *obj = malloc(sizeof(Object));
    obj->type = type;
    obj->size = size;
    obj->content = content;
    memcpy(obj->sha1, sha1, 20);
    
    free(obj_path);
    return obj;
}
```

---

## Index Manager Implementation

### Reading Index
```c
Index *read_index(Repository *repo) {
    FILE *file = fopen(repo->index_path, "rb");
    if (!file) return NULL;
    
    Index *index = malloc(sizeof(Index));
    index->entries = NULL;
    index->entry_count = 0;
    
    // Read header
    char signature[4];
    fread(signature, 1, 4, file);
    if (memcmp(signature, "DIRC", 4) != 0) {
        fclose(file);
        free(index);
        return NULL;
    }
    
    uint32_t version, entry_count;
    fread(&version, 4, 1, file);
    fread(&entry_count, 4, 1, file);
    
    // Read entries
    for (uint32_t i = 0; i < entry_count; i++) {
        IndexEntry *entry = read_entry(file);
        add_entry(index, entry);
    }
    
    fclose(file);
    return index;
}
```

### Writing Index
```c
int write_index(Index *index, Repository *repo) {
    FILE *file = fopen(repo->index_path, "wb");
    if (!file) return -1;
    
    // Write header
    fwrite("DIRC", 1, 4, file);
    uint32_t version = 2;
    fwrite(&version, 4, 1, file);
    fwrite(&index->entry_count, 4, 1, file);
    
    // Write entries
    for (uint32_t i = 0; i < index->entry_count; i++) {
        write_entry(file, index->entries[i]);
    }
    
    // Calculate and write checksum
    SHA1Context ctx;
    SHA1Init(&ctx);
    // ... update with file content
    SHA1Final(&ctx, checksum);
    fwrite(checksum, 1, 20, file);
    
    fclose(file);
    return 0;
}
```

---

## Repository Manager Implementation

### Repository Initialization
```c
Repository *init_repository(const char *path) {
    Repository *repo = malloc(sizeof(Repository));
    repo->path = strdup(path);
    
    // Create directory structure
    char *objects_path = join_path(path, ".git/objects");
    char *refs_heads_path = join_path(path, ".git/refs/heads");
    char *refs_tags_path = join_path(path, ".git/refs/tags");
    char *index_path = join_path(path, ".git/index");
    char *head_path = join_path(path, ".git/HEAD");
    char *config_path = join_path(path, ".git/config");
    
    repo->objects_path = objects_path;
    repo->refs_heads_path = refs_heads_path;
    repo->refs_tags_path = refs_tags_path;
    repo->index_path = index_path;
    repo->head_path = head_path;
    repo->config_path = config_path;
    
    // Create directories
    mkdir(objects_path, 0755);
    mkdir(refs_heads_path, 0755);
    mkdir(refs_tags_path, 0755);
    
    // Create HEAD file
    FILE *head_file = fopen(head_path, "w");
    fprintf(head_file, "ref: refs/heads/master\n");
    fclose(head_file);
    
    // Create config file
    FILE *config_file = fopen(config_path, "w");
    fprintf(config_file, "[core]\n");
    fprintf(config_file, "    repositoryformatversion = 0\n");
    fprintf(config_file, "    filemode = true\n");
    fclose(config_file);
    
    return repo;
}
```

---

## Command Implementation

### git init
```c
int cmd_init(int argc, char **argv) {
    const char *path = argc > 0 ? argv[0] : ".";
    
    Repository *repo = init_repository(path);
    if (!repo) {
        fprintf(stderr, "Failed to initialize repository\n");
        return 1;
    }
    
    printf("Initialized empty Git repository in %s/.git/\n", path);
    
    free_repository(repo);
    return 0;
}
```

### git add
```c
int cmd_add(int argc, char **argv) {
    if (argc < 1) {
        fprintf(stderr, "usage: git add <file>...\n");
        return 1;
    }
    
    Repository *repo = open_repository(".");
    if (!repo) {
        fprintf(stderr, "Not a git repository\n");
        return 1;
    }
    
    Index *index = read_index(repo);
    if (!index) {
        index = malloc(sizeof(Index));
        index->entries = NULL;
        index->entry_count = 0;
    }
    
    for (int i = 0; i < argc; i++) {
        add_file_to_index(repo, index, argv[i]);
    }
    
    write_index(index, repo);
    free_index(index);
    free_repository(repo);
    
    return 0;
}
```

### git commit
```c
int cmd_commit(int argc, char **argv) {
    char *message = NULL;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            message = argv[++i];
        }
    }
    
    if (!message) {
        fprintf(stderr, "usage: git commit -m <message>\n");
        return 1;
    }
    
    Repository *repo = open_repository(".");
    if (!repo) {
        fprintf(stderr, "Not a git repository\n");
        return 1;
    }
    
    Index *index = read_index(repo);
    if (!index) {
        fprintf(stderr, "No changes to commit\n");
        free_repository(repo);
        return 1;
    }
    
    // Write tree object
    char tree_sha1[41];
    write_tree(repo, index, tree_sha1);
    
    // Write commit object
    char commit_sha1[41];
    write_commit(repo, tree_sha1, message, commit_sha1);
    
    // Update HEAD
    update_head(repo, commit_sha1);
    
    free_index(index);
    free_repository(repo);
    
    printf("[%s] %s\n", commit_sha1, message);
    return 0;
}
```

---

## Memory Management

### Object Memory
```c
Object *create_object(ObjectType type, void *content, size_t size) {
    Object *obj = malloc(sizeof(Object));
    obj->type = type;
    obj->content = content;
    obj->size = size;
    return obj;
}

void free_object(Object *obj) {
    if (!obj) return;
    free(obj->content);
    free(obj);
}
```

### Index Memory
```c
Index *create_index() {
    Index *index = malloc(sizeof(Index));
    index->entries = NULL;
    index->entry_count = 0;
    return index;
}

void free_index(Index *index) {
    if (!index) return;
    for (uint32_t i = 0; i < index->entry_count; i++) {
        free(index->entries[i]);
    }
    free(index->entries);
    free(index);
}
```

---

## Error Handling

### Error Types
```c
typedef enum {
    ERROR_OBJECT,
    ERROR_INDEX,
    ERROR_REPOSITORY,
    ERROR_FILE,
    ERROR_MEMORY,
} ErrorType;

typedef struct {
    ErrorType type;
    char *message;
} GitError;
```

### Error Reporting
```c
void report_error(ErrorType type, const char *message) {
    fprintf(stderr, "Git Error [%s]: %s\n", error_type_names[type], message);
}
```

---

## Performance Considerations

### Optimization Strategies
1. **Object Caching**: Cache recently accessed objects
2. **Index Optimization**: Use memory-mapped files
3. **Delta Compression**: Store deltas for similar objects
4. **Pack Files**: Combine objects into pack files

### Current Limitations
- Single-threaded
- No pack files
- Simple compression
- Limited object types

---

## Future Enhancements

### Phase 2 Features
- Multi-threading support
- Pack files
- Delta compression
- Full object types

### Phase 3 Features
- Remote operations
- Merge support
- Rebase support
- Hooks

---

## References

- [Git Source Code](https://github.com/git/git)
- [Git Internals](https://git-scm.com/book/en/v2/Git-Internals-Git-Objects)
- [Pro Git Book](https://git-scm.com/book/en/v2)
- [Git Technical Docs](https://github.com/git/git/tree/master/Documentation/technical/)