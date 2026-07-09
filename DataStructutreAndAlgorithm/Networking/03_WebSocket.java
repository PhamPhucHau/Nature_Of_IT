import java.util.*;

/**
 * WEBSOCKET ARCHITECTURE - JAVA IMPLEMENTATION
 * 
 * STORY:
 * HTTP was a one-way street: client asks, server answers, connection closes.
 * But what if we need continuous conversation? Stock prices changing, chat messages arriving,
 * game updates flowing?
 * WebSocket opened a two-way channel. Now server can push updates to client at will.
 * The connection stays open, messages flow both directions, real-time magic happens.
 * 
 * ARCHITECTURE:
 * Client                Server
 *   |                    |
 *   ├─ Upgrade to WS ───>|
 *   |                    |
 *   |<─ Upgrade OK ──────┤
 *   |                    |
 *   |<─ Data Flow ─────>|  (Bidirectional)
 *   |                    |
 * Event Loop            Event Loop
 * Ring Buffer           Ring Buffer
 * 
 * Data Structures:
 * - Ring Buffer (for efficient message buffering)
 * - Queue (for event processing)
 * - HashMap (for connection management)
 * 
 * Algorithms:
 * - Event Loop (single-threaded async)
 * - Publish/Subscribe pattern
 * - Frame encoding/decoding
 * - Backpressure handling
 * 
 * COMPLEXITY:
 * - Send message: O(1) ring buffer
 * - Receive message: O(1) ring buffer
 * - Broadcast: O(n) where n = connections
 */

public class WebSocket {
    
    // Message frame
    static class Message {
        int clientId;
        String payload;
        long timestamp;
        int type;  // 0=text, 1=binary, 2=control
        
        Message(int clientId, String payload, int type) {
            this.clientId = clientId;
            this.payload = payload;
            this.timestamp = System.currentTimeMillis();
            this.type = type;
        }
    }
    
    // Ring buffer - O(1) circular messaging
    static class RingBuffer {
        private static final int CAPACITY = 1024;
        private Message[] messages;
        private int writeIdx;
        private int readIdx;
        private int count;
        
        RingBuffer() {
            this.messages = new Message[CAPACITY];
            this.writeIdx = 0;
            this.readIdx = 0;
            this.count = 0;
        }
        
        // Enqueue message - O(1)
        boolean enqueue(Message msg) {
            if (count >= CAPACITY) {
                return false;  // Backpressure
            }
            
            messages[writeIdx] = msg;
            writeIdx = (writeIdx + 1) % CAPACITY;
            count++;
            return true;
        }
        
        // Dequeue message - O(1)
        Message dequeue() {
            if (count == 0) {
                return null;
            }
            
            Message msg = messages[readIdx];
            readIdx = (readIdx + 1) % CAPACITY;
            count--;
            return msg;
        }
        
        int size() {
            return count;
        }
    }
    
    // WebSocket connection state
    static class WSConnection {
        int clientId;
        String clientAddress;
        boolean isConnected;
        long connectedTime;
        RingBuffer sendBuffer;      // Outgoing messages
        RingBuffer receiveBuffer;   // Incoming messages
        int messagesReceived;
        int messagesSent;
        
        WSConnection(int clientId, String address) {
            this.clientId = clientId;
            this.clientAddress = address;
            this.isConnected = true;
            this.connectedTime = System.currentTimeMillis();
            this.sendBuffer = new RingBuffer();
            this.receiveBuffer = new RingBuffer();
            this.messagesReceived = 0;
            this.messagesSent = 0;
        }
    }
    
    // Pub/Sub topic
    static class Topic {
        String topicName;
        List<Integer> subscribers;
        int messageCount;
        
        Topic(String name) {
            this.topicName = name;
            this.subscribers = new ArrayList<>();
            this.messageCount = 0;
        }
        
        void subscribe(int clientId) {
            if (!subscribers.contains(clientId)) {
                subscribers.add(clientId);
            }
        }
    }
    
    // WebSocket server
    static class WSServer {
        List<WSConnection> connections;
        Map<String, Topic> topics;
        int totalMessagesProcessed;
        
        WSServer() {
            this.connections = new ArrayList<>();
            this.topics = new HashMap<>();
            this.totalMessagesProcessed = 0;
            System.out.println("WebSocket Server created");
        }
        
        // Accept connection
        boolean acceptConnection(int clientId, String address) {
            if (connections.size() >= 100) {
                System.out.println("Max connections reached!");
                return false;
            }
            
            connections.add(new WSConnection(clientId, address));
            System.out.println("✓ WebSocket upgraded for Client " + clientId + 
                             " from " + address);
            System.out.println("  Active connections: " + connections.size());
            return true;
        }
        
        // Find connection by client ID
        WSConnection getConnection(int clientId) {
            for (WSConnection conn : connections) {
                if (conn.clientId == clientId) {
                    return conn;
                }
            }
            return null;
        }
        
        // Send message from client
        boolean sendMessage(int clientId, String payload) {
            WSConnection conn = getConnection(clientId);
            if (conn == null) return false;
            
            Message msg = new Message(clientId, payload, 0);
            boolean success = conn.receiveBuffer.enqueue(msg);
            
            if (success) {
                conn.messagesReceived++;
            }
            
            return success;
        }
        
        // Broadcast to all clients
        void broadcast(int sourceClientId, String payload) {
            System.out.println("\n[Broadcast] Client " + sourceClientId + 
                             " sends to all:");
            System.out.println("  Message: " + payload);
            
            Message msg = new Message(sourceClientId, payload, 0);
            int sent = 0;
            
            for (WSConnection conn : connections) {
                if (conn.isConnected && conn.sendBuffer.enqueue(msg)) {
                    conn.messagesSent++;
                    sent++;
                }
            }
            
            System.out.println("  Delivered to " + sent + " clients");
        }
        
        // Subscribe to topic
        void subscribe(int clientId, String topicName) {
            Topic topic = topics.computeIfAbsent(topicName, Topic::new);
            topic.subscribe(clientId);
            System.out.println("✓ Client " + clientId + " subscribed to topic: " + 
                             topicName);
            System.out.println("  Subscribers: " + topic.subscribers.size());
        }
        
        // Publish to topic
        void publish(String topicName, String payload) {
            Topic topic = topics.get(topicName);
            if (topic == null) {
                System.out.println("Topic not found: " + topicName);
                return;
            }
            
            System.out.println("\n[Publish] Topic '" + topicName + "':");
            System.out.println("  Message: " + payload);
            
            Message msg = new Message(0, payload, 0);  // Server message
            int delivered = 0;
            
            for (int clientId : topic.subscribers) {
                WSConnection conn = getConnection(clientId);
                if (conn != null && conn.sendBuffer.enqueue(msg)) {
                    conn.messagesSent++;
                    delivered++;
                }
            }
            
            topic.messageCount++;
            System.out.println("  Delivered to " + delivered + " subscribers");
        }
        
        // Event loop - process all messages
        void eventLoop() {
            System.out.println("\n=== EVENT LOOP - PROCESSING MESSAGES ===");
            
            int messagesProcessed = 0;
            
            for (WSConnection conn : connections) {
                if (!conn.isConnected) continue;
                
                Message msg;
                while ((msg = conn.sendBuffer.dequeue()) != null) {
                    System.out.println("\n[Client " + conn.clientId + 
                                     " receives] From Client " + msg.clientId + 
                                     ": " + msg.payload);
                    messagesProcessed++;
                }
            }
            
            totalMessagesProcessed += messagesProcessed;
            System.out.println("\nProcessed " + messagesProcessed + " messages");
        }
        
        // Print statistics
        void printStats() {
            System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
            System.out.println("║ WEBSOCKET SERVER STATISTICS                                  ║");
            System.out.println("╚════════════════════════════════════════════════════════════════╝");
            
            System.out.println("\nServer Status:");
            System.out.println("  Active connections: " + connections.size());
            System.out.println("  Topics: " + topics.size());
            System.out.println("  Total messages: " + totalMessagesProcessed);
            
            System.out.println("\nConnections:");
            System.out.println(String.format("  %-8s %-20s %-15s %-15s", 
                                           "ID", "Address", "Received", "Sent"));
            System.out.println(String.format("  %-8s %-20s %-15s %-15s", 
                                           "---", "-------", "--------", "----"));
            
            for (WSConnection conn : connections) {
                System.out.println(String.format("  %-8d %-20s %-15d %-15d",
                                                conn.clientId, conn.clientAddress,
                                                conn.messagesReceived, conn.messagesSent));
            }
            
            System.out.println("\nTopics:");
            System.out.println(String.format("  %-20s %-15s %-15s", 
                                           "Name", "Subscribers", "Messages"));
            System.out.println(String.format("  %-20s %-15s %-15s", 
                                           "----", "-----------", "--------"));
            
            for (Map.Entry<String, Topic> entry : topics.entrySet()) {
                Topic topic = entry.getValue();
                System.out.println(String.format("  %-20s %-15d %-15d",
                                                topic.topicName, topic.subscribers.size(),
                                                topic.messageCount));
            }
        }
    }
    
    // DEMONSTRATION
    
    static void websocketDemo() {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ WEBSOCKET ARCHITECTURE DEMONSTRATION                          ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        WSServer server = new WSServer();
        
        System.out.println("\n1. Clients Upgrade to WebSocket:");
        System.out.println("────────────────────────────────");
        
        server.acceptConnection(1, "192.168.1.10");
        server.acceptConnection(2, "192.168.1.11");
        server.acceptConnection(3, "192.168.1.12");
        server.acceptConnection(4, "192.168.1.13");
        
        System.out.println("\n2. Clients Subscribe to Topics:");
        System.out.println("───────────────────────────────");
        
        server.subscribe(1, "notifications");
        server.subscribe(2, "notifications");
        server.subscribe(3, "notifications");
        
        server.subscribe(1, "stock-prices");
        server.subscribe(2, "stock-prices");
        
        server.subscribe(3, "chat");
        server.subscribe(4, "chat");
        
        System.out.println("\n3. Bidirectional Message Flow:");
        System.out.println("──────────────────────────────");
        
        server.sendMessage(1, "Hello everyone!");
        server.sendMessage(2, "Hi from client 2");
        server.sendMessage(3, "Status update");
        
        System.out.println("\n4. Broadcast Messages:");
        System.out.println("──────────────────────");
        
        server.broadcast(1, "Important system notification");
        server.broadcast(2, "Connection established successfully");
        
        System.out.println("\n5. Topic-Based Publishing:");
        System.out.println("──────────────────────────");
        
        server.publish("notifications", "You have a new message");
        server.publish("stock-prices", "AAPL: $150.25");
        server.publish("chat", "User joined the chat");
        
        System.out.println("\n6. Event Loop Processing:");
        System.out.println("─────────────────────────");
        
        server.eventLoop();
        
        server.printStats();
    }
    
    static void complexityAnalysis() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nOperations:");
        System.out.println("  Ring buffer enqueue: O(1)");
        System.out.println("  Ring buffer dequeue: O(1)");
        System.out.println("  Broadcast to n clients: O(n)");
        System.out.println("  Topic publish to m subscribers: O(m)");
        System.out.println("  Event loop processing: O(m) where m = total messages\n");
        
        System.out.println("Memory efficiency:");
        System.out.println("  Ring buffer: Fixed size, no allocation/deallocation");
        System.out.println("  Circular design: Reuse memory efficiently");
        System.out.println("  Per-connection: 2 ring buffers + connection state");
    }
    
    static void networkingPatterns() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ WEBSOCKET NETWORKING PATTERNS                                ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Bidirectional Communication:");
        System.out.println("   - Server can push updates to client");
        System.out.println("   - Client can send messages anytime");
        System.out.println("   - No polling needed\n");
        
        System.out.println("2. Ring Buffer Efficiency:");
        System.out.println("   - Fixed memory allocation");
        System.out.println("   - O(1) enqueue/dequeue");
        System.out.println("   - Prevents memory fragmentation\n");
        
        System.out.println("3. Event Loop (Single-threaded async):");
        System.out.println("   - Process all events sequentially");
        System.out.println("   - No race conditions");
        System.out.println("   - Scales to thousands of connections\n");
        
        System.out.println("4. Pub/Sub Pattern:");
        System.out.println("   - Clients subscribe to topics");
        System.out.println("   - Server publishes to subscribers");
        System.out.println("   - Decouples publishers from subscribers\n");
        
        System.out.println("5. Backpressure Handling:");
        System.out.println("   - Reject if ring buffer full");
        System.out.println("   - Prevents memory exhaustion");
        System.out.println("   - Signals client to slow down");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ WEBSOCKET ARCHITECTURE - COMPREHENSIVE DEMONSTRATION         ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        websocketDemo();
        complexityAnalysis();
        networkingPatterns();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Ring Buffer: O(1) ops, fixed memory, circular design");
        System.out.println("2. Event Loop: Single-threaded async processing");
        System.out.println("3. Pub/Sub: Scalable message distribution");
        System.out.println("4. Bidirectional: Server push + client pull");
        System.out.println("5. Backpressure: Prevent buffer overflow");
    }
}
