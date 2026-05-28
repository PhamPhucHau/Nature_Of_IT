# 🌐 Phase 7: Client-Server

## Goal
Implement a client-server architecture for remote database access. This phase teaches network programming concepts: TCP sockets, network protocols, connection pooling, and client-server communication.

---

## What You'll Learn

### Core Concepts
- **TCP Sockets** - Network communication
- **Client-Server** - Distributed architecture
- **Network Protocol** - Message format and exchange
- **Connection Pooling** - Resource management
- **Message Serialization** - Data encoding
- **Request-Response** - Communication pattern
- **Connection Management** - Lifecycle handling

### Practical Skills
- TCP server implementation
- Client library development
- Connection pool management
- Protocol design
- Message serialization/deserialization
- Network error handling

---

## What You'll Build

### Features
- ✅ TCP Server - Listen for connections
- ✅ Database Client - Connect and query
- ✅ Connection Pool - Reuse connections
- ✅ Network Protocol - Message format
- ✅ Request Handling - Process queries
- ✅ Response Generation - Return results
- ✅ Connection Management - Lifecycle

### Architecture
```
Client Application
    ↓
Database Client Library
├── Connection Management
├── Query Execution
└── Result Processing
    ↓
Network (TCP/IP)
    ↓
Database Server
├── Connection Handling
├── Request Processing
├── Query Execution
└── Response Generation
    ↓
Database Engine
├── Transaction Manager
├── Concurrency Control
├── Buffer Pool
└── Storage
```

---

## File Structure

```
phase7/
├── include/
│   ├── server.h              # TCP server
│   ├── client.h              # Database client
│   ├── connection_pool.h     # Connection pooling
│   └── protocol.h            # Network protocol
├── src/
│   ├── server.c              # Server implementation
│   ├── client.c              # Client implementation
│   ├── connection_pool.c     # Pool implementation
│   ├── protocol.c            # Protocol implementation
│   └── main.c                # Test program
├── Makefile                  # Build configuration
└── README.md                 # This file
```

---

## Building and Running

### Build
```bash
cd Database/Oracle/C/phase7
make
```

### Run
```bash
./bin/client_server_test
```

### Run with Debugging
```bash
# With debug symbols
make debug

# With memory checking
valgrind --leak-check=full ./bin/client_server_test

# With GDB
gdb ./bin/client_server_test
```

### Clean
```bash
make clean
```

---

## Key Concepts Explained

### 1. TCP Server
Listens for client connections:
```c
DatabaseServer *server = server_create(5432);
server_start(server);
uint32_t conn_id = server_accept_connection(server);
server_process_request(server, conn_id, &request, &response);
server_destroy(server);
```

**Server Lifecycle:**
1. Create server
2. Start listening
3. Accept connections
4. Process requests
5. Close connections
6. Stop server

### 2. Database Client
Connects to server and executes queries:
```c
DatabaseClient *client = client_create("localhost", 5432);
client_connect(client);
QueryResult result;
client_execute_query(client, "SELECT * FROM users", &result);
client_disconnect(client);
client_destroy(client);
```

**Client Lifecycle:**
1. Create client
2. Connect to server
3. Execute queries
4. Receive results
5. Disconnect
6. Destroy client

### 3. Connection Pool
Manages pool of reusable connections:
```c
ConnectionPool *pool = connection_pool_create("localhost", 5432, 5, 10);
DatabaseClient *client = connection_pool_get_connection(pool);
// Use client
connection_pool_return_connection(pool, client);
connection_pool_destroy(pool);
```

**Benefits:**
- Reuse connections
- Reduce overhead
- Improve performance
- Resource management

### 4. Network Protocol
Defines message format:
```c
Message *msg = protocol_create_query_message(1, 0, "SELECT 1");
char buffer[1024];
int size = protocol_serialize_message(msg, buffer, sizeof(buffer));
// Send buffer over network
Message received;
protocol_deserialize_message(buffer, size, &received);
```

**Message Types:**
- QUERY: Client query
- RESULT: Server result
- ERROR: Error message
- PING: Heartbeat
- PONG: Heartbeat response

---

## Testing

### Unit Tests
```bash
make run
```

### Manual Testing
```bash
./bin/client_server_test
```

### Memory Checking
```bash
valgrind --leak-check=full ./bin/client_server_test
```

### Debugging
```bash
gdb ./bin/client_server_test
(gdb) break server_accept_connection
(gdb) run
(gdb) print server->conn_count
```

---

## Common Issues

### Issue: "Connection refused"
**Cause**: Server not running or wrong port
**Solution**:
1. Check server is started
2. Verify port number
3. Check firewall

### Issue: "Connection timeout"
**Cause**: Server not responding
**Solution**:
1. Check server status
2. Verify network connectivity
3. Increase timeout

### Issue: "Protocol error"
**Cause**: Message format incorrect
**Solution**:
1. Check serialization
2. Verify message format
3. Check protocol version

### Issue: "Memory leak"
**Cause**: Connections not freed
**Solution**:
1. Use valgrind to find leak
2. Check destroy calls
3. Verify cleanup

---

## Performance Expectations

### Time Complexity
| Operation | Complexity | Notes |
|-----------|-----------|-------|
| Accept Connection | O(1) | Constant time |
| Process Request | O(1) | Constant time |
| Get Connection | O(n) | n = pool size |
| Return Connection | O(n) | n = pool size |
| Serialize Message | O(m) | m = message size |
| Deserialize Message | O(m) | m = message size |

### Space Complexity
| Component | Complexity | Notes |
|-----------|-----------|-------|
| Server | O(n) | n = connections |
| Client | O(1) | Constant |
| Connection Pool | O(n) | n = pool size |
| Protocol | O(m) | m = message size |

### Expected Performance
- **Connection Accept**: 1-10µs
- **Request Processing**: 10-100µs
- **Message Serialization**: 1-10µs
- **Throughput**: 1000-10000 requests/sec

---

## Next Steps

After completing Phase 7:
1. ✅ Understand TCP server
2. ✅ Understand client library
3. ✅ Understand connection pooling
4. ✅ Understand network protocol
5. Project complete!

---

## Learning Resources

### Network Programming
- "Unix Network Programming" by Stevens & Rago
- "TCP/IP Illustrated" by Stevens
- Socket programming tutorials

### Protocol Design
- Protocol buffers
- Message serialization
- Binary formats

### Connection Pooling
- Resource pooling patterns
- Connection lifecycle
- Performance optimization

---

## Success Criteria

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] No memory leaks (valgrind clean)
- [ ] Server works correctly
- [ ] Client works correctly
- [ ] Connection pool works
- [ ] Protocol works
- [ ] Error messages helpful
- [ ] Code is well-commented

---

**Happy Learning! 📚**
