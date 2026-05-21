# 🛠️ Environment Setup Guide

Complete guide for setting up your C++ database development environment.

---

## Prerequisites

### Linux (Recommended)
- GCC 9+ or Clang 10+
- CMake 3.15+
- Standard C++ library (libstdc++ or libc++)
- Development tools

### macOS
- Xcode Command Line Tools
- Homebrew (optional)
- CMake

### Windows
- WSL2 (Windows Subsystem for Linux)
- Or MinGW/MSVC

---

## Installation

### Ubuntu/Debian

```bash
# Update package manager
sudo apt-get update

# Install compiler and build tools
sudo apt-get install build-essential cmake git

# Install C++ development tools
sudo apt-get install g++ clang lldb

# Install debugging tools
sudo apt-get install gdb valgrind

# Install testing framework
sudo apt-get install libgtest-dev

# Install optional tools
sudo apt-get install sqlite3 postgresql
```

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install development tools
brew install cmake git gcc clang-format

# Install debugging tools
brew install gdb lldb valgrind

# Install testing framework
brew install googletest

# Install optional tools
brew install sqlite postgresql
```

### Windows (WSL2)

```bash
# In WSL2 terminal, run Ubuntu setup above
sudo apt-get update
sudo apt-get install build-essential cmake git g++ gdb valgrind
```

---

## Verify Installation

```bash
# Check compiler
g++ --version
clang++ --version

# Check CMake
cmake --version

# Check debugger
gdb --version

# Check valgrind
valgrind --version
```

---

## Project Setup

### 1. Create Directory Structure

```bash
cd Database/Oracle/C_Plus_Plus

# Create phase directories
mkdir -p phase{1..7}/{src,include,tests}

# Create common directories
mkdir -p {tests,tools,docs,build}

# Create CMakeLists.txt files
touch CMakeLists.txt
```

### 2. Create Root CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(MiniOracleDB)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -g")

# Add subdirectories
add_subdirectory(phase1)
add_subdirectory(phase2)
add_subdirectory(phase3)
add_subdirectory(phase4)
add_subdirectory(phase5)
add_subdirectory(phase6)
add_subdirectory(phase7)

# Enable testing
enable_testing()
add_subdirectory(tests)
```

### 3. Create Phase CMakeLists.txt Template

For each phase, create `phase{N}/CMakeLists.txt`:

```cmake
# Phase 1 CMakeLists.txt
add_library(phase1_lib
    src/storage.cpp
    src/serializer.cpp
)

target_include_directories(phase1_lib PUBLIC include)

# Executable
add_executable(phase1_test
    src/main.cpp
)

target_link_libraries(phase1_test phase1_lib)

# Tests
add_executable(phase1_unit_test
    tests/test_storage.cpp
)

target_link_libraries(phase1_unit_test phase1_lib gtest gtest_main)

add_test(NAME Phase1Tests COMMAND phase1_unit_test)
```

### 4. Build Project

```bash
cd Database/Oracle/C_Plus_Plus
mkdir -p build
cd build

# Configure
cmake ..

# Build
make

# Run tests
ctest --output-on-failure
```

---

## IDE Setup

### VS Code

1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools (Microsoft)
   - CodeLLDB (Vadim Chugunov)

2. Create `.vscode/settings.json`:

```json
{
    "C_Cpp.default.cppStandard": "c++17",
    "C_Cpp.default.cIntelliSenseEngine": "Tag Parser",
    "cmake.configureOnOpen": true,
    "cmake.buildDirectory": "${workspaceFolder}/build"
}
```

3. Create `.vscode/launch.json`:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Phase 1",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/phase1_test",
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

4. Create `.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "cd ${workspaceFolder}/build && cmake .. && make",
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]
}
```

### CLion

1. Open project folder
2. CLion will auto-detect CMakeLists.txt
3. Configure run/debug configurations
4. Build and run

### Visual Studio

1. Install C++ workload
2. Open folder
3. CMake will be auto-detected
4. Configure and build

---

## Debugging Tools

### GDB (GNU Debugger)

```bash
# Compile with debug symbols
g++ -g -o program program.cpp

# Run with gdb
gdb ./program

# Common commands
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

### GProf (Performance Profiler)

```bash
# Compile with profiling
g++ -pg -o program program.cpp

# Run program
./program

# Generate report
gprof program gmon.out > report.txt
```

---

## Testing Framework

### Google Test Setup

```bash
# Install Google Test
sudo apt-get install libgtest-dev

# Or build from source
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake ..
make
sudo make install
```

### Example Test

```cpp
#include <gtest/gtest.h>
#include "storage.h"

TEST(StorageTest, PutGet) {
    Storage storage;
    storage.put("key1", "value1");
    EXPECT_EQ(storage.get("key1"), "value1");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

---

## Build Commands

```bash
# Configure
cmake -B build -S .

# Build
cmake --build build

# Build specific target
cmake --build build --target phase1_test

# Run tests
ctest --test-dir build --output-on-failure

# Clean
cmake --build build --target clean

# Verbose build
cmake --build build --verbose
```

---

## Common Issues & Solutions

### Issue: "CMake not found"
```bash
# Install CMake
sudo apt-get install cmake
# or
brew install cmake
```

### Issue: "g++ not found"
```bash
# Install GCC
sudo apt-get install build-essential
# or
brew install gcc
```

### Issue: "gtest not found"
```bash
# Install Google Test
sudo apt-get install libgtest-dev
# or
brew install googletest
```

### Issue: "Permission denied"
```bash
# Make executable
chmod +x ./program
```

### Issue: "Segmentation fault"
```bash
# Run with gdb
gdb ./program
(gdb) run
# Program will stop at crash location
(gdb) backtrace
```

---

## Development Workflow

### 1. Write Code
```bash
vim phase1/src/storage.cpp
```

### 2. Compile
```bash
cd build
cmake ..
make
```

### 3. Debug
```bash
gdb ./phase1_test
```

### 4. Test
```bash
ctest --output-on-failure
```

### 5. Profile
```bash
valgrind --leak-check=full ./phase1_test
```

---

## Quick Start Script

Create `setup.sh`:

```bash
#!/bin/bash

echo "=== Mini Oracle DB Setup ==="

# Create directories
mkdir -p phase{1..7}/{src,include,tests}
mkdir -p {tests,tools,docs,build}

# Create build
cd build
cmake ..
make

echo "Setup complete!"
echo "Run: cd build && ctest --output-on-failure"
```

Make executable:
```bash
chmod +x setup.sh
./setup.sh
```

---

## Next Steps

1. ✅ Install required tools
2. ✅ Create project structure
3. ✅ Set up IDE
4. ✅ Build project
5. ✅ Run tests
6. ✅ Start Phase 1

---

## Useful Resources

- [CMake Documentation](https://cmake.org/documentation/)
- [GDB Documentation](https://www.gnu.org/software/gdb/documentation/)
- [Valgrind Documentation](https://valgrind.org/docs/manual/)
- [Google Test Documentation](https://google.github.io/googletest/)
- [C++ Reference](https://en.cppreference.com/)

---

**Setup Complete! Ready to start coding! 🚀**
