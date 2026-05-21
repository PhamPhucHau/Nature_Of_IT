# Makefile for Phase 1: Key-Value Store
# 
# This Makefile builds the key-value store project with various options:
# - make: Build the project
# - make debug: Build with debug symbols
# - make run: Build and run tests
# - make clean: Remove build artifacts
# - make valgrind: Run with memory checking
# - make gdb: Run with debugger
#
# Author: Database Learning Project
# Date: May 2026

# ============================================================================
# COMPILER AND FLAGS
# ============================================================================

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99 -I./include

# Debug flags
DEBUG_FLAGS = -g -O0 -DDEBUG=1

# Optimization flags
OPT_FLAGS = -O2

# Default flags
CFLAGS += $(OPT_FLAGS)

# ============================================================================
# DIRECTORIES
# ============================================================================

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# ============================================================================
# SOURCE FILES
# ============================================================================

# Source files
SOURCES = $(SRC_DIR)/storage.c $(SRC_DIR)/serializer.c $(SRC_DIR)/main.c

# Object files
OBJECTS = $(BUILD_DIR)/storage.o $(BUILD_DIR)/serializer.o $(BUILD_DIR)/main.o

# Executable
EXECUTABLE = $(BIN_DIR)/storage_test

# ============================================================================
# TARGETS
# ============================================================================

# Default target
all: $(EXECUTABLE)

# Build executable
$(EXECUTABLE): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking $@..."
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build complete: $@"

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# ============================================================================
# SPECIAL TARGETS
# ============================================================================

# Debug build
debug: CFLAGS = -Wall -Wextra -std=c99 -I./include $(DEBUG_FLAGS)
debug: clean $(EXECUTABLE)
	@echo "Debug build complete"

# Run tests
run: $(EXECUTABLE)
	@echo "Running tests..."
	@./$(EXECUTABLE)

# Run with valgrind (memory checking)
valgrind: debug
	@echo "Running with valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(EXECUTABLE)

# Run with gdb (debugger)
gdb: debug
	@echo "Starting gdb..."
	gdb ./$(EXECUTABLE)

# Run with strace (system call tracing)
strace: $(EXECUTABLE)
	@echo "Running with strace..."
	strace -o strace.log ./$(EXECUTABLE)
	@echo "Trace saved to strace.log"

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@rm -f strace.log
	@rm -f test_persistence.db
	@echo "Clean complete"

# Show help
help:
	@echo "Available targets:"
	@echo "  make              - Build the project"
	@echo "  make debug        - Build with debug symbols"
	@echo "  make run          - Build and run tests"
	@echo "  make valgrind     - Run with memory checking"
	@echo "  make gdb          - Run with debugger"
	@echo "  make strace       - Run with system call tracing"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make help         - Show this help message"

# ============================================================================
# PHONY TARGETS
# ============================================================================

.PHONY: all debug run valgrind gdb strace clean help

# ============================================================================
# DEBUGGING TIPS
# ============================================================================

# To debug with GDB:
#   make gdb
#   (gdb) break storage_put
#   (gdb) run
#   (gdb) print key
#   (gdb) next
#   (gdb) quit
#
# To check for memory leaks:
#   make valgrind
#
# To trace system calls:
#   make strace
#   cat strace.log
#
# To see compiler warnings:
#   make clean
#   make CFLAGS="-Wall -Wextra -std=c99 -I./include -O2"
