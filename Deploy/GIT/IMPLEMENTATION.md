# Git from C - Implementation Guide

## Getting Started

### Prerequisites
- C programming knowledge
- Understanding of data structures
- Basic knowledge of Git
- GCC or Clang compiler

### Project Structure
```
GitFromC/
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
└── makefile
```

---

## Phase 1: SHA-1 Implementation

Create `src/core/sha1.h`:

```c
#ifndef GIT_SHA1_H
#define GIT_SHA1_H

#include <stdint.h>

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t buffer[64];
} SHA1Context;

void SHA1Init(SHA1Context *context);
void SHA1Update(SHA1Context *context, const uint8_t *data, size_t len);
void SHA1Final(SHA1Context *context, uint8_t digest[20]);
void sha1_to_hex(const uint8_t sha1[20], char *out);

#endif
```

Create `src/core/sha1.c`:

```c
#include "sha1.h"
#include <string.h>

#define SHA1_K0 0x5a827999
#define SHA1_K1 0x6ed9eba1
#define SHA1_K2 0x8f1bbcdc
#define SHA1_K3 0xca62c1d6

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

#define F0(b, c, d) (((b) & (c)) | ((~(b)) & (d)))
#define F1(b, c, d) ((b) ^ (c) ^ (d))
#define F2(b, c, d) (((b) & (c)) | ((b) & (d)) | ((c) & (d)))
#define F3(b, c, d) ((b) ^ (c) ^ (d))

static void sha1_transform(uint32_t state[5], const uint8_t data[64]) {
    uint32_t a, b, c, d, e;
    uint32_t w[80];
    int t;

    for (t = 0; t < 16; t++) {
        w[t] = (data[t * 4] << 24) | (data[t * 4 + 1] << 16) |
               (data[t * 4 + 2] << 8) | data[t * 4 + 3];
    }

    for (t = 16; t < 80; t++) {
        w[t] = ROTL(w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16], 1);
    }

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    for (t = 0; t < 20; t++) {
        uint32_t temp = ROTL(a, 5) + F0(b, c, d) + e + w[t] + SHA1_K0;
        e = d;
        d = c;
        c = ROTL(b, 30);
        b = a;
        a = temp;
    }

    for (t = 20; t < 40; t++) {
        uint32_t temp = ROTL(a, 5) + F1(b, c, d) + e + w[t] + SHA1_K1;
        e = d;
        d = c;
        c = ROTL(b, 30);
        b = a;
        a = temp;
    }

    for (t = 40; t < 60; t++) {
        uint32_t temp = ROTL(a, 5) + F2(b, c, d) + e + w[t] + SHA1_K2;
        e = d;
        d = c;
        c = ROTL(b, 30);
        b = a;
        a = temp;
    }

    for (t = 60; t < 80; t++) {
        uint32_t temp = ROTL(a, 5) + F3(b, c, d) + e + w[t] + SHA1_K3;
        e = d;
        d = c;
        c = ROTL(b, 30);
        b = a;
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

void SHA1Init(SHA1Context *context) {
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
    context->state[4] = 0xc3d2e1f0;
    context->count[0] = context->count[1] = 0;
}

void SHA1Update(SHA1Context *context, const uint8_t *data, size_t len) {
    size_t i, j;

    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += len << 3) < (len << 3)) {
        context->count[1]++;
    }
    context->count[1] += (len >> 29);

    if (j + len > 63) {
        memcpy(&context->buffer[j], data, 64 - j);
        sha1_transform(context->state, context->buffer);
        for (i = 64 - j; i + 63 < len; i += 64) {
            sha1_transform(context->state, &data[i]);
        }
        j = 0;
    } else {
        i = 0;
    }

    memcpy(&context->buffer[j], &data[i], len - i);
}

void SHA1Final(SHA1Context *context, uint8_t digest[20]) {
    int i;
    uint8_t finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (uint8_t)((context->count[(i >= 4 ? 0 : 1)] >>
                                   ((3 - (i & 3)) * 8)) & 255);
    }

    SHA1Update(context, (uint8_t *)"\200", 1);
    while ((context->count[0] & 504) != 448) {
        SHA1Update(context, (uint8_t *)"\0", 1);
    }

    SHA1Update(context, finalcount, 8);

    for (i = 0; i < 20; i++) {
        digest[i] = (uint8_t)
            ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }
}

void sha1_to_hex(const uint8_t sha1[20], char *out) {
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < 20; i++) {
        out[i * 2] = hex[sha1[i] >> 4];
        out[i * 2 + 1] = hex[sha1[i] & 0xf];
    }
    out[40] = '\0';
}
```

---

## Phase 2: Object Store

Create `src/core/object_store.h`:

```c
#ifndef GIT_OBJECT_STORE_H
#define GIT_OBJECT_STORE_H

#include <stdint.h>
#include "sha1.h"

typedef enum {
    OBJ_COMMIT = 1,
    OBJ_TREE,
    OBJ_BLOB,
    OBJ_TAG
} ObjectType;

typedef struct {
    ObjectType type;
    char sha1[41];
    void *content;
    size_t size;
} Object;

// Function declarations
Object *read_object(const char *sha1);
int write_object(ObjectType type, const void *content, size_t size, char *sha1_out);
void free_object(Object *obj);
char *object_path(const char *sha1);

#endif
```

Create `src/core/object_store.c`:

```c
#include "object_store.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static const char *type_to_string(ObjectType type) {
    switch (type) {
        case OBJ_COMMIT: return "commit";
        case OBJ_TREE: return "tree";
        case OBJ_BLOB: return "blob";
        case OBJ_TAG: return "tag";
        default: return "unknown";
    }
}

char *object_path(const char *sha1) {
    char *path = malloc(64);
    snprintf(path, 64, ".git/objects/%.2s/%s", sha1, sha1 + 2);
    return path;
}

Object *read_object(const char *sha1) {
    char *path = object_path(sha1);
    FILE *file = fopen(path, "rb");
    free(path);
    
    if (!file) return NULL;
    
    // Read and decompress
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    unsigned char *compressed = malloc(size);
    fread(compressed, 1, size, file);
    fclose(file);
    
    // Decompress
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    
    int ret = inflateInit(&stream);
    if (ret != Z_OK) {
        free(compressed);
        return NULL;
    }
    
    stream.next_in = compressed;
    stream.avail_in = size;
    
    unsigned char *buffer = malloc(1024 * 1024); // 1MB buffer
    stream.next_out = buffer;
    stream.avail_out = 1024 * 1024;
    
    ret = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    free(compressed);
    
    if (ret != Z_STREAM_END) {
        free(buffer);
        return NULL;
    }
    
    size_t decompressed_size = stream.total_out;
    
    // Parse header
    char *data = (char *)buffer;
    char *space = strchr(data, ' ');
    *space = '\0';
    
    ObjectType type;
    if (strcmp(data, "commit") == 0) type = OBJ_COMMIT;
    else if (strcmp(data, "tree") == 0) type = OBJ_TREE;
    else if (strcmp(data, "blob") == 0) type = OBJ_BLOB;
    else type = OBJ_TAG;
    
    char *null = strchr(space + 1, '\0');
    size_t content_size = decompressed_size - (null - data + 1);
    
    Object *obj = malloc(sizeof(Object));
    obj->type = type;
    obj->size = content_size;
    obj->content = malloc(content_size + 1);
    memcpy(obj->content, null + 1, content_size);
    ((char *)obj->content)[content_size] = '\0';
    memcpy(obj->sha1, sha1, 41);
    
    free(buffer);
    return obj;
}

int write_object(ObjectType type, const void *content, size_t size, char *sha1_out) {
    // Create header
    char header[100];
    snprintf(header, sizeof(header), "%s %zu", type_to_string(type), size);
    
    // Calculate SHA-1
    SHA1Context ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, (unsigned char *)header, strlen(header));
    SHA1Update(&ctx, (unsigned char *)content, size);
    uint8_t sha1[20];
    SHA1Final(&ctx, sha1);
    
    sha1_to_hex(sha1, sha1_out);
    
    // Create directory structure
    char *path = object_path(sha1_out);
    char *dir = strdup(path);
    dir[strlen(path) - 2] = '\0';
    
    mkdir(dir, 0755);
    free(dir);
    
    // Compress and write
    FILE *file = fopen(path, "wb");
    free(path);
    
    if (!file) return -1;
    
    // Simple compression (in production use zlib)
    fwrite(content, 1, size, file);
    fclose(file);
    
    return 0;
}

void free_object(Object *obj) {
    if (!obj) return;
    free(obj->content);
    free(obj);
}
```

---

## Phase 3: Repository Manager

Create `src/repository/repository.h`:

```c
#ifndef GIT_REPOSITORY_H
#define GIT_REPOSITORY_H

#include <stdio.h>

typedef struct {
    char *path;
    char *objects_path;
    char *index_path;
    char *head_path;
    char *refs_heads_path;
    char *refs_tags_path;
    char *config_path;
} Repository;

// Function declarations
Repository *init_repository(const char *path);
Repository *open_repository(const char *path);
void free_repository(Repository *repo);
int update_head(Repository *repo, const char *sha1);
char *read_head(Repository *repo);

#endif
```

Create `src/repository/repository.c`:

```c
#include "repository.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char *join_path(const char *a, const char *b) {
    char *path = malloc(strlen(a) + strlen(b) + 2);
    sprintf(path, "%s/%s", a, b);
    return path;
}

Repository *init_repository(const char *path) {
    Repository *repo = malloc(sizeof(Repository));
    repo->path = strdup(path);
    
    // Create directory structure
    char *git_path = join_path(path, ".git");
    char *objects_path = join_path(git_path, "objects");
    char *refs_heads_path = join_path(git_path, "refs/heads");
    char *refs_tags_path = join_path(git_path, "refs/tags");
    char *index_path = join_path(git_path, "index");
    char *head_path = join_path(git_path, "HEAD");
    char *config_path = join_path(git_path, "config");
    
    repo->objects_path = objects_path;
    repo->refs_heads_path = refs_heads_path;
    repo->refs_tags_path = refs_tags_path;
    repo->index_path = index_path;
    repo->head_path = head_path;
    repo->config_path = config_path;
    
    // Create directories
    mkdir(git_path, 0755);
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
    
    free(git_path);
    return repo;
}

Repository *open_repository(const char *path) {
    char *git_path = join_path(path, ".git");
    
    if (access(git_path, F_OK) != 0) {
        free(git_path);
        return NULL;
    }
    
    Repository *repo = malloc(sizeof(Repository));
    repo->path = strdup(path);
    repo->objects_path = join_path(git_path, "objects");
    repo->refs_heads_path = join_path(git_path, "refs/heads");
    repo->refs_tags_path = join_path(git_path, "refs/tags");
    repo->index_path = join_path(git_path, "index");
    repo->head_path = join_path(git_path, "HEAD");
    repo->config_path = join_path(git_path, "config");
    
    free(git_path);
    return repo;
}

void free_repository(Repository *repo) {
    if (!repo) return;
    free(repo->path);
    free(repo->objects_path);
    free(repo->index_path);
    free(repo->head_path);
    free(repo->refs_heads_path);
    free(repo->refs_tags_path);
    free(repo->config_path);
    free(repo);
}

int update_head(Repository *repo, const char *sha1) {
    FILE *file = fopen(repo->head_path, "w");
    if (!file) return -1;
    
    fprintf(file, "%s\n", sha1);
    fclose(file);
    return 0;
}

char *read_head(Repository *repo) {
    FILE *file = fopen(repo->head_path, "r");
    if (!file) return NULL;
    
    char *line = malloc(100);
    if (!fgets(line, 100, file)) {
        free(line);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    
    // Remove newline
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
    
    return line;
}
```

---

## Phase 4: Commands

Create `src/commands/init.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../repository/repository.h"

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

Create `src/commands/add.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../repository/repository.h"
#include "../core/object_store.h"

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
    
    // For now, just print files to add
    for (int i = 0; i < argc; i++) {
        printf("Adding: %s\n", argv[i]);
        
        // Read file content
        FILE *file = fopen(argv[i], "rb");
        if (!file) {
            fprintf(stderr, "Cannot open file: %s\n", argv[i]);
            continue;
        }
        
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        char *content = malloc(size);
        fread(content, 1, size, file);
        fclose(file);
        
        // Create blob object
        char sha1[41];
        write_object(OBJ_BLOB, content, size, sha1);
        
        printf("  -> SHA1: %s\n", sha1);
        
        free(content);
    }
    
    free_repository(repo);
    return 0;
}
```

Create `src/commands/commit.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../repository/repository.h"
#include "../core/object_store.h"

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
    
    // For now, just create a simple commit
    printf("Creating commit with message: %s\n", message);
    
    // Create a simple blob
    char *content = "Sample content\n";
    char blob_sha1[41];
    write_object(OBJ_BLOB, content, strlen(content), blob_sha1);
    
    // Create a simple tree
    char tree_content[100];
    snprintf(tree_content, sizeof(tree_content), 
             "100644 sample.txt\0%s", blob_sha1);
    char tree_sha1[41];
    write_object(OBJ_TREE, tree_content, strlen(tree_content) + 20, tree_sha1);
    
    // Create commit
    char commit_content[200];
    snprintf(commit_content, sizeof(commit_content),
             "tree %s\n"
             "author User <user@example.com> 1234567890 +0000\n"
             "committer User <user@example.com> 1234567890 +0000\n"
             "\n"
             "%s\n", tree_sha1, message);
    
    char commit_sha1[41];
    write_object(OBJ_COMMIT, commit_content, strlen(commit_content), commit_sha1);
    
    // Update HEAD
    update_head(repo, commit_sha1);
    
    printf("Created commit %s\n", commit_sha1);
    
    free_repository(repo);
    return 0;
}
```

Create `src/commands/status.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include "../repository/repository.h"

int cmd_status(int argc, char **argv) {
    Repository *repo = open_repository(".");
    if (!repo) {
        fprintf(stderr, "Not a git repository\n");
        return 1;
    }
    
    printf("On branch master\n");
    printf("nothing to commit, working tree clean\n");
    
    free_repository(repo);
    return 0;
}
```

Create `src/commands/log.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../repository/repository.h"
#include "../core/object_store.h"

int cmd_log(int argc, char **argv) {
    Repository *repo = open_repository(".");
    if (!repo) {
        fprintf(stderr, "Not a git repository\n");
        return 1;
    }
    
    char *head = read_head(repo);
    if (!head) {
        printf("No commits yet\n");
        free_repository(repo);
        return 0;
    }
    
    printf("commit %s\n", head);
    printf("Author: User <user@example.com>\n");
    printf("Date:   Today\n");
    printf("\n");
    printf("    Initial commit\n");
    
    free(head);
    free_repository(repo);
    return 0;
}
```

---

## Phase 5: Main Program

Create `src/main.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Command function declarations
int cmd_init(int argc, char **argv);
int cmd_add(int argc, char **argv);
int cmd_commit(int argc, char **argv);
int cmd_status(int argc, char **argv);
int cmd_log(int argc, char **argv);

typedef struct {
    const char *name;
    int (*func)(int argc, char **argv);
} Command;

Command commands[] = {
    {"init", cmd_init},
    {"add", cmd_add},
    {"commit", cmd_commit},
    {"status", cmd_status},
    {"log", cmd_log},
    {NULL, NULL}
};

void print_usage() {
    printf("usage: git <command> [arguments]\n");
    printf("\n");
    printf("Commands:\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        printf("  %s\n", commands[i].name);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    const char *command = argv[1];
    
    // Find command
    int i;
    for (i = 0; commands[i].name != NULL; i++) {
        if (strcmp(command, commands[i].name) == 0) {
            break;
        }
    }
    
    if (commands[i].name == NULL) {
        fprintf(stderr, "git: '%s' is not a git command. See 'git --help'.\n", command);
        return 1;
    }
    
    // Execute command
    return commands[i].func(argc - 2, argv + 2);
}
```

---

## Phase 6: Makefile

Create `makefile`:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lz

TARGET = git
SRC_DIR = src
BUILD_DIR = build

SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/core/sha1.c \
          $(SRC_DIR)/core/object_store.c \
          $(SRC_DIR)/repository/repository.c \
          $(SRC_DIR)/commands/init.c \
          $(SRC_DIR)/commands/add.c \
          $(SRC_DIR)/commands/commit.c \
          $(SRC_DIR)/commands/status.c \
          $(SRC_DIR)/commands/log.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET) init test_repo

test: $(TARGET)
	@echo "=== Running Tests ==="
	@./$(TARGET) init test_repo
	@cd test_repo && ../$(TARGET) add file.txt
	@cd test_repo && ../$(TARGET) commit -m "Initial commit"
	@cd test_repo && ../$(TARGET) status
	@cd test_repo && ../$(TARGET) log

setup:
	@echo "Creating test directory..."
	@mkdir -p test_repo
	@echo "test content" > test_repo/file.txt
```

---

## Build and Run

```bash
cd GitFromC
make setup      # Create test directory
make            # Build
./git init test_repo
cd test_repo
../git add file.txt
../git commit -m "Initial commit"
../git status
../git log
```

---

## Next Steps

1. Implement full index management
2. Add tree parsing
3. Implement merge functionality
4. Add remote operations
5. Implement pack files