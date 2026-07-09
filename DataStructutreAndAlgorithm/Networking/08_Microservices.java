import java.util.*;

/**
 * MICROSERVICES ARCHITECTURE - JAVA IMPLEMENTATION
 */

public class Microservices {
    
    enum Status { HEALTHY, DEGRADED, CIRCUIT_OPEN }
    
    static class Service {
        int serviceId;
        String name;
        String address;
        int port;
        Status status;
        int requestsSuccess;
        int requestsFailed;
        
        Service(int id, String name, String addr, int port) {
            this.serviceId = id;
            this.name = name;
            this.address = addr;
            this.port = port;
            this.status = Status.HEALTHY;
            this.requestsSuccess = 0;
            this.requestsFailed = 0;
        }
    }
    
    static class Dependency {
        int fromServiceId;
        int toServiceId;
        int callCount;
        int failureCount;
        
        Dependency(int from, int to) {
            this.fromServiceId = from;
            this.toServiceId = to;
            this.callCount = 0;
            this.failureCount = 0;
        }
    }
    
    static class MS {
        Map<Integer, Service> services;
        List<Dependency> dependencies;
        int totalRequests;
        int totalFailures;
        int circuitBreakerTrips;
        Random random;
        
        MS() {
            this.services = new HashMap<>();
            this.dependencies = new ArrayList<>();
            this.totalRequests = 0;
            this.totalFailures = 0;
            this.circuitBreakerTrips = 0;
            this.random = new Random();
            System.out.println("Microservices Architecture created");
        }
        
        void registerService(int id, String name, String addr, int port) {
            services.put(id, new Service(id, name, addr, port));
            System.out.println("✓ Service " + id + " (" + name + ") registered: " + 
                             addr + ":" + port);
        }
        
        void addDependency(int from, int to) {
            dependencies.add(new Dependency(from, to));
            System.out.println("✓ Dependency added: " + from + " → " + to);
        }
        
        void callService(int fromId, int toId) {
            Service fromSvc = services.get(fromId);
            Service toSvc = services.get(toId);
            
            if (fromSvc == null || toSvc == null) return;
            
            totalRequests++;
            
            if (toSvc.status == Status.CIRCUIT_OPEN) {
                System.out.println("[Circuit Breaker OPEN] " + fromSvc.name + 
                                 " → " + toSvc.name + ": REJECTED");
                circuitBreakerTrips++;
                return;
            }
            
            boolean success = random.nextInt(100) < 85;
            
            if (success) {
                fromSvc.requestsSuccess++;
                toSvc.requestsSuccess++;
                System.out.println("[Success] " + fromSvc.name + " → " + toSvc.name);
            } else {
                fromSvc.requestsFailed++;
                toSvc.requestsFailed++;
                totalFailures++;
                System.out.println("[Failed] " + fromSvc.name + " → " + toSvc.name);
                
                if (toSvc.requestsFailed > 5) {
                    toSvc.status = Status.CIRCUIT_OPEN;
                    System.out.println("  ⚠️ Circuit breaker TRIPPED for " + toSvc.name);
                    circuitBreakerTrips++;
                }
            }
            
            for (Dependency dep : dependencies) {
                if (dep.fromServiceId == fromId && dep.toServiceId == toId) {
                    dep.callCount++;
                    if (!success) dep.failureCount++;
                }
            }
        }
        
        void printStats() {
            System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
            System.out.println("║ MICROSERVICES STATISTICS                                     ║");
            System.out.println("╚════════════════════════════════════════════════════════════════╝");
            
            System.out.println("\nOverall Status:");
            System.out.println("  Total services: " + services.size());
            System.out.println("  Total requests: " + totalRequests);
            System.out.println("  Total failures: " + totalFailures);
            System.out.println("  Circuit breaker trips: " + circuitBreakerTrips);
            
            if (totalRequests > 0) {
                System.out.println("  Success rate: " + 
                                 String.format("%.1f%%", 
                                 (float)(totalRequests - totalFailures) / 
                                 totalRequests * 100));
            }
            
            System.out.println("\nService Status:");
            System.out.println(String.format("  %-10s %-20s %-15s %-15s %-15s",
                                           "ID", "Name", "Success", "Failed", "Status"));
            System.out.println(String.format("  %-10s %-20s %-15s %-15s %-15s",
                                           "---", "----", "-------", "------", "------"));
            
            for (Service svc : services.values()) {
                System.out.println(String.format("  %-10d %-20s %-15d %-15d %s",
                                                svc.serviceId, svc.name,
                                                svc.requestsSuccess, svc.requestsFailed,
                                                svc.status));
            }
        }
    }
    
    static void demo() {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ MICROSERVICES ARCHITECTURE DEMONSTRATION                      ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        MS ms = new MS();
        
        System.out.println("\n1. Register Microservices:");
        System.out.println("──────────────────────────");
        
        ms.registerService(1, "OrderService", "order.local", 8080);
        ms.registerService(2, "PaymentService", "payment.local", 8081);
        ms.registerService(3, "InventoryService", "inventory.local", 8082);
        ms.registerService(4, "ShippingService", "shipping.local", 8083);
        
        System.out.println("\n2. Define Dependencies:");
        System.out.println("───────────────────────");
        
        ms.addDependency(1, 2);
        ms.addDependency(1, 3);
        ms.addDependency(1, 4);
        
        System.out.println("\n3. Service-to-Service Calls:");
        System.out.println("──────────────────────────");
        
        for (int i = 0; i < 15; i++) {
            ms.callService(1, 2);
            ms.callService(1, 3);
            ms.callService(1, 4);
        }
        
        ms.printStats();
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ MICROSERVICES ARCHITECTURE - COMPREHENSIVE DEMONSTRATION     ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        demo();
        
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ KEY PATTERNS                                                 ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Service Discovery: Find service by name/ID");
        System.out.println("2. Circuit Breaker: Prevent cascading failures");
        System.out.println("3. Retry Logic: Exponential backoff");
        System.out.println("4. Timeout: Prevent hanging");
        System.out.println("5. Load Balancing: Distribute traffic");
    }
}
