# 🛠️ Environment Setup Guide

## Prerequisites

### Linux (Recommended)
- GCC or Clang compiler
- GNU Make
- Standard C library (glibc)
- Development tools

### macOS
- Xcode Command Line Tools
- Homebrew (optional)

### Windows
- WSL2 (Windows Subsystem for Linux)
- Or use MinGW/MSYS2

---

## Installation

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install gdb valgrind
sudo apt-get install apache2-utils  # for benchmarking
```

### macOS
```bash
xcode-select --install
brew install gcc make gdb valgrind
brew install wrk  # for load testing
```

### Verify Installation
```bash
gcc --version
make --version
gdb --version
```

---

## Project Setup

### 1. Create Directory Structure
```bash
cd web_server/language/C
mkdir -p phase{1..7}
mkdir -p tests tools docs
```

### 2. Create Base Makefile Template
Create `Makefile` in each phase directory:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lpthread

# Phase 1
phase1: phase1/01_basic_socket.c
	$(CC) $(CFLAGS) -o bin/phase1_server $< $(LDFLAGS)

# Phase 2
phase2: phase2/02_http_server.c phase2/http_parser.c phase2/http_response.c
	$(CC) $(CFLAGS) -o bin/phase2_server $^ $(LDFLAGS)

# ... continue for other phases

clean:
	rm -f bin/*
	rm -f *.o

run-phase1: phase1
	./bin/phase1_server

run-phase2: phase2
	./bin/phase2_server

.PHONY: clean run-phase1 run-phase2
```

### 3. Create bin Directory
```bash
mkdir -p bin
```

---

## Compilation Flags Explained

```bash
gcc [options] source.c -o output

# Common flags:
-Wall          # Enable all warnings
-Wextra        # Enable extra warnings
-O2            # Optimization level 2
-g             # Include debug symbols
-lpthread      # Link pthread library
-D_GNU_SOURCE  # Enable GNU extensions (for epoll)
```

---

## Debugging Tools

### GDB (GNU Debugger)
```bash
# Compile with debug symbols
gcc -g -o program program.c

# Run with gdb
gdb ./program

# Common gdb commands
(gdb) break main           # Set breakpoint
(gdb) run                  # Run program
(gdb) next                 # Next line
(gdb) step                 # Step into function
(gdb) print variable       # Print variable
(gdb) backtrace            # Show call stack
(gdb) quit                 # Exit gdb
```

### Valgrind (Memory Checker)
```bash
# Check for memory leaks
valgrind --leak-check=full ./program

# Check for invalid memory access
valgrind --track-origins=yes ./program

# Generate detailed report
valgrind --log-file=valgrind.log ./program
```

### Strace (System Call Tracer)
```bash
# Trace system calls
strace ./program

# Trace specific syscalls
strace -e trace=socket,bind,listen ./program

# Save output to file
strace -o trace.log ./program
```

---

## Testing Tools

### Apache Bench (ab)
```bash
# Simple benchmark
ab -n 1000 -c 10 http://localhost:8080/

# Options:
# -n: Total requests
# -c: Concurrent requests
# -t: Timeout in seconds
```

### wrk (Modern Load Tester)
```bash
# Install
git clone https://github.com/wg/wrk.git
cd wrk && make

# Run benchmark
./wrk -t4 -c100 -d30s http://localhost:8080/

# Options:
# -t: Number of threads
# -c: Number of connections
# -d: Duration
```

### curl (HTTP Client)
```bash
# Simple request
curl http://localhost:8080/

# With headers
curl -i http://localhost:8080/

# POST request
curl -X POST -d "data" http://localhost:8080/

# Verbose output
curl -v http://localhost:8080/
```

---

## Development Workflow

### 1. Write Code
```bash
# Edit source file
vim phase1/01_basic_socket.c
```

### 2. Compile
```bash
# Compile with debug symbols
gcc -Wall -Wextra -g -o bin/server phase1/01_basic_socket.c
```

### 3. Debug
```bash
# Run with gdb
gdb ./bin/server

# Or run with strace
strace ./bin/server
```

### 4. Test
```bash
# In one terminal, run server
./bin/server

# In another terminal, test
curl http://localhost:8080/
ab -n 100 -c 10 http://localhost:8080/
```

### 5. Check Memory
```bash
valgrind --leak-check=full ./bin/server
```

---

## Common Issues & Solutions

### Issue: "Address already in use"
```bash
# Find process using port 8080
lsof -i :8080

# Kill process
kill -9 <PID>

# Or use SO_REUSEADDR in code
int reuse = 1;
setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
```

### Issue: "Permission denied" on port < 1024
```bash
# Use port >= 1024 (recommended for development)
# Or run with sudo (not recommended)
sudo ./bin/server
```

### Issue: "Segmentation fault"
```bash
# Run with gdb to find location
gdb ./bin/server
(gdb) run
# Program will stop at crash location
(gdb) backtrace
```

### Issue: "Memory leak detected"
```bash
# Run with valgrind
valgrind --leak-check=full ./bin/server

# Fix: Ensure all malloc() calls have corresponding free()
```

---

## Quick Start Script

Create `start_dev.sh`:

```bash
#!/bin/bash

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== NGINX-like Server Development Setup ===${NC}"

# Check tools
echo -e "${BLUE}Checking tools...${NC}"
gcc --version > /dev/null && echo -e "${GREEN}✓ GCC${NC}" || echo "✗ GCC not found"
make --version > /dev/null && echo -e "${GREEN}✓ Make${NC}" || echo "✗ Make not found"
gdb --version > /dev/null && echo -e "${GREEN}✓ GDB${NC}" || echo "✗ GDB not found"

# Create directories
echo -e "${BLUE}Creating directories...${NC}"
mkdir -p bin
mkdir -p phase{1..7}
mkdir -p tests tools docs

echo -e "${GREEN}Setup complete!${NC}"
echo -e "${BLUE}Next steps:${NC}"
echo "1. Read LEARNING_SCENARIO.md"
echo "2. Start with Phase 1: phase1/01_basic_socket.c"
echo "3. Run: make -f phase1/Makefile"
echo "4. Test: ./bin/phase1_server"
```

Make it executable:
```bash
chmod +x start_dev.sh
./start_dev.sh
```

---

## IDE Setup

### VS Code
1. Install extensions:
   - C/C++ (Microsoft)
   - CodeLLDB (Vadim Chugunov)

2. Create `.vscode/launch.json`:
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Phase 1",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/bin/phase1_server",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build",
            "miDebuggerPath": "/usr/bin/gdb"
        }
    ]
}
```

3. Create `.vscode/tasks.json`:
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "make",
            "args": ["phase1"],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]
}
```

### CLion
- Open project folder
- CLion will auto-detect CMakeLists.txt or Makefile
- Configure run/debug configurations

---

## Performance Profiling

### Using perf
```bash
# Record performance data
perf record -g ./bin/server

# Generate report
perf report

# Flamegraph
perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg
```

### Using time
```bash
# Measure execution time
time ./bin/server

# real: actual elapsed time
# user: CPU time in user mode
# sys: CPU time in kernel mode
```

---

## Next Steps

1. ✅ Install required tools
2. ✅ Create directory structure
3. ✅ Set up your IDE
4. ✅ Read LEARNING_SCENARIO.md
5. ✅ Start Phase 1!

Happy coding! 🚀
