# ==========================================
# JVM Makefile
# ==========================================

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = 

TARGET = jvm
SRC_DIR = src
BUILD_DIR = build

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/jvm/classfile/parser.c \
          $(SRC_DIR)/jvm/classfile/constant_pool.c \
          $(SRC_DIR)/jvm/runtime/memory.c \
          $(SRC_DIR)/jvm/runtime/stack.c \
          $(SRC_DIR)/jvm/interpreter/executor.c \
          $(SRC_DIR)/jvm/interpreter/instructions/load_store.c \
          $(SRC_DIR)/jvm/interpreter/instructions/arithmetic.c \
          $(SRC_DIR)/jvm/interpreter/instructions/control.c \
          $(SRC_DIR)/jvm/interpreter/instructions/object.c \
          $(SRC_DIR)/jvm/gc/mark_sweep.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run test

# ==========================================
# Build
# ==========================================
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ==========================================
# Clean
# ==========================================
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# ==========================================
# Run
# ==========================================
run: $(TARGET)
	./$(TARGET) examples/Hello.class

# ==========================================
# Test
# ==========================================
test: $(TARGET)
	@echo "=== Running JVM Tests ==="
	@echo ""
	@echo "1. Testing Class File Parser..."
	@./$(TARGET) examples/Hello.class || echo "Parser test failed"
	@echo ""
	@echo "2. Testing Calculator..."
	@./$(TARGET) examples/Calculator.class || echo "Calculator test failed"
	@echo ""
	@echo "=== Tests Complete ==="

# ==========================================
# Rebuild
# ==========================================
rebuild: clean all

# ==========================================
# Create example files
# ==========================================
setup:
	@echo "Creating example Java files..."
	@mkdir -p examples
	@echo 'public class Hello { public static void main(String[] args) { System.out.println("Hello, JVM!"); } }' > examples/Hello.java
	@echo 'public class Calculator { public static int add(int a, int b) { return a + b; } public static void main(String[] args) { int result = add(5, 3); System.out.println("5 + 3 = " + result); } }' > examples/Calculator.java
	@javac examples/Hello.java
	@javac examples/Calculator.java
	@echo "Example files created!"