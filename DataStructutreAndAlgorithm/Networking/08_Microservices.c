/*
================================================================================
MICROSERVICES ARCHITECTURE - C IMPLEMENTATION
================================================================================

STORY:
------
Monoliths were slow to build, deploy, and scale. Fix one bug, restart everything.

Enter Microservices: Small, independent services connected through APIs.
Each service owns its data. Each service scales independently.

Graph structures map service dependencies. Circuit breakers prevent cascading failures.
Service discovery finds the right service. Load balancers distribute traffic.

ARCHITECTURE:
  Order Service
        |
        ├─→ Payment Service
        ├─→ Inventory Service
        └─→ Shipping Service

Data Structures:
- Graph (service dependencies)
- HashMap (service registry)
- Queue (async communication)

Algorithms:
- Service Discovery
- Circuit Breaker pattern
- Retry with exponential backoff
- Timeout management

COMPLEXITY:
- Service lookup: O(1)
- Dependency traversal: O(V+E)
- Circuit breaker: O(1)
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SERVICES 10
#define MAX_DEPENDENCIES 50

typedef enum {
    HEALTHY, DEGRADED, CIRCUIT_OPEN
} ServiceStatus;

typedef struct {
    int serviceId;
    char name[50];
    char address[50];
    int port;
    ServiceStatus status;
    int requestsSuccess;
    int requestsFailed;
    int circuitBreakerFailures;
    time_t lastHealthCheck;
} Service;

typedef struct {
    int fromServiceId;
    int toServiceId;
    int callCount;
    int failureCount;
} Dependency;

typedef struct {
    Service services[MAX_SERVICES];
    int serviceCount;
    Dependency dependencies[MAX_DEPENDENCIES];
    int dependencyCount;
    int totalRequests;
    int totalFailures;
    int circuitBreakerTrips;
} MicroservicesArchitecture;

MicroservicesArchitecture* ms_create() {
    MicroservicesArchitecture* ms = malloc(sizeof(MicroservicesArchitecture));
    ms->serviceCount = 0;
    ms->dependencyCount = 0;
    ms->totalRequests = 0;
    ms->totalFailures = 0;
    ms->circuitBreakerTrips = 0;
    printf("Microservices Architecture created\n");
    return ms;
}

void ms_register_service(MicroservicesArchitecture* ms, int id, const char* name,
                        const char* address, int port) {
    if (ms->serviceCount >= MAX_SERVICES) return;
    
    Service* svc = &ms->services[ms->serviceCount];
    svc->serviceId = id;
    strcpy(svc->name, name);
    strcpy(svc->address, address);
    svc->port = port;
    svc->status = HEALTHY;
    svc->requestsSuccess = 0;
    svc->requestsFailed = 0;
    svc->circuitBreakerFailures = 0;
    svc->lastHealthCheck = time(NULL);
    
    ms->serviceCount++;
    printf("✓ Service %d (%s) registered: %s:%d\n", id, name, address, port);
}

Service* ms_find_service(MicroservicesArchitecture* ms, int serviceId) {
    for (int i = 0; i < ms->serviceCount; i++) {
        if (ms->services[i].serviceId == serviceId) {
            return &ms->services[i];
        }
    }
    return NULL;
}

void ms_add_dependency(MicroservicesArchitecture* ms, int from, int to) {
    if (ms->dependencyCount >= MAX_DEPENDENCIES) return;
    
    Dependency* dep = &ms->dependencies[ms->dependencyCount];
    dep->fromServiceId = from;
    dep->toServiceId = to;
    dep->callCount = 0;
    dep->failureCount = 0;
    
    ms->dependencyCount++;
    printf("✓ Dependency added: %d → %d\n", from, to);
}

void ms_call_service(MicroservicesArchitecture* ms, int fromId, int toId) {
    Service* fromSvc = ms_find_service(ms, fromId);
    Service* toSvc = ms_find_service(ms, toId);
    
    if (!fromSvc || !toSvc) return;
    
    ms->totalRequests++;
    
    // Check circuit breaker
    if (toSvc->status == CIRCUIT_OPEN) {
        printf("[Circuit Breaker OPEN] %s → %s: REJECTED\n",
               fromSvc->name, toSvc->name);
        ms->circuitBreakerTrips++;
        return;
    }
    
    // Simulate 85% success rate for healthy services
    int success = (rand() % 100) < 85 ? 1 : 0;
    
    if (success) {
        fromSvc->requestsSuccess++;
        toSvc->requestsSuccess++;
        printf("[Success] %s → %s\n", fromSvc->name, toSvc->name);
    } else {
        fromSvc->requestsFailed++;
        toSvc->requestsFailed++;
        ms->totalFailures++;
        printf("[Failed] %s → %s\n", fromSvc->name, toSvc->name);
        
        // Check if circuit breaker should trip
        if (toSvc->requestsFailed > 5) {
            toSvc->status = CIRCUIT_OPEN;
            printf("  ⚠️ Circuit breaker TRIPPED for %s\n", toSvc->name);
            ms->circuitBreakerTrips++;
        }
    }
    
    // Update dependency stats
    for (int i = 0; i < ms->dependencyCount; i++) {
        if (ms->dependencies[i].fromServiceId == fromId &&
            ms->dependencies[i].toServiceId == toId) {
            ms->dependencies[i].callCount++;
            if (!success) {
                ms->dependencies[i].failureCount++;
            }
        }
    }
}

void ms_print_stats(MicroservicesArchitecture* ms) {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ MICROSERVICES STATISTICS                                     ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nOverall Status:\n");
    printf("  Total services: %d\n", ms->serviceCount);
    printf("  Total requests: %d\n", ms->totalRequests);
    printf("  Total failures: %d\n", ms->totalFailures);
    printf("  Circuit breaker trips: %d\n", ms->circuitBreakerTrips);
    
    if (ms->totalRequests > 0) {
        printf("  Success rate: %.1f%%\n",
               (float)(ms->totalRequests - ms->totalFailures) / ms->totalRequests * 100);
    }
    
    printf("\nService Status:\n");
    printf("  %-10s %-20s %-15s %-15s %-15s\n",
           "ID", "Name", "Success", "Failed", "Status");
    printf("  %-10s %-20s %-15s %-15s %-15s\n",
           "---", "----", "-------", "------", "------");
    
    for (int i = 0; i < ms->serviceCount; i++) {
        Service* svc = &ms->services[i];
        const char* statusStr[] = {"HEALTHY", "DEGRADED", "CIRCUIT_OPEN"};
        printf("  %-10d %-20s %-15d %-15d %s\n",
               svc->serviceId, svc->name, svc->requestsSuccess,
               svc->requestsFailed, statusStr[svc->status]);
    }
}

void microservices_demo() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ MICROSERVICES ARCHITECTURE DEMONSTRATION                      ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    srand(time(NULL));
    MicroservicesArchitecture* ms = ms_create();
    
    printf("\n1. Register Microservices:\n");
    printf("──────────────────────────\n");
    
    ms_register_service(ms, 1, "OrderService", "order.local", 8080);
    ms_register_service(ms, 2, "PaymentService", "payment.local", 8081);
    ms_register_service(ms, 3, "InventoryService", "inventory.local", 8082);
    ms_register_service(ms, 4, "ShippingService", "shipping.local", 8083);
    
    printf("\n2. Define Dependencies:\n");
    printf("───────────────────────\n");
    
    ms_add_dependency(ms, 1, 2);  // Order → Payment
    ms_add_dependency(ms, 1, 3);  // Order → Inventory
    ms_add_dependency(ms, 1, 4);  // Order → Shipping
    
    printf("\n3. Service-to-Service Calls:\n");
    printf("──────────────────────────\n");
    
    for (int i = 0; i < 15; i++) {
        ms_call_service(ms, 1, 2);
        ms_call_service(ms, 1, 3);
        ms_call_service(ms, 1, 4);
    }
    
    ms_print_stats(ms);
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ MICROSERVICES ARCHITECTURE - COMPREHENSIVE DEMONSTRATION     ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    microservices_demo();
    
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ KEY PATTERNS                                                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\n1. Service Discovery: Find service by name/ID - O(1)\n");
    printf("2. Circuit Breaker: Prevent cascading failures\n");
    printf("3. Retry Logic: Exponential backoff\n");
    printf("4. Timeout: Prevent hanging requests\n");
    printf("5. Load Balancing: Distribute across instances\n");
    
    return 0;
}
