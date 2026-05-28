# 🚀 Phase 7: Quick Start Guide

Get up and running with Phase 7 (Client-Server) in 5 minutes.

---

## Prerequisites

- GCC compiler
- Make build tool
- Valgrind (optional, for memory checking)
- GDB (optional, for debugging)

---

## Build

```bash
cd Database/Oracle/C/phase7
make
```

**Output:**
```
Compiling src/server.c...
Compiling src/client.c...
Compiling src/connection_pool.c...
Compiling src/protocol.c...
Compiling src/main.c...
Linking bin/client_server_test...
Build complete: bin/client_server_test
```

---

## Run Tests

```bash
make run
```

**Expected Output:**
```
╔════════════════════════════════════════════════════════════╗
║        Client-Server Test Suite - Phase 7                  ║
╚════════════════════════════════════════════════════════════╝

=== Test: Server Create/Destroy ===
✓ PASS: Server created
✓ PASS: Server port is 5432

... (more tests) ...

╔════════════════════════════════════════════════════════════╗
║                      TEST SUMMARY                           ║
╚════════════════════════════════════════════════════════════╝
Total Tests: 28
Passed: 28
Failed: 0

✓ All tests passed!
```

---

## Debug with GDB

```bash
make gdb
```

**In GDB:**
```
(gdb) break server_accept_connection
(gdb) run
(gdb) print server->conn_count
(gdb) continue
(gdb) quit
```

---

## Check Memory Leaks

```bash
make valgrind
```

**Expected Output:**
```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap alloc: X bytes in Y blocks
==12345==   total heap freed: X bytes in Y blocks
==12345==   total heap usage: Y allocs, Y frees, X bytes allocated
==12345== 
==12345== All heap blocks were freed -- no leaks are possible
```

---

## Clean Build Artifacts

```bash
make clean
```

---

## File Structure

```
phase7/
├── include/
│   ├── server.h              # Server API
│   ├── client.h              # Client API
│   ├── connection_pool.h     # Pool API
│   └── protocol.h            # Protocol API
├── src/
│   ├── server.c              # Server implementation
│   ├── client.c              # Client implementation
│   ├── connection_pool.c     # Pool implementation
│   ├── protocol.c            # Protocol implementation
│   └── main.c                # Test program
├── Makefile                  # Build configuration
├── README.md                 # Full documentation
└── QUICK_START.md            # This file
```

---

## Key Concepts

### TCP Server
Listen for client connections:
```c
DatabaseServer *server = server_create(5432);
server_start(server);
uint32_t conn_id = server_accept_connection(server);
server_destroy(server);
```

### Database Client
Connect and execute queries:
```c
DatabaseClient *client = client_create("localhost", 5432);
client_connect(client);
QueryResult result;
client_execute_query(client, "SELECT 1", &result);
client_destroy(client);
```

### Connection Pool
Manage reusable connections:
```c
ConnectionPool *pool = connection_pool_create("localhost", 5432, 5, 10);
DatabaseClient *client = connection_pool_get_connection(pool);
connection_pool_return_connection(pool, client);
connection_pool_destroy(pool);
```

### Network Protocol
Serialize/deserialize messages:
```c
Message *msg = protocol_create_query_message(1, 0, "SELECT 1");
char buffer[1024];
protocol_serialize_message(msg, buffer, sizeof(buffer));
protocol_destroy_message(msg);
```

---

## Test Coverage

- ✅ Server (6 tests)
  - Create/Destroy
  - Start/Stop
  - Accept connection
  - Close connection
  - Process request
  - Get connection count

- ✅ Client (6 tests)
  - Create/Destroy
  - Connect/Disconnect
  - Is connected
  - Execute query
  - Ping
  - Get query count

- ✅ Connection Pool (5 tests)
  - Create/Destroy
  - Get connection
  - Return connection
  - Get idle count
  - Get active count

- ✅ Protocol (6 tests)
  - Create message
  - Serialize message
  - Deserialize message
  - Create query message
  - Create result message
  - Create error message

- ✅ Integration Tests (3 tests)
  - Server-Client
  - Connection Pool
  - Protocol

**Total: 28 test cases**

---

## Common Commands

| Command | Purpose |
|---------|---------|
| `make` | Build the project |
| `make run` | Build and run tests |
| `make debug` | Build with debug symbols |
| `make valgrind` | Check for memory leaks |
| `make gdb` | Debug with GDB |
| `make clean` | Remove build artifacts |
| `make help` | Show help message |

---

## Troubleshooting

### Build fails
```bash
# Check compiler
gcc --version

# Check make
make --version

# Try clean build
make clean
make
```

### Tests fail
```bash
# Run with debug output
make debug
./bin/client_server_test

# Check with valgrind
make valgrind
```

### Memory leaks
```bash
# Run with valgrind
make valgrind

# Look for "definitely lost" or "indirectly lost"
# Check destroy() calls
```

---

## Next Steps

1. ✅ Understand TCP server
2. ✅ Understand client library
3. ✅ Understand connection pooling
4. ✅ Understand network protocol
5. Project complete!

---

## Learning Resources

- **Unix Network Programming** by Stevens & Rago
- **TCP/IP Illustrated** by Stevens
- Socket programming tutorials
- Protocol design patterns

---

**Happy Learning! 📚**
