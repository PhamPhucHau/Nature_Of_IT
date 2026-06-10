# Makefile for Human OS Phase 6 - Integrated System
#
# Build targets:
#   make         - Build the executable
#   make run     - Build and run
#   make clean   - Remove build artifacts
#   make rebuild - Clean and build

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/human_os.c
HEADERS = $(INC_DIR)/human_os.h
TARGET = $(BUILD_DIR)/phase6

# Default target
all: $(TARGET)

# Build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Main executable
$(TARGET): $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) $(SOURCES) $(LDFLAGS) -o $(TARGET)
	@echo "✅ Build successful: $(TARGET)"

# Run the program
run: $(TARGET)
	@echo "🚀 Running Human OS Phase 6..."
	@$(TARGET)

# Clean build artifacts
clean:
	@rm -rf $(BUILD_DIR)
	@echo "🧹 Cleaned build artifacts"

# Rebuild from scratch
rebuild: clean all

# Help
help:
	@echo "Human OS Phase 6 - Integrated System Build"
	@echo ""
	@echo "Available targets:"
	@echo "  make         - Build the executable"
	@echo "  make run     - Build and run"
	@echo "  make clean   - Remove build artifacts"
	@echo "  make rebuild - Clean and build"
	@echo "  make help    - Show this help"
	@echo ""

.PHONY: all run clean rebuild help
