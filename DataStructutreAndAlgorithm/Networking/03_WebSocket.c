/*
================================================================================
WEBSOCKET ARCHITECTURE - C IMPLEMENTATION
================================================================================

STORY:
------
HTTP was a one-way street: client asks, server answers, connection closes.

But what if we need continuous conversation? Stock prices changing, chat messages arriving,
game updates flowing?

WebSocket opened a two-way channel. Now server can push updates to client at will.
The connection stays open, messages flow both directions, real-time magic happens.

ARCHITECTURE:
  Client                Server
    |                    |
    ├─ Upgrade to WS ───>|
    |                    |
    |<─ Upgrade OK ──────┤
    |                    |
    |<─ Data Flow ─────>|  (Bidirectional)
    |                    |
  Event Loop            Event Loop
  Ring Buffer           Ring Buffer

Data Structures:
- Ring Buffer (for efficient message buffering)
- Queue (for event processing)
- HashMap (for connection management)

Algorithms:
- Event Loop (single-threaded async)
- Publish/Subscribe pattern
- Frame encoding/decoding
- Backpressure handling

COMPLEXITY:
- Send message: O(1) ring buffer
- Receive message: O(1) ring buffer
- Broadcast: O(n) where n = connections

USE CASES:
- Chat applications
- Real-time notifications
- Live stock tickers
- Collaborative editing
- Multiplayer games
- Dashboard updates
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define RING_BUFFER_SIZE 1024
#define MAX_CONNECTIONS 100
#define MAX_MESSAGE_LEN 256
#define MAX_SUBSCRIBERS 50

// Message frame
typedef struct {
    int clientId;
    char payload[MAX_MESSAGE_LEN];
    time_t timestamp;
    int type;  // 0=text, 1=binary, 2=control
} Message;

// Ring buffer for efficient circular messaging
typedef struct {
    Message messages[RING_BUFFER_SIZE];
    int writeIdx;
    int readIdx;
    int count;
} RingBuffer;

// WebSocket connection state
typedef struct {
    int clientId;
    char clientAddress[50];
    int isConnected;
    time_t connectedTime;
    RingBuffer* sendBuffer;    // Outgoing messages
    RingBuffer* receiveBuffer; // Incoming messages
    int messagesReceived;
    int messagesSent;
} WSConnection;

// Pub/Sub topic
typedef struct {
    char topicName[50];
    int subscribers[MAX_SUBSCRIBERS];
    int subscriberCount;
    int messageCount;
} Topic;

// WebSocket server
typedef struct {
    WSConnection connections[MAX_CONNECTIONS];
    int activeConnections;
    Topic topics[20];
    int topicCount;
    int totalMessagesProcessed;
} WSServer;

/*
CREATE RING BUFFER - O(1) space, circular design
*/
RingBuffer* ring_buffer_create() {
    RingBuffer* rb = malloc(sizeof(RingBuffer));
    rb->writeIdx = 0;
    rb->readIdx = 0;
    rb->count = 0;
    return rb;
}

/*
ENQUEUE MESSAGE TO RING BUFFER - O(1)
*/
int ring_buffer_enqueue(RingBuffer* rb, Message msg) {
    if (rb->count >= RING_BUFFER_SIZE) {
        printf("Ring buffer full!\n");
        return 0;  // Backpressure
    }
    
    rb->messages[rb->writeIdx] = msg;
    rb->writeIdx = (rb->writeIdx + 1) % RING_BUFFER_SIZE;
    rb->count++;
    
    return 1;
}

/*
DEQUEUE MESSAGE FROM RING BUFFER - O(1)
*/
int ring_buffer_dequeue(RingBuffer* rb, Message* msg) {
    if (rb->count == 0) {
        return 0;
    }
    
    *msg = rb->messages[rb->readIdx];
    rb->readIdx = (rb->readIdx + 1) % RING_BUFFER_SIZE;
    rb->count--;
    
    return 1;
}

/*
GET RING BUFFER SIZE
*/
int ring_buffer_size(RingBuffer* rb) {
    return rb->count;
}

/*
CREATE WEBSOCKET SERVER
*/
WSServer* wsserver_create() {
    WSServer* server = malloc(sizeof(WSServer));
    server->activeConnections = 0;
    server->topicCount = 0;
    server->totalMessagesProcessed = 0;
    
    printf("WebSocket Server created\n");
    return server;
}

/*
CLIENT UPGRADES TO WEBSOCKET
*/
int wsserver_accept_connection(WSServer* server, int clientId, const char* address) {
    if (server->activeConnections >= MAX_CONNECTIONS) {
        printf("Max connections reached!\n");
        return 0;
    }
    
    WSConnection* conn = &server->connections[server->activeConnections];
    conn->clientId = clientId;
    strcpy(conn->clientAddress, address);
    conn->isConnected = 1;
    conn->connectedTime = time(NULL);
    conn->sendBuffer = ring_buffer_create();
    conn->receiveBuffer = ring_buffer_create();
    conn->messagesReceived = 0;
    conn->messagesSent = 0;
    
    server->activeConnections++;
    
    printf("✓ WebSocket upgraded for Client %d from %s\n", clientId, address);
    printf("  Active connections: %d\n", server->activeConnections);
    
    return 1;
}

/*
CLIENT SENDS MESSAGE
*/
int wsserver_send_message(WSServer* server, int clientId, const char* payload) {
    for (int i = 0; i < server->activeConnections; i++) {
        if (server->connections[i].clientId == clientId) {
            Message msg;
            msg.clientId = clientId;
            strcpy(msg.payload, payload);
            msg.timestamp = time(NULL);
            msg.type = 0;  // Text
            
            if (!ring_buffer_enqueue(server->connections[i].receiveBuffer, msg)) {
                printf("  [Backpressure] Client %d buffer full\n", clientId);
                return 0;
            }
            
            server->connections[i].messagesReceived++;
            return 1;
        }
    }
    
    return 0;
}

/*
BROADCAST MESSAGE TO ALL CLIENTS
*/
void wsserver_broadcast(WSServer* server, int sourceClientId, const char* payload) {
    printf("\n[Broadcast] Client %d sends to all:\n", sourceClientId);
    printf("  Message: %s\n", payload);
    
    Message msg;
    msg.clientId = sourceClientId;
    strcpy(msg.payload, payload);
    msg.timestamp = time(NULL);
    msg.type = 0;
    
    int sent = 0;
    for (int i = 0; i < server->activeConnections; i++) {
        if (server->connections[i].isConnected) {
            if (ring_buffer_enqueue(server->connections[i].sendBuffer, msg)) {
                server->connections[i].messagesSent++;
                sent++;
            }
        }
    }
    
    printf("  Delivered to %d clients\n", sent);
}

/*
SUBSCRIBE CLIENT TO TOPIC
*/
int wsserver_subscribe(WSServer* server, int clientId, const char* topicName) {
    // Find or create topic
    int topicIdx = -1;
    for (int i = 0; i < server->topicCount; i++) {
        if (strcmp(server->topics[i].topicName, topicName) == 0) {
            topicIdx = i;
            break;
        }
    }
    
    if (topicIdx == -1) {
        if (server->topicCount >= 20) {
            return 0;
        }
        topicIdx = server->topicCount;
        strcpy(server->topics[topicIdx].topicName, topicName);
        server->topics[topicIdx].subscriberCount = 0;
        server->topics[topicIdx].messageCount = 0;
        server->topicCount++;
    }
    
    // Add subscriber
    Topic* topic = &server->topics[topicIdx];
    if (topic->subscriberCount >= MAX_SUBSCRIBERS) {
        return 0;
    }
    
    topic->subscribers[topic->subscriberCount] = clientId;
    topic->subscriberCount++;
    
    printf("✓ Client %d subscribed to topic: %s\n", clientId, topicName);
    printf("  Subscribers: %d\n", topic->subscriberCount);
    
    return 1;
}

/*
PUBLISH MESSAGE TO TOPIC
*/
void wsserver_publish(WSServer* server, const char* topicName, const char* payload) {
    for (int i = 0; i < server->topicCount; i++) {
        if (strcmp(server->topics[i].topicName, topicName) == 0) {
            Topic* topic = &server->topics[i];
            printf("\n[Publish] Topic '%s':\n", topicName);
            printf("  Message: %s\n", payload);
            
            Message msg;
            msg.clientId = 0;  // Server message
            strcpy(msg.payload, payload);
            msg.timestamp = time(NULL);
            msg.type = 0;
            
            int delivered = 0;
            for (int j = 0; j < topic->subscriberCount; j++) {
                int clientId = topic->subscribers[j];
                for (int k = 0; k < server->activeConnections; k++) {
                    if (server->connections[k].clientId == clientId) {
                        ring_buffer_enqueue(server->connections[k].sendBuffer, msg);
                        server->connections[k].messagesSent++;
                        delivered++;
                        break;
                    }
                }
            }
            
            topic->messageCount++;
            printf("  Delivered to %d subscribers\n", delivered);
            return;
        }
    }
}

/*
EVENT LOOP - Process all messages
*/
void wsserver_event_loop(WSServer* server) {
    printf("\n=== EVENT LOOP - PROCESSING MESSAGES ===\n");
    
    int messagesProcessed = 0;
    
    for (int i = 0; i < server->activeConnections; i++) {
        if (!server->connections[i].isConnected) continue;
        
        WSConnection* conn = &server->connections[i];
        Message msg;
        
        // Process all messages in send buffer (outgoing)
        while (ring_buffer_dequeue(conn->sendBuffer, &msg)) {
            printf("\n[Client %d receives] From Client %d: %s\n",
                   conn->clientId, msg.clientId, msg.payload);
            messagesProcessed++;
        }
    }
    
    server->totalMessagesProcessed += messagesProcessed;
    printf("\nProcessed %d messages\n", messagesProcessed);
}

/*
PRINT SERVER STATISTICS
*/
void wsserver_print_stats(WSServer* server) {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ WEBSOCKET SERVER STATISTICS                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nServer Status:\n");
    printf("  Active connections: %d\n", server->activeConnections);
    printf("  Topics: %d\n", server->topicCount);
    printf("  Total messages: %d\n", server->totalMessagesProcessed);
    
    printf("\nConnections:\n");
    printf("  %-8s %-20s %-15s %-15s\n", "ID", "Address", "Received", "Sent");
    printf("  %-8s %-20s %-15s %-15s\n", "---", "-------", "--------", "----");
    
    for (int i = 0; i < server->activeConnections; i++) {
        WSConnection* conn = &server->connections[i];
        printf("  %-8d %-20s %-15d %-15d\n",
               conn->clientId, conn->clientAddress,
               conn->messagesReceived, conn->messagesSent);
    }
    
    printf("\nTopics:\n");
    printf("  %-20s %-15s %-15s\n", "Name", "Subscribers", "Messages");
    printf("  %-20s %-15s %-15s\n", "----", "-----------", "--------");
    
    for (int i = 0; i < server->topicCount; i++) {
        Topic* topic = &server->topics[i];
        printf("  %-20s %-15d %-15d\n",
               topic->topicName, topic->subscriberCount, topic->messageCount);
    }
}

/*
DEMONSTRATION
*/
void websocket_demo() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ WEBSOCKET ARCHITECTURE DEMONSTRATION                          ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    WSServer* server = wsserver_create();
    
    printf("\n1. Clients Upgrade to WebSocket:\n");
    printf("────────────────────────────────\n");
    
    wsserver_accept_connection(server, 1, "192.168.1.10");
    wsserver_accept_connection(server, 2, "192.168.1.11");
    wsserver_accept_connection(server, 3, "192.168.1.12");
    wsserver_accept_connection(server, 4, "192.168.1.13");
    
    printf("\n2. Clients Subscribe to Topics:\n");
    printf("───────────────────────────────\n");
    
    wsserver_subscribe(server, 1, "notifications");
    wsserver_subscribe(server, 2, "notifications");
    wsserver_subscribe(server, 3, "notifications");
    
    wsserver_subscribe(server, 1, "stock-prices");
    wsserver_subscribe(server, 2, "stock-prices");
    
    wsserver_subscribe(server, 3, "chat");
    wsserver_subscribe(server, 4, "chat");
    
    printf("\n3. Bidirectional Message Flow:\n");
    printf("──────────────────────────────\n");
    
    wsserver_send_message(server, 1, "Hello everyone!");
    wsserver_send_message(server, 2, "Hi from client 2");
    wsserver_send_message(server, 3, "Status update");
    
    printf("\n4. Broadcast Messages:\n");
    printf("──────────────────────\n");
    
    wsserver_broadcast(server, 1, "Important system notification");
    wsserver_broadcast(server, 2, "Connection established successfully");
    
    printf("\n5. Topic-Based Publishing:\n");
    printf("──────────────────────────\n");
    
    wsserver_publish(server, "notifications", "You have a new message");
    wsserver_publish(server, "stock-prices", "AAPL: $150.25");
    wsserver_publish(server, "chat", "User joined the chat");
    
    printf("\n6. Event Loop Processing:\n");
    printf("─────────────────────────\n");
    
    wsserver_event_loop(server);
    
    wsserver_print_stats(server);
}

void complexity_analysis() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nOperations:\n");
    printf("  Ring buffer enqueue: O(1)\n");
    printf("  Ring buffer dequeue: O(1)\n");
    printf("  Broadcast to n clients: O(n)\n");
    printf("  Topic publish to m subscribers: O(m)\n");
    printf("  Event loop processing: O(m) where m = total messages\n\n");
    
    printf("Memory efficiency:\n");
    printf("  Ring buffer: Fixed size, no allocation/deallocation\n");
    printf("  Circular design: Reuse memory efficiently\n");
    printf("  Per-connection: 2 ring buffers + connection state\n");
}

void networking_patterns() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ WEBSOCKET NETWORKING PATTERNS                                ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\n1. Bidirectional Communication:\n");
    printf("   - Server can push updates to client\n");
    printf("   - Client can send messages anytime\n");
    printf("   - No polling needed\n\n");
    
    printf("2. Ring Buffer Efficiency:\n");
    printf("   - Fixed memory allocation\n");
    printf("   - O(1) enqueue/dequeue\n");
    printf("   - Prevents memory fragmentation\n\n");
    
    printf("3. Event Loop (Single-threaded async):\n");
    printf("   - Process all events sequentially\n");
    printf("   - No race conditions\n");
    printf("   - Scales to thousands of connections\n\n");
    
    printf("4. Pub/Sub Pattern:\n");
    printf("   - Clients subscribe to topics\n");
    printf("   - Server publishes to subscribers\n");
    printf("   - Decouples publishers from subscribers\n\n");
    
    printf("5. Backpressure Handling:\n");
    printf("   - Reject if ring buffer full\n");
    printf("   - Prevents memory exhaustion\n");
    printf("   - Signals client to slow down\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ WEBSOCKET ARCHITECTURE - COMPREHENSIVE DEMONSTRATION         ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    websocket_demo();
    complexity_analysis();
    networking_patterns();
    
    printf("\nKEY TAKEAWAYS:\n");
    printf("1. Ring Buffer: O(1) ops, fixed memory, circular design\n");
    printf("2. Event Loop: Single-threaded async processing\n");
    printf("3. Pub/Sub: Scalable message distribution\n");
    printf("4. Bidirectional: Server push + client pull\n");
    printf("5. Backpressure: Prevent buffer overflow\n");
    
    return 0;
}
