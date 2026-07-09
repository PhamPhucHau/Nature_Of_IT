import java.util.*;

/**
 * PUB/SUB ARCHITECTURE - JAVA IMPLEMENTATION
 */

public class PubSub {
    
    static class Subscriber {
        int subscriberId;
        String name;
        int messagesReceived;
        
        Subscriber(int id, String name) {
            this.subscriberId = id;
            this.name = name;
            this.messagesReceived = 0;
        }
    }
    
    static class Topic {
        String topicName;
        List<Subscriber> subscribers;
        int messageCount;
        
        Topic(String name) {
            this.topicName = name;
            this.subscribers = new ArrayList<>();
            this.messageCount = 0;
        }
    }
    
    static class Message {
        int messageId;
        int publisherId;
        String topicName;
        String payload;
        long timestamp;
        
        Message(int id, int pubId, String topic, String payload) {
            this.messageId = id;
            this.publisherId = pubId;
            this.topicName = topic;
            this.payload = payload;
            this.timestamp = System.currentTimeMillis();
        }
    }
    
    static class PubSubSystem {
        Map<String, Topic> topics;
        List<Message> messageHistory;
        int totalMessagesPublished;
        int totalMessagesFanned;
        
        PubSubSystem() {
            this.topics = new HashMap<>();
            this.messageHistory = new ArrayList<>();
            this.totalMessagesPublished = 0;
            this.totalMessagesFanned = 0;
            System.out.println("Pub/Sub System created");
        }
        
        Topic getOrCreateTopic(String topicName) {
            return topics.computeIfAbsent(topicName, Topic::new);
        }
        
        void subscribe(int subscriberId, String name, String topicName) {
            Topic topic = getOrCreateTopic(topicName);
            
            // Check if already subscribed
            for (Subscriber sub : topic.subscribers) {
                if (sub.subscriberId == subscriberId) {
                    return;
                }
            }
            
            topic.subscribers.add(new Subscriber(subscriberId, name));
            System.out.println("✓ Subscriber " + subscriberId + " (" + name + 
                             ") subscribed to '" + topicName + "'");
            System.out.println("  Subscribers in topic: " + topic.subscribers.size());
        }
        
        void publish(int publisherId, String topicName, String payload) {
            Topic topic = topics.get(topicName);
            
            if (topic == null) {
                System.out.println("✗ Topic '" + topicName + "' not found");
                return;
            }
            
            System.out.println("\n[Publish] Publisher " + publisherId + 
                             " sends to '" + topicName + "':");
            System.out.println("  Message: " + payload);
            
            Message msg = new Message(totalMessagesPublished + 1, publisherId, 
                                     topicName, payload);
            totalMessagesPublished++;
            topic.messageCount++;
            messageHistory.add(msg);
            
            // FAN-OUT: Send to all subscribers
            int deliveredCount = 0;
            for (Subscriber sub : topic.subscribers) {
                sub.messagesReceived++;
                totalMessagesFanned++;
                deliveredCount++;
                System.out.println("  → [" + sub.subscriberId + "] " + sub.name + " received");
            }
            
            System.out.println("  Delivered to " + deliveredCount + " subscribers");
        }
        
        void getSubscriberStats(String topicName) {
            Topic topic = topics.get(topicName);
            if (topic == null) return;
            
            System.out.println("\n=== SUBSCRIBERS FOR TOPIC '" + topicName + "' ===");
            System.out.println(String.format("  %-8s %-20s %-15s",
                                           "ID", "Name", "Messages Rcv"));
            System.out.println(String.format("  %-8s %-20s %-15s",
                                           "---", "----", "-----------"));
            
            for (Subscriber sub : topic.subscribers) {
                System.out.println(String.format("  %-8d %-20s %-15d",
                                                sub.subscriberId, sub.name,
                                                sub.messagesReceived));
            }
        }
        
        void printStats() {
            System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
            System.out.println("║ PUB/SUB STATISTICS                                           ║");
            System.out.println("╚════════════════════════════════════════════════════════════════╝");
            
            System.out.println("\nSystem Status:");
            System.out.println("  Total topics: " + topics.size());
            System.out.println("  Total messages published: " + totalMessagesPublished);
            System.out.println("  Total messages fanned-out: " + totalMessagesFanned);
            System.out.println("  Average fan-out per message: " +
                             String.format("%.1f", totalMessagesPublished > 0 ?
                             (float)totalMessagesFanned / totalMessagesPublished : 0));
            
            System.out.println("\nTopic Details:");
            System.out.println(String.format("  %-20s %-15s %-15s",
                                           "Topic", "Subscribers", "Messages"));
            System.out.println(String.format("  %-20s %-15s %-15s",
                                           "-----", "-----------", "--------"));
            
            for (Map.Entry<String, Topic> entry : topics.entrySet()) {
                Topic topic = entry.getValue();
                System.out.println(String.format("  %-20s %-15d %-15d",
                                                topic.topicName, topic.subscribers.size(),
                                                topic.messageCount));
            }
        }
    }
    
    static void demo() {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ PUB/SUB ARCHITECTURE DEMONSTRATION                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        PubSubSystem ps = new PubSubSystem();
        
        System.out.println("\n1. Create Topics:");
        System.out.println("──────────────────");
        ps.getOrCreateTopic("sports");
        ps.getOrCreateTopic("tech");
        ps.getOrCreateTopic("politics");
        System.out.println("✓ Topics created: sports, tech, politics");
        
        System.out.println("\n2. Subscribe to Topics:");
        System.out.println("─────────────────────────");
        
        ps.subscribe(1, "Alice", "sports");
        ps.subscribe(2, "Bob", "sports");
        ps.subscribe(3, "Charlie", "tech");
        ps.subscribe(1, "Alice", "tech");
        ps.subscribe(2, "Bob", "politics");
        
        System.out.println("\n3. Publish Messages - FAN-OUT:");
        System.out.println("──────────────────────────────");
        
        ps.publish(101, "sports", "Final Score: TeamA 3-2 TeamB");
        ps.publish(102, "tech", "New AI Model Released");
        ps.publish(103, "sports", "Championship Announced");
        ps.publish(104, "tech", "5G Network Expansion");
        ps.publish(105, "politics", "Election Results");
        
        System.out.println("\n4. View Subscriber Statistics:");
        System.out.println("──────────────────────────────");
        
        ps.getSubscriberStats("sports");
        ps.getSubscriberStats("tech");
        
        ps.printStats();
    }
    
    static void complexityAnalysis() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nOperations:");
        System.out.println("  Subscribe: O(1)");
        System.out.println("  Publish: O(m) where m = subscribers");
        System.out.println("  Topic lookup: O(1)");
        System.out.println("  Total fan-out: O(n) where n = total subscribers");
    }
    
    static void networkingPatterns() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ PUB/SUB NETWORKING PATTERNS                                  ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Fan-Out Distribution: One message → Many subscribers");
        System.out.println("2. Topic Hierarchy: Organize by categories");
        System.out.println("3. Wildcard Subscriptions: Pattern-based matching");
        System.out.println("4. Message Retention: Store for late subscribers");
        System.out.println("5. Filtering: Client-side or server-side");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ PUB/SUB ARCHITECTURE - COMPREHENSIVE DEMONSTRATION           ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        demo();
        complexityAnalysis();
        networkingPatterns();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Fan-out: O(m) distribution");
        System.out.println("2. Decouple: Publishers and subscribers independent");
        System.out.println("3. Topics: Logical message grouping");
        System.out.println("4. Scalability: Can handle high fan-out");
        System.out.println("5. Flexibility: Client-side filtering");
    }
}
