# ==========================================
# JavaScript Compiler Makefile
# ==========================================

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
LDFLAGS = 

TARGET = jsc
SRC_DIR = src
BUILD_DIR = build

# Source files
SOURCES = $(SRC_DIR)/main.cc \
          $(SRC_DIR)/lexer/token.cc \
          $(SRC_DIR)/parser/ast.cc \
          $(SRC_DIR)/parser/parser.cc

OBJECTS = $(SOURCES:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)

.PHONY: all clean run test setup

# ==========================================
# Build
# ==========================================
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

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
	./$(TARGET) examples/hello.js

# ==========================================
# Test
# ==========================================
test: $(TARGET)
	@echo "=== Running JavaScript Compiler Tests ==="
	@echo ""
	@echo "1. Testing Hello World..."
	@./$(TARGET) examples/hello.js || echo "Hello test failed"
	@echo ""
	@echo "2. Testing Factorial..."
	@./$(TARGET) examples/factorial.js || echo "Factorial test failed"
	@echo ""
	@echo "=== Tests Complete ==="

# ==========================================
# Setup example files
# ==========================================
setup:
	@echo "Creating example JavaScript files..."
	@mkdir -p examples
	@echo 'console.log("Hello, JavaScript Compiler!");' > examples/hello.js
	@echo 'function factorial(n) {' > examples/factorial.js
	@echo '    if (n <= 1) {' >> examples/factorial.js
	@echo '        return 1;' >> examples/factorial.js
	@echo '    }' >> examples/factorial.js
	@echo '    return n * factorial(n - 1);' >> examples/factorial.js
	@echo '}' >> examples/factorial.js
	@echo 'console.log("5! =", factorial(5));' >> examples/factorial.js
	@echo "Example files created!"

# ==========================================
# Rebuild
# ==========================================
rebuild: clean all