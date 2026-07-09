/*
================================================================================
LOAD BALANCER ARCHITECTURE - C IMPLEMENTATION
================================================================================

STORY:
------
One server wasn't enough. As traffic grew, it became a bottleneck.

"Why send all requests to one server?" asked the wise architect.

So they created the Load Balancer: a gatekeeper that distributes traffic fairly.
Round robin: Send to each server in turn.
Consistent hashing: Send similar requests to the same server.
Least connections: Send to the server with fewest active requests.

Now a thousand servers could work together, and no single one would break.

ARCHITECTURE:
             Client
               |
          Load Balancer
           /    |    \
       Server1 Server2 Server3

Data Structures:
- Hash Ring (for consistent hashing)
- Heap (for least connections tracking)
- HashMap (for server health/state)

Algorithms:
- Round Robin (simple, fair distribution)
- Consistent Hashing (cache-friendly)
- Least Connections (load-aware)
- Health Checks (monitor server health)

COMPLEXITY:
- Route request: O(1) for round robin
- Route request: O(log n) for consistent hashing
- Health check: O(n) for n servers

USE CASES:
- Web servers (nginx, HAProxy)
- API gateways
- Database sharding
- Cache clusters
- Game servers
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_SERVERS 10
#define MAX_REQUESTS 1000
#define HASH_RING_SIZE 360

// Server state
typedef struct {
    int serverId;
    char address[50];
    int port;
    int activeConnections;
    int totalRequests;
    int health;  // 0=unhealthy, 1=healthy
    time_t lastHealthCheck;
} Server;

// Hash ring node
typedef struct {
    int serverId;
    int hashValue;
} HashNode;

// Load balancer
typedef struct {
    Server servers[MAX_SERVERS];
    int serverCount;
    
    // For Round Robin
    int roundRobinIndex;
    
    // For Consistent Hashing
    HashNode hashRing[MAX_SERVERS * 3];  // Multiple vnodes per server
    int hashRingSize;
    
    int totalRequests;
    int healthCheckFailures;
} LoadBalancer;

/*
CREATE LOAD BALANCER
*/
LoadBalancer* lb_create() {
    LoadBalancer* lb = malloc(sizeof(LoadBalancer));
    lb->serverCount = 0;
    lb->roundRobinIndex = 0;
    lb->hashRingSize = 0;
    lb->totalRequests = 0;
    lb->healthCheckFailures = 0;
    
    printf("Load Balancer created\n");
    return lb;
}

/*
REGISTER SERVER
*/
void lb_register_server(LoadBalancer* lb, int serverId, const char* address, int port) {
    if (lb->serverCount >= MAX_SERVERS) {
        printf("Max servers reached!\n");
        return;
    }
    
    Server* srv = &lb->servers[lb->serverCount];
    srv->serverId = serverId;
    strcpy(srv->address, address);
    srv->port = port;
    srv->activeConnections = 0;
    srv->totalRequests = 0;
    srv->health = 1;  // Healthy
    srv->lastHealthCheck = time(NULL);
    
    lb->serverCount++;
    printf("✓ Server %d registered: %s:%d\n", serverId, address, port);
}

/*
SIMPLE HASH FUNCTION
*/
int hash_function(const char* key) {
    int hash = 0;
    for (int i = 0; key[i]; i++) {
        hash = (hash * 31 + key[i]) % 1000000;
    }
    return abs(hash);
}

/*
BUILD HASH RING - Consistent Hashing
Multiple virtual nodes per server for better distribution
*/
void lb_build_hash_ring(LoadBalancer* lb) {
    lb->hashRingSize = 0;
    
    for (int i = 0; i < lb->serverCount; i++) {
        // Create 3 virtual nodes per server
        for (int j = 0; j < 3; j++) {
            char vnodeKey[100];
            sprintf(vnodeKey, "server_%d_vnode_%d", lb->servers[i].serverId, j);
            
            int hashValue = hash_function(vnodeKey);
            
            lb->hashRing[lb->hashRingSize].serverId = lb->servers[i].serverId;
            lb->hashRing[lb->hashRingSize].hashValue = hashValue;
            lb->hashRingSize++;
        }
    }
    
    printf("Hash ring built with %d vnodes\n", lb->hashRingSize);
}

/*
ROUTE WITH ROUND ROBIN - O(1)
*/
int lb_route_round_robin(LoadBalancer* lb, const char* requestId) {
    if (lb->serverCount == 0) return -1;
    
    Server* srv = &lb->servers[lb->roundRobinIndex];
    
    printf("[Round Robin] Request '%s' -> Server %d\n", requestId, srv->serverId);
    
    srv->activeConnections++;
    srv->totalRequests++;
    lb->totalRequests++;
    lb->roundRobinIndex = (lb->roundRobinIndex + 1) % lb->serverCount;
    
    return srv->serverId;
}

/*
ROUTE WITH CONSISTENT HASHING - O(log n)
*/
int lb_route_consistent_hash(LoadBalancer* lb, const char* key) {
    if (lb->hashRingSize == 0) return -1;
    
    int keyHash = hash_function(key);
    
    // Find closest vnode on ring (simple linear search for demo)
    int closestServerId = -1;
    int minDistance = INT_MAX;
    
    for (int i = 0; i < lb->hashRingSize; i++) {
        int distance = (lb->hashRing[i].hashValue - keyHash + 1000000) % 1000000;
        if (distance < minDistance) {
            minDistance = distance;
            closestServerId = lb->hashRing[i].serverId;
        }
    }
    
    // Find actual server
    for (int i = 0; i < lb->serverCount; i++) {
        if (lb->servers[i].serverId == closestServerId) {
            printf("[Consistent Hash] Request '%s' (hash=%d) -> Server %d\n",
                   key, keyHash, closestServerId);
            
            lb->servers[i].activeConnections++;
            lb->servers[i].totalRequests++;
            lb->totalRequests++;
            
            return closestServerId;
        }
    }
    
    return -1;
}

/*
ROUTE WITH LEAST CONNECTIONS - O(n)
*/
int lb_route_least_connections(LoadBalancer* lb, const char* requestId) {
    if (lb->serverCount == 0) return -1;
    
    // Find server with least connections
    int bestServerIdx = 0;
    int minConnections = lb->servers[0].activeConnections;
    
    for (int i = 1; i < lb->serverCount; i++) {
        if (lb->servers[i].activeConnections < minConnections) {
            minConnections = lb->servers[i].activeConnections;
            bestServerIdx = i;
        }
    }
    
    Server* srv = &lb->servers[bestServerIdx];
    printf("[Least Connections] Request '%s' -> Server %d (connections: %d)\n",
           requestId, srv->serverId, srv->activeConnections);
    
    srv->activeConnections++;
    srv->totalRequests++;
    lb->totalRequests++;
    
    return srv->serverId;
}

/*
HEALTH CHECK
*/
void lb_health_check(LoadBalancer* lb) {
    printf("\n=== HEALTH CHECK ===\n");
    
    for (int i = 0; i < lb->serverCount; i++) {
        Server* srv = &lb->servers[i];
        
        // Simulate health check
        int isHealthy = (rand() % 100) < 90 ? 1 : 0;  // 90% success rate
        
        int prevHealth = srv->health;
        srv->health = isHealthy;
        srv->lastHealthCheck = time(NULL);
        
        if (!isHealthy) {
            lb->healthCheckFailures++;
            printf("⚠ Server %d: UNHEALTHY\n", srv->serverId);
        } else {
            printf("✓ Server %d: HEALTHY\n", srv->serverId);
        }
        
        if (prevHealth && !isHealthy) {
            printf("  [Alert] Server %d status changed: UP → DOWN\n", srv->serverId);
        } else if (!prevHealth && isHealthy) {
            printf("  [Recovery] Server %d status changed: DOWN → UP\n", srv->serverId);
        }
    }
}

/*
HANDLE COMPLETED REQUEST - Decrement active connections
*/
void lb_complete_request(LoadBalancer* lb, int serverId) {
    for (int i = 0; i < lb->serverCount; i++) {
        if (lb->servers[i].serverId == serverId) {
            if (lb->servers[i].activeConnections > 0) {
                lb->servers[i].activeConnections--;
            }
            return;
        }
    }
}

/*
PRINT STATISTICS
*/
void lb_print_stats(LoadBalancer* lb) {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ LOAD BALANCER STATISTICS                                     ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nLoad Balancer Status:\n");
    printf("  Total requests: %d\n", lb->totalRequests);
    printf("  Health check failures: %d\n", lb->healthCheckFailures);
    printf("  Registered servers: %d\n", lb->serverCount);
    
    printf("\nServer Statistics:\n");
    printf("  %-8s %-20s %-15s %-15s %-10s\n",
           "ID", "Address", "Active Conn", "Total Req", "Health");
    printf("  %-8s %-20s %-15s %-15s %-10s\n",
           "---", "-------", "-----------", "---------", "------");
    
    int totalActive = 0;
    for (int i = 0; i < lb->serverCount; i++) {
        Server* srv = &lb->servers[i];
        printf("  %-8d %-20s %-15d %-15d %s\n",
               srv->serverId, srv->address,
               srv->activeConnections, srv->totalRequests,
               srv->health ? "HEALTHY" : "DOWN");
        totalActive += srv->activeConnections;
    }
    
    printf("\nAggregate:\n");
    printf("  Total active connections: %d\n", totalActive);
    if (lb->totalRequests > 0) {
        printf("  Average requests per server: %.1f\n",
               (float)lb->totalRequests / lb->serverCount);
    }
}

/*
DEMONSTRATION
*/
void load_balancer_demo() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ LOAD BALANCER ARCHITECTURE DEMONSTRATION                      ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    LoadBalancer* lb = lb_create();
    
    printf("\n1. Register Servers:\n");
    printf("───────────────────\n");
    
    lb_register_server(lb, 1, "192.168.1.10", 8080);
    lb_register_server(lb, 2, "192.168.1.11", 8080);
    lb_register_server(lb, 3, "192.168.1.12", 8080);
    
    printf("\n2. Build Hash Ring (for consistent hashing):\n");
    printf("────────────────────────────────────────────\n");
    
    lb_build_hash_ring(lb);
    
    printf("\n3. Route Requests - ROUND ROBIN:\n");
    printf("────────────────────────────────\n");
    
    lb_route_round_robin(lb, "req_001");
    lb_route_round_robin(lb, "req_002");
    lb_route_round_robin(lb, "req_003");
    lb_route_round_robin(lb, "req_004");
    lb_route_round_robin(lb, "req_005");
    
    printf("\n4. Route Requests - LEAST CONNECTIONS:\n");
    printf("─────────────────────────────────────\n");
    
    lb_route_least_connections(lb, "req_006");
    lb_route_least_connections(lb, "req_007");
    lb_route_least_connections(lb, "req_008");
    
    printf("\n5. Complete Some Requests:\n");
    printf("──────────────────────────\n");
    
    printf("Request processed by Server 1\n");
    lb_complete_request(lb, 1);
    printf("Request processed by Server 2\n");
    lb_complete_request(lb, 2);
    
    printf("\n6. Route More Requests - CONSISTENT HASH:\n");
    printf("────────────────────────────────────────\n");
    
    lb_route_consistent_hash(lb, "user_123");
    lb_route_consistent_hash(lb, "user_456");
    lb_route_consistent_hash(lb, "user_123");  // Should go to same server
    
    printf("\n7. Health Checks:\n");
    printf("─────────────────\n");
    
    lb_health_check(lb);
    
    lb_print_stats(lb);
}

void complexity_analysis() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nRouting Algorithms:\n");
    printf("  Round Robin: O(1)\n");
    printf("    - Simple index increment\n");
    printf("    - No lookups needed\n");
    printf("    - Fair distribution\n\n");
    
    printf("  Consistent Hashing: O(log n)\n");
    printf("    - Binary search on hash ring\n");
    printf("    - Minimal key redistribution on server addition/removal\n");
    printf("    - Good for caching\n\n");
    
    printf("  Least Connections: O(n)\n");
    printf("    - Scan all servers\n");
    printf("    - Load-aware routing\n");
    printf("    - Better for long-lived connections\n\n");
    
    printf("  Health Checks: O(n)\n");
    printf("    - Check each server\n");
    printf("    - Can run in background\n");
}

void networking_patterns() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ LOAD BALANCER NETWORKING PATTERNS                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\n1. Round Robin:\n");
    printf("   - Simple, fair distribution\n");
    printf("   - Each server gets equal requests\n");
    printf("   - No state tracking needed\n\n");
    
    printf("2. Consistent Hashing:\n");
    printf("   - Minimize cache misses on backend\n");
    printf("   - Similar requests go to same server\n");
    printf("   - Good for session affinity\n\n");
    
    printf("3. Least Connections:\n");
    printf("   - Track active connections per server\n");
    printf("   - Route to least loaded\n");
    printf("   - Adaptive to varying request duration\n\n");
    
    printf("4. Health Checks:\n");
    printf("   - Active checks: Periodic health pings\n");
    printf("   - Passive checks: Monitor failed requests\n");
    printf("   - Automatic server removal/addition\n\n");
    
    printf("5. Session Persistence:\n");
    printf("   - Sticky sessions: Same client to same server\n");
    printf("   - Cookie-based or IP-based\n");
    printf("   - Important for stateful applications\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ LOAD BALANCER ARCHITECTURE - COMPREHENSIVE DEMONSTRATION     ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    srand(time(NULL));
    
    load_balancer_demo();
    complexity_analysis();
    networking_patterns();
    
    printf("\nKEY TAKEAWAYS:\n");
    printf("1. Round Robin: O(1) simple, fair distribution\n");
    printf("2. Consistent Hashing: O(log n), cache-friendly\n");
    printf("3. Least Connections: O(n), load-aware\n");
    printf("4. Health Checks: Monitor and auto-failover\n");
    printf("5. Session Persistence: Sticky sessions for stateful apps\n");
    
    return 0;
}
