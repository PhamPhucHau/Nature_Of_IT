import java.util.*;

/**
 * MESSAGE QUEUE ARCHITECTURE - JAVA IMPLEMENTATION
 */

public class MessageQueue {
    
    static class Message {
        int messageId;
        String payload;
        int priority;  // 0=low, 1=normal, 2=high
        int retryCount;
        long createdAt;
        long nextRetryAt;
        int status;  // 0=pending, 1=processing, 2=delivered, 3=failed
        
        Message(int id, String payload, int priority) {
            this.messageId = id;
            this.payload = payload;
            this.priority = priority;
            this.retryCount = 0;
            this.createdAt = System.currentTimeMillis();
            this.nextRetryAt = createdAt;
            this.status = 0;
        }
    }
    
    static class Consumer {
        int consumerId;
        String name;
        int messagesProcessed;
        int messagesFailed;
        
        Consumer(int id, String name) {
            this.consumerId = id;
            this.name = name;
            this.messagesProcessed = 0;
            this.messagesFailed = 0;
        }
    }
    
    static class MQS {
        PriorityQueue<Message> queue;  // Priority queue (higher priority first)
        Queue<Message> dlq;             // Dead letter queue
        List<Consumer> consumers;
        int totalMessagesProduced;
        int totalMessagesDelivered;
        int maxRetries;
        Random random;
        
        MQS() {
            // Comparator: higher priority comes first
            this.queue = new PriorityQueue<>((a, b) -> Integer.compare(b.priority, a.priority));
            this.dlq = new LinkedList<>();
            this.consumers = new ArrayList<>();
            this.totalMessagesProduced = 0;
            this.totalMessagesDelivered = 0;
            this.maxRetries = 3;
            this.random = new Random();
            System.out.println("Message Queue System created");
        }
        
        void registerConsumer(int id, String name) {
            consumers.add(new Consumer(id, name));
            System.out.println("✓ Consumer " + id + " (" + name + ") registered");
        }
        
        void produce(int msgId, String payload, int priority) {
            Message msg = new Message(msgId, payload, priority);
            queue.add(msg);
            totalMessagesProduced++;
            String[] priorityStr = {"LOW", "NORMAL", "HIGH"};
            System.out.println("✓ Message " + msgId + " produced (priority: " + 
                             priorityStr[priority] + ", queue size: " + queue.size() + ")");
        }
        
        void consume(int consumerId) {
            Message msg = queue.poll();
            if (msg == null) {
                System.out.println("Queue empty, nothing to consume");
                return;
            }
            
            Consumer consumer = null;
            for (Consumer c : consumers) {
                if (c.consumerId == consumerId) {
                    consumer = c;
                    break;
                }
            }
            
            if (consumer == null) return;
            
            msg.status = 1;  // processing
            System.out.println("\n[Consumer " + consumerId + "] Processing message " + 
                             msg.messageId + ": " + msg.payload);
            
            // Simulate processing with 80% success rate
            boolean success = random.nextInt(100) < 80;
            
            if (success) {
                msg.status = 2;  // delivered
                consumer.messagesProcessed++;
                totalMessagesDelivered++;
                System.out.println("  ✓ Message " + msg.messageId + " delivered");
            } else {
                System.out.println("  ✗ Message " + msg.messageId + " failed");
                consumer.messagesFailed++;
                
                if (msg.retryCount < maxRetries) {
                    msg.retryCount++;
                    int backoffSeconds = (int)Math.pow(2, msg.retryCount);
                    msg.nextRetryAt = System.currentTimeMillis() + (backoffSeconds * 1000);
                    msg.status = 0;
                    
                    System.out.println("  [Retry] Scheduled retry #" + msg.retryCount + 
                                     " in " + backoffSeconds + " seconds");
                    queue.add(msg);
                } else {
                    System.out.println("  [DLQ] Max retries exceeded, moving to Dead Letter Queue");
                    dlq.add(msg);
                    msg.status = 3;
                }
            }
        }
        
        void processDLQ() {
            System.out.println("\n=== PROCESSING DEAD LETTER QUEUE ===");
            System.out.println("DLQ messages: " + dlq.size());
            
            int idx = 1;
            for (Message msg : dlq) {
                System.out.println("  [DLQ " + idx + "] MessageId: " + msg.messageId + 
                                 ", Payload: " + msg.payload + ", Retries: " + msg.retryCount);
                idx++;
            }
        }
        
        void printStats() {
            System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
            System.out.println("║ MESSAGE QUEUE STATISTICS                                     ║");
            System.out.println("╚════════════════════════════════════════════════════════════════╝");
            
            System.out.println("\nQueue Status:");
            System.out.println("  Total produced: " + totalMessagesProduced);
            System.out.println("  Total delivered: " + totalMessagesDelivered);
            System.out.println("  Pending in queue: " + queue.size());
            System.out.println("  In Dead Letter Queue: " + dlq.size());
            
            if (totalMessagesProduced > 0) {
                System.out.println("  Delivery rate: " + 
                                 String.format("%.1f%%", (float)totalMessagesDelivered / 
                                 totalMessagesProduced * 100));
            }
            
            System.out.println("\nConsumer Statistics:");
            System.out.println(String.format("  %-8s %-20s %-15s %-15s",
                                           "ID", "Name", "Processed", "Failed"));
            System.out.println(String.format("  %-8s %-20s %-15s %-15s",
                                           "---", "----", "---------", "------"));
            
            for (Consumer c : consumers) {
                System.out.println(String.format("  %-8d %-20s %-15d %-15d",
                                                c.consumerId, c.name,
                                                c.messagesProcessed, c.messagesFailed));
            }
        }
    }
    
    static void demo() {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ MESSAGE QUEUE ARCHITECTURE DEMONSTRATION                      ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        MQS mqs = new MQS();
        
        System.out.println("\n1. Register Consumers:");
        System.out.println("─────────────────────");
        
        mqs.registerConsumer(1, "EmailService");
        mqs.registerConsumer(2, "PaymentService");
        mqs.registerConsumer(3, "NotificationService");
        
        System.out.println("\n2. Produce Messages (different priorities):");
        System.out.println("───────────────────────────────────────────");
        
        mqs.produce(1, "Send welcome email", 1);
        mqs.produce(2, "Process urgent payment", 2);
        mqs.produce(3, "Log event", 0);
        mqs.produce(4, "Send invoice", 1);
        mqs.produce(5, "Critical alert", 2);
        
        System.out.println("\n3. Consume Messages (priority order):");
        System.out.println("────────────────────────────────────");
        
        mqs.consume(1);
        mqs.consume(2);
        mqs.consume(3);
        mqs.consume(1);
        mqs.consume(2);
        
        System.out.println("\n4. Dead Letter Queue Processing:");
        System.out.println("────────────────────────────────");
        
        mqs.processDLQ();
        
        mqs.printStats();
    }
    
    static void complexityAnalysis() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nOperations:");
        System.out.println("  Enqueue (priority queue): O(log n)");
        System.out.println("  Dequeue (priority queue): O(log n)");
        System.out.println("  Retry with backoff: O(1)");
        System.out.println("  DLQ operations: O(1)");
    }
    
    static void networkingPatterns() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ MESSAGE QUEUE NETWORKING PATTERNS                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Asynchronous Processing: Decouple producer/consumer");
        System.out.println("2. Priority Handling: High-priority messages first");
        System.out.println("3. Retry Logic: Exponential backoff");
        System.out.println("4. Dead Letter Queue: Storage for failed messages");
        System.out.println("5. Acknowledgments: At-least-once delivery");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ MESSAGE QUEUE ARCHITECTURE - COMPREHENSIVE DEMONSTRATION     ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        demo();
        complexityAnalysis();
        networkingPatterns();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Priority Queue: O(log n) operations");
        System.out.println("2. Asynchronous: Decouple producer/consumer");
        System.out.println("3. Retry Logic: Exponential backoff");
        System.out.println("4. DLQ: Storage for failed messages");
        System.out.println("5. Reliability: At-least-once delivery");
    }
}
