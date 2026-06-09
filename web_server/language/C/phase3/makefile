# ==========================================
# File: web_server/language/C/phase3/makefile
# ==========================================
# Phase 3: Non-Blocking I/O with select()
# ==========================================

CC = gcc

CFLAGS = -Wall -Wextra -O2 -g

LDFLAGS = -lpthread

TARGET = server

SRC = connection.c 03_select_server.c

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
# Setup test files
# ==========================================
setup:
	mkdir -p www
	echo '<!DOCTYPE html><html><head><title>Phase 3 Server</title></head><body><h1>Phase 3: Non-Blocking I/O</h1><p>Server with select() multiplexing</p></body></html>' > www/index.html

# ==========================================
# Run with setup
# ==========================================
all: build setup run