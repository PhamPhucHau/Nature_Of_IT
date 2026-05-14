# Go to Docker Compiler Test Cases

## Overview

This document contains test cases for the Go to Docker compiler implementation.

---

## Test Categories

### 1. Parser Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TP-001 | Parse package decl | `package main` | NODE_PACKAGE_CLAUSE | ✅ |
| TP-002 | Parse import decl | `import "fmt"` | NODE_IMPORT_DECL | ✅ |
| TP-003 | Parse func decl | `func main()` | NODE_FUNCTION_DECL | ✅ |
| TP-004 | Parse var decl | `var x int` | NODE_VARIABLE_DECL | ✅ |
| TP-005 | Parse expression | `1 + 2` | NODE_BINARY_EXPR | ✅ |
| TP-006 | Parse return stmt | `return 42` | NODE_RETURN_STMT | ✅ |

### 2. Dependency Analysis Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TA-001 | Extract imports | `import "fmt"` | DEP_STANDARD | ✅ |
| TA-002 | Detect port | `:8080` | DEP_NETWORK | ✅ |
| TA-003 | Detect volume | `file` | DEP_VOLUME | ✅ |
| TA-004 | Multiple deps | Multiple imports | All detected | ✅ |

### 3. Dockerfile Generation Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TG-001 | Generate basic | Simple Go app | FROM, WORKDIR, COPY, RUN, CMD | ✅ |
| TG-002 | Generate with port | Port 8080 | EXPOSE 8080 | ✅ |
| TG-003 | Generate with volume | Volume detected | VOLUME ["/data"] | ✅ |
| TG-004 | Multi-stage build | Complex app | Builder stage + Runtime stage | ✅ |

### 4. Integration Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TI-001 | Hello World | hello.go | Valid Dockerfile | ✅ |
| TI-002 | Web Server | webserver.go | Dockerfile with EXPOSE | ✅ |
| TI-003 | Database App | database.go | Dockerfile with VOLUME | ✅ |
| TI-004 | Complex App | complex.go | Multi-stage Dockerfile | ✅ |

---

## Test Programs

### 1. Simple Hello World
```go
package main

import "fmt"

func main() {
    fmt.Println("Hello, Docker!")
}
```

**Expected Dockerfile**:
```dockerfile
FROM golang:1.21-alpine
WORKDIR /app
COPY . .
RUN go build -o main .
CMD ["./main"]
```

### 2. Web Server
```go
package main

import (
    "fmt"
    "net/http"
)

func handler(w http.ResponseWriter, r *http.Request) {
    fmt.Fprintf(w, "Hello, Docker!")
}

func main() {
    http.HandleFunc("/", handler)
    http.ListenAndServe(":8080", nil)
}
```

**Expected Dockerfile**:
```dockerfile
FROM golang:1.21-alpine
WORKDIR /app
EXPOSE 8080
COPY . .
RUN go build -o main .
CMD ["./main"]
```

### 3. Database Application
```go
package main

import (
    "database/sql"
    "fmt"
    _ "github.com/go-sql-driver/mysql"
)

func main() {
    db, err := sql.Open("mysql", "user:password@/dbname")
    if err != nil {
        panic(err)
    }
    defer db.Close()
    
    fmt.Println("Connected to database!")
}
```

**Expected Dockerfile**:
```dockerfile
FROM golang:1.21-alpine
WORKDIR /app
VOLUME ["/data"]
COPY . .
RUN go build -o main .
CMD ["./main"]
```

### 4. Multi-stage Build
```go
package main

import (
    "fmt"
    "os"
)

func main() {
    fmt.Println("Hello, Docker!")
    os.Exit(0)
}
```

**Expected Dockerfile**:
```dockerfile
FROM golang:1.21-alpine AS builder
WORKDIR /app
COPY . .
RUN go build -o main .

FROM alpine:latest
WORKDIR /app
COPY --from=builder /app/main .
CMD ["./main"]
```

---

## Test Execution

### Run All Tests
```bash
make test
```

### Run Specific Test
```bash
./g2d examples/hello.go
./g2d examples/webserver.go
```

### Test with GDB
```bash
gdb ./g2d
(gdb) run examples/hello.go
(gdb) break parse_function_decl
(gdb) run
```

### Test with Valgrind
```bash
valgrind ./g2d examples/hello.go
```

---

## Test Results Template

```
=== Test Results ===

Parser Tests
------------
TP-001: ✅ PASS
TP-002: ✅ PASS
TP-003: ✅ PASS

Dependency Analysis Tests
-------------------------
TA-001: ✅ PASS
TA-002: ✅ PASS

Dockerfile Generation Tests
---------------------------
TG-001: ✅ PASS
TG-002: ✅ PASS

Integration Tests
-----------------
TI-001: ✅ PASS
TI-002: ✅ PASS

Total: 10/10 tests passed
```