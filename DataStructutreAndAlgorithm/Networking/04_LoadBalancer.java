import java.util.*;

/**
 * LOAD BALANCER ARCHITECTURE - JAVA IMPLEMENTATION
 */

public class LoadBalancer {
    
    static class Server {
        int serverId;
        String address;
        int port;
        int activeConnections;
        int totalRequests;
        boolean isHealthy;
        long lastHealthCheck;
        
        Server(int id, String addr, int port) {
            this.serverId = id;
            this.address = addr;
            this.port = port;
            this.activeConnections = 0;
            this.totalRequests = 0;
            this.isHealthy = true;
            this.lastHealthCheck = System.currentTimeMillis();
        }
    }
    
    static class HashNode {
        int serverId;
        int hashValue;
        
        HashNode(int sid, int hash) {
            this.serverId = sid;
            this.hashValue = hash;
        }
    }
    
    static class LB {
        List<Server> servers;
        int roundRobinIndex;
        List<HashNode> hashRing;
        int totalRequests;
        int healthCheckFailures;
        Random random;
        
        LB() {
            this.servers = new ArrayList<>();
            this.roundRobinIndex = 0;
            this.hashRing = new ArrayList<>();
            this.totalRequests = 0;
            this.healthCheckFailures = 0;
            this.random = new Random();
            System.out.println("Load Balancer created");
        }
        
        void registerServer(int id, String addr, int port) {
            servers.add(new Server(id, addr, port));
            System.out.println("✓ Server " + id + " registered: " + addr + ":" + port);
        }
        
        void buildHashRing() {
            hashRing.clear();
            for (Server srv : servers) {
                for (int j = 0; j < 3; j++) {
                    String vnodeKey = "server_" + srv.serverId + "_vnode_" + j;
                    int hashValue = Math.abs(vnodeKey.hashCode());
                    hashRing.add(new HashNode(srv.serverId, hashValue));
                }
            }
            System.out.println("Hash ring built with " + hashRing.size() + " vnodes");
        }
        
        int routeRoundRobin(String requestId) {
            if (servers.isEmpty()) return -1;
            Server srv = servers.get(roundRobinIndex);
            System.out.println("[Round Robin] Request '" + requestId + 
                             "' -> Server " + srv.serverId);
            srv.activeConnections++;
            srv.totalRequests++;
            totalRequests++;
            roundRobinIndex = (roundRobinIndex + 1) % servers.size();
            return srv.serverId;
        }
        
        int routeConsistentHash(String key) {
            if (hashRing.isEmpty()) return -1;
            int keyHash = Math.abs(key.hashCode());
            
            HashNode closest = hashRing.get(0);
            int minDistance = Integer.MAX_VALUE;
            
            for (HashNode node : hashRing) {
                int distance = (node.hashValue - keyHash + 1000000) % 1000000;
                if (distance < minDistance) {
                    minDistance = distance;
                    closest = node;
                }
            }
            
            for (Server srv : servers) {
                if (srv.serverId == closest.serverId) {
                    System.out.println("[Consistent Hash] Request '" + key + 
                                     "' (hash=" + keyHash + ") -> Server " + srv.serverId);
                    srv.activeConnections++;
                    srv.totalRequests++;
                    totalRequests++;
                    return srv.serverId;
                }
            }
            return -1;
        }
        
        int routeLeastConnections(String requestId) {
            if (servers.isEmpty()) return -1;
            
            Server best = servers.get(0);
            for (Server srv : servers) {
                if (srv.activeConnections < best.activeConnections) {
                    best = srv;
                }
            }
            
            System.out.println("[Least Connections] Request '" + requestId + 
                             "' -> Server " + best.serverId + 
                             " (connections: " + best.activeConnections + ")");
            best.activeConnections++;
            best.totalRequests++;
            totalRequests++;
            return best.serverId;
        }
        
        void healthCheck() {
            System.out.println("\n=== HEALTH CHECK ===");
            for (Server srv : servers) {
                boolean wasHealthy = srv.isHealthy;
                boolean isHealthy = random.nextInt(100) < 90;
                srv.isHealthy = isHealthy;
                srv.lastHealthCheck = System.currentTimeMillis();
                
                if (!isHealthy) {
                    healthCheckFailures++;
                    System.out.println("⚠ Server " + srv.serverId + ": UNHEALTHY");
                } else {
                    System.out.println("✓ Server " + srv.serverId + ": HEALTHY");
                }
                
                if (wasHealthy && !isHealthy) {
                    System.out.println("  [Alert] Server " + srv.serverId + 
                                     " status changed: UP → DOWN");
                } else if (!wasHealthy && isHealthy) {
                    System.out.println("  [Recovery] Server " + srv.serverId + 
                                     " status changed: DOWN → UP");
                }
            }
        }
        
        void completeRequest(int serverId) {
            for (Server srv : servers) {
                if (srv.serverId == serverId && srv.activeConnections > 0) {
                    srv.activeConnections--;
                }
            }
        }
        
        void printStats() {
            System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
            System.out.println("║ LOAD BALANCER STATISTICS                                     ║");
            System.out.println("╚════════════════════════════════════════════════════════════════╝");
            
            System.out.println("\nLoad Balancer Status:");
            System.out.println("  Total requests: " + totalRequests);
            System.out.println("  Health check failures: " + healthCheckFailures);
            System.out.println("  Registered servers: " + servers.size());
            
            System.out.println("\nServer Statistics:");
            System.out.println(String.format("  %-8s %-20s %-15s %-15s %-10s",
                                           "ID", "Address", "Active Conn", "Total Req", "Health"));
            System.out.println(String.format("  %-8s %-20s %-15s %-15s %-10s",
                                           "---", "-------", "-----------", "---------", "------"));
            
            int totalActive = 0;
            for (Server srv : servers) {
                System.out.println(String.format("  %-8d %-20s %-15d %-15d %s",
                                                srv.serverId, srv.address,
                                                srv.activeConnections, srv.totalRequests,
                                                srv.isHealthy ? "HEALTHY" : "DOWN"));
                totalActive += srv.activeConnections;
            }
            
            System.out.println("\nAggregate:");
            System.out.println("  Total active connections: " + totalActive);
            if (totalRequests > 0) {
                System.out.println("  Average requests per server: " +
                                 String.format("%.1f", (float)totalRequests / servers.size()));
            }
        }
    }
    
    static void demo() {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ LOAD BALANCER ARCHITECTURE DEMONSTRATION                      ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        LB lb = new LB();
        
        System.out.println("\n1. Register Servers:");
        System.out.println("───────────────────");
        
        lb.registerServer(1, "192.168.1.10", 8080);
        lb.registerServer(2, "192.168.1.11", 8080);
        lb.registerServer(3, "192.168.1.12", 8080);
        
        System.out.println("\n2. Build Hash Ring:");
        System.out.println("──────────────────");
        
        lb.buildHashRing();
        
        System.out.println("\n3. Route Requests - ROUND ROBIN:");
        System.out.println("────────────────────────────────");
        
        lb.routeRoundRobin("req_001");
        lb.routeRoundRobin("req_002");
        lb.routeRoundRobin("req_003");
        lb.routeRoundRobin("req_004");
        
        System.out.println("\n4. Route Requests - LEAST CONNECTIONS:");
        System.out.println("─────────────────────────────────────");
        
        lb.routeLeastConnections("req_005");
        lb.routeLeastConnections("req_006");
        
        System.out.println("\n5. Complete Requests:");
        System.out.println("────────────────────");
        
        lb.completeRequest(1);
        lb.completeRequest(2);
        
        System.out.println("\n6. Route More Requests - CONSISTENT HASH:");
        System.out.println("────────────────────────────────────────");
        
        lb.routeConsistentHash("user_123");
        lb.routeConsistentHash("user_456");
        lb.routeConsistentHash("user_123");  // Should go to same server
        
        System.out.println("\n7. Health Checks:");
        System.out.println("─────────────────");
        
        lb.healthCheck();
        
        lb.printStats();
    }
    
    static void complexityAnalysis() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nRouting Algorithms:");
        System.out.println("  Round Robin: O(1)");
        System.out.println("  Consistent Hashing: O(log n)");
        System.out.println("  Least Connections: O(n)");
        System.out.println("  Health Checks: O(n)");
    }
    
    static void networkingPatterns() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ LOAD BALANCER NETWORKING PATTERNS                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Round Robin: Fair, simple distribution");
        System.out.println("2. Consistent Hashing: Cache-friendly, minimal redistribution");
        System.out.println("3. Least Connections: Load-aware routing");
        System.out.println("4. Health Checks: Monitor server health");
        System.out.println("5. Session Persistence: Sticky sessions");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ LOAD BALANCER ARCHITECTURE - COMPREHENSIVE DEMONSTRATION     ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        demo();
        complexityAnalysis();
        networkingPatterns();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Round Robin: O(1), simple, fair");
        System.out.println("2. Consistent Hashing: O(log n), cache-friendly");
        System.out.println("3. Least Connections: O(n), load-aware");
        System.out.println("4. Health Checks: Auto-failover");
        System.out.println("5. Session Persistence: Sticky sessions");
    }
}
