import java.util.*;

/**
 * DISTRIBUTED DATABASE ARCHITECTURE - JAVA IMPLEMENTATION
 */

public class DistributedDatabase {
    
    enum NodeRole { LEADER, FOLLOWER }
    
    static class ReplicaNode {
        int nodeId;
        String address;
        NodeRole role;
        int term;
        int logIndex;
        int commitmentsAcked;
        
        ReplicaNode(int id, String addr) {
            this.nodeId = id;
            this.address = addr;
            this.role = NodeRole.FOLLOWER;
            this.term = 0;
            this.logIndex = 0;
            this.commitmentsAcked = 0;
        }
    }
    
    static class LogEntry {
        int entryId;
        int term;
        String data;
        
        LogEntry(int id, int t, String d) {
            this.entryId = id;
            this.term = t;
            this.data = d;
        }
    }
    
    static class DDB {
        List<ReplicaNode> replicas;
        List<LogEntry> log;
        int totalWrites;
        int totalReads;
        int quorumSize;
        
        DDB(int replicaCount) {
            this.replicas = new ArrayList<>();
            this.log = new ArrayList<>();
            this.totalWrites = 0;
            this.totalReads = 0;
            this.quorumSize = (replicaCount / 2) + 1;
            
            for (int i = 0; i < replicaCount; i++) {
                ReplicaNode node = new ReplicaNode(i + 1, "db" + (i + 1) + ".local");
                if (i == 0) node.role = NodeRole.LEADER;
                replicas.add(node);
            }
            
            System.out.println("Distributed Database created with " + replicaCount + 
                             " replicas (quorum: " + quorumSize + ")");
        }
        
        void write(String data) {
            System.out.println("\n[Write] " + data);
            
            log.add(new LogEntry(log.size() + 1, replicas.get(0).term, data));
            totalWrites++;
            
            int acked = 0;
            for (ReplicaNode replica : replicas) {
                System.out.println("  → Replicating to Replica " + replica.nodeId + 
                                 ": " + data);
                replica.commitmentsAcked++;
                acked++;
            }
            
            System.out.println("  Replicated to " + acked + " replicas (quorum: " + 
                             quorumSize + " achieved)");
        }
        
        void read(int entryId) {
            System.out.println("\n[Read] Entry " + entryId);
            totalReads++;
            
            if (entryId > log.size()) {
                System.out.println("  Entry not found");
                return;
            }
            
            LogEntry entry = log.get(entryId - 1);
            System.out.println("  Data: " + entry.data + " (term: " + entry.term + ")");
        }
        
        void printStats() {
            System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
            System.out.println("║ DISTRIBUTED DATABASE STATISTICS                              ║");
            System.out.println("╚════════════════════════════════════════════════════════════════╝");
            
            System.out.println("\nDatabase Status:");
            System.out.println("  Total replicas: " + replicas.size());
            System.out.println("  Quorum size: " + quorumSize);
            System.out.println("  Total writes: " + totalWrites);
            System.out.println("  Total reads: " + totalReads);
            System.out.println("  Log size: " + log.size());
            
            System.out.println("\nReplica Status:");
            System.out.println(String.format("  %-10s %-20s %-10s %-10s",
                                           "ID", "Address", "Role", "Acked"));
            System.out.println(String.format("  %-10s %-20s %-10s %-10s",
                                           "---", "-------", "----", "-----"));
            
            for (ReplicaNode r : replicas) {
                System.out.println(String.format("  %-10d %-20s %-10s %-10d",
                                                r.nodeId, r.address,
                                                r.role, r.commitmentsAcked));
            }
        }
    }
    
    static void demo() {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ DISTRIBUTED DATABASE - COMPREHENSIVE DEMONSTRATION           ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        DDB ddb = new DDB(3);
        
        System.out.println("\n1. Perform Writes (Replicated):");
        System.out.println("──────────────────────────────");
        
        ddb.write("INSERT users VALUES ('Alice', 25)");
        ddb.write("INSERT users VALUES ('Bob', 30)");
        ddb.write("UPDATE users SET age=26 WHERE name='Alice'");
        
        System.out.println("\n2. Perform Reads:");
        System.out.println("─────────────────");
        
        ddb.read(1);
        ddb.read(2);
        ddb.read(3);
        
        ddb.printStats();
    }
    
    public static void main(String[] args) {
        demo();
        
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ KEY PATTERNS                                                 ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Raft Consensus: Leader election");
        System.out.println("2. Quorum Reads/Writes: Ensure consistency");
        System.out.println("3. Two-Phase Commit: Distributed transactions");
        System.out.println("4. Log Replication: Durability");
        System.out.println("5. Failover: Automatic recovery");
    }
}
