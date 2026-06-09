# 📡 Phase 10: Spring Event System

> **Spring Concept**: `ApplicationEventPublisher`, `@EventListener`, `ApplicationEvent`
> **C Equivalent**: Observer pattern — list of registered callbacks per event type

---

## What Are Spring Events?

Spring's event system is the built-in **Observer / Pub-Sub pattern**.

```java
// Publisher (any bean)
@Service class OrderService {
    @Autowired ApplicationEventPublisher eventPublisher;

    public Order createOrder(OrderRequest req) {
        Order order = orderRepo.save(new Order(req));
        eventPublisher.publishEvent(new OrderCreatedEvent(order));  // ← fire event
        return order;
    }
}

// Listener 1: Email notification
@Component class EmailNotificationListener {
    @EventListener
    public void onOrderCreated(OrderCreatedEvent event) {
        emailService.send("Order confirmed: " + event.getOrder().getId());
    }
}

// Listener 2: Inventory update
@Component class InventoryListener {
    @EventListener
    public void onOrderCreated(OrderCreatedEvent event) {
        inventoryService.reserve(event.getOrder().getItems());
    }
}
```

---

## Built-in Spring Events

Spring itself publishes events you can listen to:

```java
@EventListener
void onContextRefreshed(ContextRefreshedEvent e) {
    // ApplicationContext fully started — all beans ready
}

@EventListener
void onContextClosed(ContextClosedEvent e) {
    // Application shutting down
}

@EventListener
void onContextStarted(ContextStartedEvent e) { }

@EventListener
void onContextStopped(ContextStoppedEvent e) { }

// Simpler alternative to ContextRefreshedEvent:
@EventListener(ApplicationReadyEvent.class)
void onReady() {
    log.info("Application started successfully!");
}
```

---

## @TransactionalEventListener

```java
// Wait until transaction COMMITS before sending notification
// This prevents sending email if the transaction rolls back
@TransactionalEventListener(phase = TransactionPhase.AFTER_COMMIT)
public void onOrderCreated(OrderCreatedEvent event) {
    emailService.send("Order " + event.getOrderId() + " confirmed");
}
```

Phases:
- `AFTER_COMMIT` — run after transaction commits (most common)
- `AFTER_ROLLBACK` — run after transaction rolls back (error handling)
- `AFTER_COMPLETION` — run after commit OR rollback
- `BEFORE_COMMIT` — run just before commit

---

## Async Events

```java
@Async
@EventListener
public void onOrderCreated(OrderCreatedEvent event) {
    // Runs in a separate thread — doesn't block the publisher
    emailService.send(...);
}
// Requires: @EnableAsync on a @Configuration class
```

---

## Event Class Design

```java
// Simple POJO event (Spring 4.2+)
public record OrderCreatedEvent(Order order, Instant timestamp) {}

// Or extend ApplicationEvent (classic style)
public class OrderCreatedEvent extends ApplicationEvent {
    private final Order order;
    public OrderCreatedEvent(Object source, Order order) {
        super(source);
        this.order = order;
    }
}
```

---

## C Files in This Phase

| File                  | What It Simulates                         |
|-----------------------|-------------------------------------------|
| `event_publisher.c`   | ApplicationEventPublisher + event dispatch|
| `event_listener.c`    | @EventListener registration and invocation|

---

## Build and Run

```bash
gcc -Wall -Wextra -o event_publisher event_publisher.c && ./event_publisher
```

---

## When to Use Events vs Direct Calls

```text
DIRECT call (tight coupling):
  OrderService → EmailService.sendOrderConfirmation()
  Problem: OrderService KNOWS about EmailService
           Adding another notification = modifying OrderService

EVENTS (loose coupling):
  OrderService → publish(OrderCreatedEvent)
  Problem: NONE — OrderService knows nothing about listeners
           Adding another listener = just add @EventListener, no other change

Rule: use events when:
  - Multiple components react to the same action
  - You want to decouple the producer from consumers
  - Cross-module communication (orders → notifications, inventory, analytics)
  - The reaction can be delayed (async, after-commit)
```

---

## Key Insight

```text
Spring events = Observer pattern with automatic wiring.

The ApplicationContext IS the event bus.
publishEvent() → scans all beans for @EventListener methods
               → finds matching ones by event type
               → invokes them synchronously (by default)

In C: this is a HashMap<EventType, List<FunctionPointer>>
      publishEvent = iterate list for matching type, call each fn
```
