# ==========================================
# Go Compiler Makefile
# ==========================================

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = 

TARGET = goc
SRC_DIR = src
BUILD_DIR = build

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/lexer/token.c \
          $(SRC_DIR)/lexer/lexer.c \
          $(SRC_DIR)/parser/ast.c \
          $(SRC_DIR)/parser/parser.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run test setup

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
	./$(TARGET) examples/hello.go

# ==========================================
# Test
# ==========================================
test: $(TARGET)
	@echo "=== Running Go Compiler Tests ==="
	@echo ""
	@echo "1. Testing Hello World..."
	@./$(TARGET) examples/hello.go || echo "Hello test failed"
	@echo ""
	@echo "2. Testing Factorial..."
	@./$(TARGET) examples/factorial.go || echo "Factorial test failed"
	@echo ""
	@echo "=== Tests Complete ==="

# ==========================================
# Setup example files
# ==========================================
setup:
	@echo "Creating example Go files..."
	@mkdir -p examples
	@echo 'package main' > examples/hello.go
	@echo '' >> examples/hello.go
	@echo 'func main() {' >> examples/hello.go
	@echo '    println("Hello, Go Compiler!")' >> examples/hello.go
	@echo '}' >> examples/hello.go
	@echo 'package main' > examples/factorial.go
	@echo '' >> examples/factorial.go
	@echo 'func factorial(n int) int {' >> examples/factorial.go
	@echo '    if n <= 1 {' >> examples/factorial.go
	@echo '        return 1' >> examples/factorial.go
	@echo '    }' >> examples/factorial.go
	@echo '    return n * factorial(n-1)' >> examples/factorial.go
	@echo '}' >> examples/factorial.go
	@echo '' >> examples/factorial.go
	@echo 'func main() {' >> examples/factorial.go
	@echo '    println("5! =", factorial(5))' >> examples/factorial.go
	@echo '}' >> examples/factorial.go
	@echo "Example files created!"

# ==========================================
# Rebuild
# ==========================================
rebuild: clean all