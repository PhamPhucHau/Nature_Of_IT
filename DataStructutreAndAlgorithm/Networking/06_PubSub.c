/*
================================================================================
PUB/SUB ARCHITECTURE - C IMPLEMENTATION
================================================================================

STORY:
------
Imagine a newspaper. Reporters publish stories. Readers subscribe to topics.

Each reader cares about different topics:
Sports, Politics, Tech, Entertainment.

When a story is published, it goes to all subscribers of that topic.
Readers don't know about each other. Publishers don't know subscribers.

This is Pub/Sub: a pattern that decouples information producers from consumers
through topics and subscriptions.

ARCHITECTURE:
  Publisher A
  Publisher B
     |
     ├─→ Topic: "sports"
     |
     Topic Manager
     |
  ├─→ Subscriber A
  ├─→ Subscriber B
  ├─→ Subscriber C

Data Structures:
- HashMap (topics → subscriber lists)
- ArrayList (subscriber list per topic)
- Queue (event queue)

Algorithms:
- Fan-out (distribute message to all subscribers)
- Publish/Subscribe matching
- Topic filtering

COMPLEXITY:
- Subscribe: O(1)
- Publish: O(m) where m = subscribers
- Topic lookup: O(1)

USE CASES:
- Message brokers (Kafka, Redis, RabbitMQ)
- Event systems
- Stock tickers
- Chat rooms
- Real-time notifications
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TOPICS 50
#define MAX_SUBSCRIBERS_PER_TOPIC 100
#define MAX_MESSAGE_LEN 256
#define MAX_MESSAGES_HISTORY 1000

// Subscriber
typedef struct {
    int subscriberId;
    char name[50];
    int messagesReceived;
} Subscriber;

// Topic
typedef struct {
    char topicName[50];
    Subscriber* subscribers[MAX_SUBSCRIBERS_PER_TOPIC];
    int subscriberCount;
    int messageCount;
} Topic;

// Message
typedef struct {
    int messageId;
    int publisherId;
    char topicName[50];
    char payload[MAX_MESSAGE_LEN];
    time_t timestamp;
} Message;

// Pub/Sub system
typedef struct {
    Topic topics[MAX_TOPICS];
    int topicCount;
    Message messageHistory[MAX_MESSAGES_HISTORY];
    int historyCount;
    int totalMessagesPublished;
    int totalMessagesFanned;
} PubSubSystem;

/*
CREATE PUB/SUB SYSTEM
*/
PubSubSystem* pubsub_create() {
    PubSubSystem* ps = malloc(sizeof(PubSubSystem));
    ps->topicCount = 0;
    ps->historyCount = 0;
    ps->totalMessagesPublished = 0;
    ps->totalMessagesFanned = 0;
    
    printf("Pub/Sub System created\n");
    return ps;
}

/*
CREATE TOPIC
*/
Topic* pubsub_create_topic(PubSubSystem* ps, const char* topicName) {
    // Check if topic exists
    for (int i = 0; i < ps->topicCount; i++) {
        if (strcmp(ps->topics[i].topicName, topicName) == 0) {
            return &ps->topics[i];
        }
    }
    
    // Create new topic
    if (ps->topicCount >= MAX_TOPICS) {
        printf("Max topics reached!\n");
        return NULL;
    }
    
    Topic* topic = &ps->topics[ps->topicCount];
    strcpy(topic->topicName, topicName);
    topic->subscriberCount = 0;
    topic->messageCount = 0;
    
    ps->topicCount++;
    printf("✓ Topic created: %s\n", topicName);
    
    return topic;
}

/*
SUBSCRIBE TO TOPIC
*/
int pubsub_subscribe(PubSubSystem* ps, int subscriberId, const char* name,
                     const char* topicName) {
    Topic* topic = pubsub_create_topic(ps, topicName);
    if (!topic) return 0;
    
    // Check if already subscribed
    for (int i = 0; i < topic->subscriberCount; i++) {
        if (topic->subscribers[i]->subscriberId == subscriberId) {
            return 1;  // Already subscribed
        }
    }
    
    if (topic->subscriberCount >= MAX_SUBSCRIBERS_PER_TOPIC) {
        printf("Max subscribers for topic reached!\n");
        return 0;
    }
    
    // Create subscriber
    Subscriber* sub = malloc(sizeof(Subscriber));
    sub->subscriberId = subscriberId;
    strcpy(sub->name, name);
    sub->messagesReceived = 0;
    
    topic->subscribers[topic->subscriberCount] = sub;
    topic->subscriberCount++;
    
    printf("✓ Subscriber %d (%s) subscribed to '%s'\n", subscriberId, name, topicName);
    printf("  Subscribers in topic: %d\n", topic->subscriberCount);
    
    return 1;
}

/*
FIND TOPIC
*/
Topic* pubsub_find_topic(PubSubSystem* ps, const char* topicName) {
    for (int i = 0; i < ps->topicCount; i++) {
        if (strcmp(ps->topics[i].topicName, topicName) == 0) {
            return &ps->topics[i];
        }
    }
    return NULL;
}

/*
PUBLISH MESSAGE TO TOPIC - O(m) where m = subscribers
*/
void pubsub_publish(PubSubSystem* ps, int publisherId, const char* topicName,
                   const char* payload) {
    Topic* topic = pubsub_find_topic(ps, topicName);
    
    if (!topic) {
        printf("✗ Topic '%s' not found\n", topicName);
        return;
    }
    
    printf("\n[Publish] Publisher %d sends to '%s':\n", publisherId, topicName);
    printf("  Message: %s\n", payload);
    
    // Create message
    Message msg;
    msg.messageId = ps->totalMessagesPublished + 1;
    msg.publisherId = publisherId;
    strcpy(msg.topicName, topicName);
    strcpy(msg.payload, payload);
    msg.timestamp = time(NULL);
    
    ps->totalMessagesPublished++;
    topic->messageCount++;
    
    // Store in history
    if (ps->historyCount < MAX_MESSAGES_HISTORY) {
        ps->messageHistory[ps->historyCount] = msg;
        ps->historyCount++;
    }
    
    // FAN-OUT: Send to all subscribers
    int deliveredCount = 0;
    for (int i = 0; i < topic->subscriberCount; i++) {
        Subscriber* sub = topic->subscribers[i];
        sub->messagesReceived++;
        deliveredCount++;
        ps->totalMessagesFanned++;
        
        printf("  → [%d] %s received\n", sub->subscriberId, sub->name);
    }
    
    printf("  Delivered to %d subscribers\n", deliveredCount);
}

/*
GET SUBSCRIBER STATS
*/
void pubsub_get_subscriber_stats(PubSubSystem* ps, const char* topicName) {
    Topic* topic = pubsub_find_topic(ps, topicName);
    if (!topic) return;
    
    printf("\n=== SUBSCRIBERS FOR TOPIC '%s' ===\n", topicName);
    printf("  %-8s %-20s %-15s\n", "ID", "Name", "Messages Rcv");
    printf("  %-8s %-20s %-15s\n", "---", "----", "-----------");
    
    for (int i = 0; i < topic->subscriberCount; i++) {
        Subscriber* sub = topic->subscribers[i];
        printf("  %-8d %-20s %-15d\n",
               sub->subscriberId, sub->name, sub->messagesReceived);
    }
}

/*
PRINT STATISTICS
*/
void pubsub_print_stats(PubSubSystem* ps) {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ PUB/SUB STATISTICS                                           ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nSystem Status:\n");
    printf("  Total topics: %d\n", ps->topicCount);
    printf("  Total messages published: %d\n", ps->totalMessagesPublished);
    printf("  Total messages fanned-out: %d\n", ps->totalMessagesFanned);
    printf("  Average fan-out per message: %.1f\n",
           ps->totalMessagesPublished > 0 ? 
           (float)ps->totalMessagesFanned / ps->totalMessagesPublished : 0);
    
    printf("\nTopic Details:\n");
    printf("  %-20s %-15s %-15s\n", "Topic", "Subscribers", "Messages");
    printf("  %-20s %-15s %-15s\n", "-----", "-----------", "--------");
    
    for (int i = 0; i < ps->topicCount; i++) {
        Topic* topic = &ps->topics[i];
        printf("  %-20s %-15d %-15d\n",
               topic->topicName, topic->subscriberCount, topic->messageCount);
    }
}

/*
DEMONSTRATION
*/
void pubsub_demo() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ PUB/SUB ARCHITECTURE DEMONSTRATION                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    PubSubSystem* ps = pubsub_create();
    
    printf("\n1. Create Topics:\n");
    printf("──────────────────\n");
    
    pubsub_create_topic(ps, "sports");
    pubsub_create_topic(ps, "tech");
    pubsub_create_topic(ps, "politics");
    
    printf("\n2. Subscribe to Topics:\n");
    printf("─────────────────────────\n");
    
    pubsub_subscribe(ps, 1, "Alice", "sports");
    pubsub_subscribe(ps, 2, "Bob", "sports");
    pubsub_subscribe(ps, 3, "Charlie", "tech");
    pubsub_subscribe(ps, 1, "Alice", "tech");
    pubsub_subscribe(ps, 2, "Bob", "politics");
    
    printf("\n3. Publish Messages - FAN-OUT:\n");
    printf("──────────────────────────────\n");
    
    pubsub_publish(ps, 101, "sports", "Final Score: TeamA 3-2 TeamB");
    pubsub_publish(ps, 102, "tech", "New AI Model Released");
    pubsub_publish(ps, 103, "sports", "Championship Announced");
    pubsub_publish(ps, 104, "tech", "5G Network Expansion");
    pubsub_publish(ps, 105, "politics", "Election Results");
    
    printf("\n4. View Subscriber Statistics:\n");
    printf("──────────────────────────────\n");
    
    pubsub_get_subscriber_stats(ps, "sports");
    pubsub_get_subscriber_stats(ps, "tech");
    
    pubsub_print_stats(ps);
}

void complexity_analysis() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nOperations:\n");
    printf("  Subscribe: O(1)\n");
    printf("  Publish: O(m) where m = subscribers of topic\n");
    printf("  Topic lookup: O(t) where t = total topics\n");
    printf("    (Can be O(1) with HashMap)\n");
    printf("  Total fan-out: O(n) where n = total subscribers\n\n");
    
    printf("Scalability:\n");
    printf("  1000 topics × 100 subscribers = 100k fan-outs per message\n");
    printf("  With proper indexing, still O(m) per topic\n");
}

void networking_patterns() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ PUB/SUB NETWORKING PATTERNS                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\n1. Fan-Out Distribution:\n");
    printf("   - One message → Many subscribers\n");
    printf("   - Ideal for broadcasts\n");
    printf("   - High fan-out can overwhelm system\n\n");
    
    printf("2. Topic Hierarchy:\n");
    printf("   - Organize topics by category\n");
    printf("   - sports.soccer, sports.basketball\n");
    printf("   - Enables pattern-based subscriptions\n\n");
    
    printf("3. Wildcard Subscriptions:\n");
    printf("   - Subscribe to sports.*\n");
    printf("   - Receive all sports messages\n");
    printf("   - Powerful but can cause message explosion\n\n");
    
    printf("4. Message Retention:\n");
    printf("   - Store messages in topic\n");
    printf("   - Late subscribers can catch up\n");
    printf("   - Replaying historical data\n\n");
    
    printf("5. Filtering:\n");
    printf("   - Client-side: subscribers filter\n");
    printf("   - Server-side: broker filters\n");
    printf("   - Reduces network bandwidth\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ PUB/SUB ARCHITECTURE - COMPREHENSIVE DEMONSTRATION           ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    pubsub_demo();
    complexity_analysis();
    networking_patterns();
    
    printf("\nKEY TAKEAWAYS:\n");
    printf("1. Fan-out: O(m) distribution to subscribers\n");
    printf("2. Decouple: Publishers don't know subscribers\n");
    printf("3. Topics: Organize messages logically\n");
    printf("4. Scalability: Can handle high fan-out\n");
    printf("5. Flexibility: Client-side filtering\n");
    
    return 0;
}
