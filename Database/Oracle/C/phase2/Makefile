# Makefile for Phase 2: B+ Tree Index
# 
# Build targets:
# - make: Build the project
# - make debug: Build with debug symbols
# - make run: Build and run tests
# - make valgrind: Run with memory checking
# - make gdb: Run with debugger
# - make clean: Remove build artifacts
#
# Author: Database Learning Project
# Date: May 2026

# ============================================================================
# COMPILER AND FLAGS
# ============================================================================

CC = gcc

CFLAGS = -Wall -Wextra -std=c99 -I./include

DEBUG_FLAGS = -g -O0 -DDEBUG=1

OPT_FLAGS = -O2

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

SOURCES = $(SRC_DIR)/btree.c $(SRC_DIR)/main.c

OBJECTS = $(BUILD_DIR)/btree.o $(BUILD_DIR)/main.o

EXECUTABLE = $(BIN_DIR)/btree_test

# ============================================================================
# TARGETS
# ============================================================================

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking $@..."
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build complete: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# ============================================================================
# SPECIAL TARGETS
# ============================================================================

debug: CFLAGS = -Wall -Wextra -std=c99 -I./include $(DEBUG_FLAGS)
debug: clean $(EXECUTABLE)
	@echo "Debug build complete"

run: $(EXECUTABLE)
	@echo "Running tests..."
	@./$(EXECUTABLE)

valgrind: debug
	@echo "Running with valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXECUTABLE)

gdb: debug
	@echo "Starting gdb..."
	gdb ./$(EXECUTABLE)

clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

help:
	@echo "Available targets:"
	@echo "  make              - Build the project"
	@echo "  make debug        - Build with debug symbols"
	@echo "  make run          - Build and run tests"
	@echo "  make valgrind     - Run with memory checking"
	@echo "  make gdb          - Run with debugger"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make help         - Show this help message"

.PHONY: all debug run valgrind gdb clean help
