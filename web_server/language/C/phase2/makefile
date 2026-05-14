# ==========================================
# File:
# web_server/language/C/phase2/makefile
# ==========================================

CC = gcc

CFLAGS = -Wall -Wextra -O2 -g

LDFLAGS = -lpthread

TARGET = server

SRC = http_parser.c http_response.c 02_http_server.c

# ==========================================
# Build
# ==========================================
build:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# ==========================================
# Run
# ==========================================
run: build
	./$(TARGET)

# ==========================================
# Clean
# ==========================================
clean:
	rm -f $(TARGET)

# ==========================================
# Rebuild
# ==========================================
rebuild: clean build

# ==========================================
# Create test directory
# ==========================================
setup:
	mkdir -p www
	echo '<!DOCTYPE html><html><head><title>HTTP Server</title></head><body><h1>Phase 2: HTTP Server</h1><p>Server is running!</p></body></html>' > www/index.html
	echo '<h1>404 Not Found</h1><p>The requested resource was not found.</p>' > www/404.html

# ==========================================
# Run with setup
# ==========================================
all: build setup run