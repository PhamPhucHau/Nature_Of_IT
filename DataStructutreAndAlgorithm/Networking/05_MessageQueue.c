/*
================================================================================
MESSAGE QUEUE ARCHITECTURE - C IMPLEMENTATION
================================================================================

STORY:
------
Imagine two colleagues on the phone during a crisis.
One talks, the other must listen. If one is busy, the call blocks.

Now imagine they use a messaging system instead:
Both can leave messages anytime, read them whenever ready.
Neither waits for the other. Work happens asynchronously.

This is the Message Queue: a buffer between producer and consumer,
enabling decoupled, async communication at scale.

ARCHITECTURE:
  Producer
     |
  Enqueue Message
     |
  ┌─────────────────┐
  │ Priority Queue  │
  │  Ring Buffer    │
  └─────────────────┘
     |
  Dequeue Message
     |
  Consumer

Data Structures:
- Queue (FIFO message ordering)
- Priority Queue (QoS levels)
- Ring Buffer (efficient circular buffer)

Algorithms:
- FIFO dequeuing
- Priority-based dispatch
- Retry with exponential backoff
- Dead Letter Queue for failures

COMPLEXITY:
- Enqueue: O(1) or O(log n) for priority queue
- Dequeue: O(1) or O(log n)
- Retry: O(1) with backoff
- Dead Letter: O(1) enqueue

USE CASES:
- Asynchronous task processing
- Email/SMS delivery
- Order processing
- Log aggregation
- Kafka, RabbitMQ, SQS
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_QUEUE_SIZE 1000
#define MAX_DLQ_SIZE 100
#define MAX_MESSAGE_LEN 256
#define MAX_CONSUMERS 10

// Message
typedef struct {
    int messageId;
    char payload[MAX_MESSAGE_LEN];
    int priority;  // 0=low, 1=normal, 2=high
    int retryCount;
    time_t createdAt;
    time_t nextRetryAt;
    int status;  // 0=pending, 1=processing, 2=delivered, 3=failed
} Message;

// Priority Queue Node
typedef struct {
    Message msg;
    int index;
} PQNode;

// Message Queue
typedef struct {
    PQNode nodes[MAX_QUEUE_SIZE];
    int size;
} PriorityQueue;

// Dead Letter Queue (for failed messages)
typedef struct {
    Message messages[MAX_DLQ_SIZE];
    int count;
} DeadLetterQueue;

// Consumer
typedef struct {
    int consumerId;
    char name[50];
    int messagesProcessed;
    int messagesFailed;
} Consumer;

// Message Queue System
typedef struct {
    PriorityQueue* queue;
    DeadLetterQueue* dlq;
    Consumer consumers[MAX_CONSUMERS];
    int consumerCount;
    int totalMessagesProduced;
    int totalMessagesDelivered;
    int maxRetries;
} MessageQueueSystem;

/*
SWAP ELEMENTS IN HEAP
*/
void swap(PQNode* a, PQNode* b) {
    PQNode temp = *a;
    *a = *b;
    *b = temp;
}

/*
CREATE PRIORITY QUEUE
*/
PriorityQueue* pq_create() {
    PriorityQueue* pq = malloc(sizeof(PriorityQueue));
    pq->size = 0;
    return pq;
}

/*
PARENT, LEFT CHILD, RIGHT CHILD INDICES
*/
int parent(int i) { return (i - 1) / 2; }
int left(int i) { return 2 * i + 1; }
int right(int i) { return 2 * i + 2; }

/*
HEAPIFY UP - Move node up after insertion
*/
void heapify_up(PriorityQueue* pq, int i) {
    while (i > 0 && pq->nodes[parent(i)].msg.priority < pq->nodes[i].msg.priority) {
        swap(&pq->nodes[i], &pq->nodes[parent(i)]);
        i = parent(i);
    }
}

/*
HEAPIFY DOWN - Move node down after removal
*/
void heapify_down(PriorityQueue* pq, int i) {
    int smallest = i;
    
    if (left(i) < pq->size && 
        pq->nodes[left(i)].msg.priority > pq->nodes[smallest].msg.priority) {
        smallest = left(i);
    }
    
    if (right(i) < pq->size && 
        pq->nodes[right(i)].msg.priority > pq->nodes[smallest].msg.priority) {
        smallest = right(i);
    }
    
    if (smallest != i) {
        swap(&pq->nodes[i], &pq->nodes[smallest]);
        heapify_down(pq, smallest);
    }
}

/*
ENQUEUE MESSAGE - O(log n) with priority
*/
int pq_enqueue(PriorityQueue* pq, Message msg) {
    if (pq->size >= MAX_QUEUE_SIZE) {
        printf("Queue full!\n");
        return 0;
    }
    
    PQNode node;
    node.msg = msg;
    node.index = pq->size;
    
    pq->nodes[pq->size] = node;
    heapify_up(pq, pq->size);
    pq->size++;
    
    return 1;
}

/*
DEQUEUE MESSAGE - O(log n)
*/
int pq_dequeue(PriorityQueue* pq, Message* msg) {
    if (pq->size == 0) {
        return 0;
    }
    
    *msg = pq->nodes[0].msg;
    pq->nodes[0] = pq->nodes[pq->size - 1];
    pq->size--;
    
    if (pq->size > 0) {
        heapify_down(pq, 0);
    }
    
    return 1;
}

/*
CREATE MESSAGE QUEUE SYSTEM
*/
MessageQueueSystem* mqs_create() {
    MessageQueueSystem* mqs = malloc(sizeof(MessageQueueSystem));
    mqs->queue = pq_create();
    mqs->dlq = malloc(sizeof(DeadLetterQueue));
    mqs->dlq->count = 0;
    mqs->consumerCount = 0;
    mqs->totalMessagesProduced = 0;
    mqs->totalMessagesDelivered = 0;
    mqs->maxRetries = 3;
    
    printf("Message Queue System created\n");
    return mqs;
}

/*
REGISTER CONSUMER
*/
void mqs_register_consumer(MessageQueueSystem* mqs, int id, const char* name) {
    if (mqs->consumerCount >= MAX_CONSUMERS) {
        return;
    }
    
    Consumer* consumer = &mqs->consumers[mqs->consumerCount];
    consumer->consumerId = id;
    strcpy(consumer->name, name);
    consumer->messagesProcessed = 0;
    consumer->messagesFailed = 0;
    
    mqs->consumerCount++;
    printf("✓ Consumer %d (%s) registered\n", id, name);
}

/*
PRODUCE MESSAGE - Enqueue to priority queue
*/
void mqs_produce(MessageQueueSystem* mqs, int msgId, const char* payload, int priority) {
    Message msg;
    msg.messageId = msgId;
    strcpy(msg.payload, payload);
    msg.priority = priority;  // 0=low, 1=normal, 2=high
    msg.retryCount = 0;
    msg.createdAt = time(NULL);
    msg.nextRetryAt = msg.createdAt;
    msg.status = 0;  // pending
    
    if (pq_enqueue(mqs->queue, msg)) {
        mqs->totalMessagesProduced++;
        const char* priorityStr[] = {"LOW", "NORMAL", "HIGH"};
        printf("✓ Message %d produced (priority: %s, queue size: %d)\n",
               msgId, priorityStr[priority], mqs->queue->size);
    }
}

/*
CONSUME MESSAGE - Dequeue from priority queue
*/
void mqs_consume(MessageQueueSystem* mqs, int consumerId) {
    Message msg;
    
    if (!pq_dequeue(mqs->queue, &msg)) {
        printf("Queue empty, nothing to consume\n");
        return;
    }
    
    // Find consumer
    Consumer* consumer = NULL;
    for (int i = 0; i < mqs->consumerCount; i++) {
        if (mqs->consumers[i].consumerId == consumerId) {
            consumer = &mqs->consumers[i];
            break;
        }
    }
    
    if (!consumer) return;
    
    msg.status = 1;  // processing
    printf("\n[Consumer %d] Processing message %d: %s\n",
           consumerId, msg.messageId, msg.payload);
    
    // Simulate processing with 80% success rate
    int success = (rand() % 100) < 80 ? 1 : 0;
    
    if (success) {
        msg.status = 2;  // delivered
        consumer->messagesProcessed++;
        mqs->totalMessagesDelivered++;
        printf("  ✓ Message %d delivered\n", msg.messageId);
    } else {
        printf("  ✗ Message %d failed\n", msg.messageId);
        consumer->messagesFailed++;
        
        // Retry logic with exponential backoff
        if (msg.retryCount < mqs->maxRetries) {
            msg.retryCount++;
            int backoffSeconds = (int)pow(2, msg.retryCount);  // 2^retryCount
            msg.nextRetryAt = time(NULL) + backoffSeconds;
            msg.status = 0;  // back to pending
            
            printf("  [Retry] Scheduled retry #%d in %d seconds\n",
                   msg.retryCount, backoffSeconds);
            
            // Re-enqueue for retry
            pq_enqueue(mqs->queue, msg);
        } else {
            // Max retries exceeded - send to DLQ
            printf("  [DLQ] Max retries exceeded, moving to Dead Letter Queue\n");
            
            if (mqs->dlq->count < MAX_DLQ_SIZE) {
                mqs->dlq->messages[mqs->dlq->count] = msg;
                mqs->dlq->count++;
                msg.status = 3;  // failed
            }
        }
    }
}

/*
PROCESS DLQ MESSAGES
*/
void mqs_process_dlq(MessageQueueSystem* mqs) {
    printf("\n=== PROCESSING DEAD LETTER QUEUE ===\n");
    printf("DLQ messages: %d\n", mqs->dlq->count);
    
    for (int i = 0; i < mqs->dlq->count; i++) {
        Message* msg = &mqs->dlq->messages[i];
        printf("  [DLQ %d] MessageId: %d, Payload: %s, Retries: %d\n",
               i + 1, msg->messageId, msg->payload, msg->retryCount);
    }
}

/*
PRINT STATISTICS
*/
void mqs_print_stats(MessageQueueSystem* mqs) {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ MESSAGE QUEUE STATISTICS                                     ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nQueue Status:\n");
    printf("  Total produced: %d\n", mqs->totalMessagesProduced);
    printf("  Total delivered: %d\n", mqs->totalMessagesDelivered);
    printf("  Pending in queue: %d\n", mqs->queue->size);
    printf("  In Dead Letter Queue: %d\n", mqs->dlq->count);
    
    if (mqs->totalMessagesProduced > 0) {
        printf("  Delivery rate: %.1f%%\n",
               (float)mqs->totalMessagesDelivered / mqs->totalMessagesProduced * 100);
    }
    
    printf("\nConsumer Statistics:\n");
    printf("  %-8s %-20s %-15s %-15s\n", "ID", "Name", "Processed", "Failed");
    printf("  %-8s %-20s %-15s %-15s\n", "---", "----", "---------", "------");
    
    for (int i = 0; i < mqs->consumerCount; i++) {
        Consumer* c = &mqs->consumers[i];
        printf("  %-8d %-20s %-15d %-15d\n",
               c->consumerId, c->name, c->messagesProcessed, c->messagesFailed);
    }
}

/*
DEMONSTRATION
*/
void message_queue_demo() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ MESSAGE QUEUE ARCHITECTURE DEMONSTRATION                      ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    srand(time(NULL));
    MessageQueueSystem* mqs = mqs_create();
    
    printf("\n1. Register Consumers:\n");
    printf("─────────────────────\n");
    
    mqs_register_consumer(mqs, 1, "EmailService");
    mqs_register_consumer(mqs, 2, "PaymentService");
    mqs_register_consumer(mqs, 3, "NotificationService");
    
    printf("\n2. Produce Messages (different priorities):\n");
    printf("───────────────────────────────────────────\n");
    
    mqs_produce(mqs, 1, "Send welcome email", 1);  // normal
    mqs_produce(mqs, 2, "Process urgent payment", 2);  // high
    mqs_produce(mqs, 3, "Log event", 0);  // low
    mqs_produce(mqs, 4, "Send invoice", 1);  // normal
    mqs_produce(mqs, 5, "Critical alert", 2);  // high
    
    printf("\n3. Consume Messages (priority order):\n");
    printf("────────────────────────────────────\n");
    
    mqs_consume(mqs, 1);
    mqs_consume(mqs, 2);
    mqs_consume(mqs, 3);
    mqs_consume(mqs, 1);
    mqs_consume(mqs, 2);
    
    printf("\n4. Dead Letter Queue Processing:\n");
    printf("────────────────────────────────\n");
    
    mqs_process_dlq(mqs);
    
    mqs_print_stats(mqs);
}

void complexity_analysis() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nOperations:\n");
    printf("  Enqueue (priority queue): O(log n)\n");
    printf("  Dequeue (priority queue): O(log n)\n");
    printf("  Simple FIFO enqueue: O(1)\n");
    printf("  Simple FIFO dequeue: O(1)\n");
    printf("  Retry with backoff: O(1)\n");
    printf("  DLQ operations: O(1)\n\n");
    
    printf("Memory usage:\n");
    printf("  Main queue: O(n)\n");
    printf("  DLQ: O(d) where d = max DLQ size\n");
}

void networking_patterns() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ MESSAGE QUEUE NETWORKING PATTERNS                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\n1. Asynchronous Processing:\n");
    printf("   - Producer doesn't wait for consumer\n");
    printf("   - Decouples producer and consumer\n");
    printf("   - Allows independent scaling\n\n");
    
    printf("2. Priority Handling:\n");
    printf("   - High-priority messages processed first\n");
    printf("   - Uses heap/priority queue\n");
    printf("   - Useful for QoS\n\n");
    
    printf("3. Retry Logic:\n");
    printf("   - Exponential backoff prevents thundering herd\n");
    printf("   - Configurable max retries\n");
    printf("   - Transient failures handled gracefully\n\n");
    
    printf("4. Dead Letter Queue:\n");
    printf("   - Persistent storage for failed messages\n");
    printf("   - Enables debugging and reprocessing\n");
    printf("   - Prevents message loss\n\n");
    
    printf("5. Acknowledgments:\n");
    printf("   - Consumer confirms message processing\n");
    printf("   - Server removes from queue only after ACK\n");
    printf("   - Ensures at-least-once delivery\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ MESSAGE QUEUE ARCHITECTURE - COMPREHENSIVE DEMONSTRATION     ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    message_queue_demo();
    complexity_analysis();
    networking_patterns();
    
    printf("\nKEY TAKEAWAYS:\n");
    printf("1. Priority Queue: O(log n) enqueue/dequeue\n");
    printf("2. Asynchronous: Decouple producer/consumer\n");
    printf("3. Retry Logic: Exponential backoff for resilience\n");
    printf("4. DLQ: Persistent storage for failed messages\n");
    printf("5. Reliability: At-least-once delivery with ACKs\n");
    
    return 0;
}
