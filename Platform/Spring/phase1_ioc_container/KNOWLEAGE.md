# 🌱 Phase 1: IoC Container & Bean Factory

> **Spring Concept**: `ApplicationContext` / `BeanFactory`
> **C Equivalent**: A registry of `void*` pointers indexed by type name.

---

## What is IoC?

**Inversion of Control** means:

```text
WITHOUT IoC (traditional):
  Your code controls object creation:
    UserService svc = new UserService();   ← YOU create it
    svc.setRepo(new UserRepository());     ← YOU wire it

WITH IoC (Spring):
  Container controls object creation:
    @Service class UserService { }         ← you declare it
    Spring creates it, wires it, owns it   ← SPRING creates it
```

The CONTROL is INVERTED — from your code to the framework.

---

## BeanFactory vs ApplicationContext

```text
BeanFactory
  = Minimal IoC container
  = Only creates beans when asked (lazy)
  = Lower-level API

ApplicationContext
  = Full-featured container
  = Extends BeanFactory
  = Creates all singletons at startup (eager)
  = Adds: Events, i18n, Resource loading, AOP
```

In Spring Boot, you always use `ApplicationContext`.

---

## What is a Bean?

A **Bean** is any object managed by the Spring IoC container.

```java
// Java: declaring a bean
@Component          // ← tells Spring to manage this class
public class UserService {
    // Spring will create one instance and keep it forever (singleton)
}
```

Under the hood, Spring creates:

```text
BeanDefinition {
    class       = UserService.class
    scope       = SINGLETON
    lazyInit    = false
    initMethod  = null
    destroyMethod = null
    dependencies = [UserRepository]
}
```

Then instantiates it and stores it in:

```text
Map<String, Object> singletonObjects
    "userService" → <UserService instance>
    "userRepository" → <UserRepository instance>
    "orderService" → <OrderService instance>
```

---

## Bean Scopes

| Scope       | Created          | Destroyed        | Use Case          |
|-------------|------------------|------------------|-------------------|
| singleton   | Once at startup  | When app closes  | Services, DAOs    |
| prototype   | Every `getBean()`| When GC runs     | Stateful objects  |
| request     | Per HTTP request | Request ends     | Web-layer beans   |
| session     | Per HTTP session | Session expires  | User session data |

---

## Bean Lifecycle

```text
BeanDefinition registered
  ↓
Constructor called (instantiation)
  ↓
Fields populated (dependency injection)
  ↓
@PostConstruct method called
  ↓
Bean is READY — stored in singleton map
  ↓
(used throughout app lifetime)
  ↓
@PreDestroy method called
  ↓
Bean destroyed
```

---

## Spring Source Reference

```text
DefaultListableBeanFactory.java
  → registerBeanDefinition()      ← stores BeanDefinition
  → getBean()                     ← retrieves or creates bean
  → doCreateBean()                ← actual instantiation
  → populateBean()                ← injects dependencies
  → initializeBean()              ← runs @PostConstruct
```

---

## C Files in This Phase

| File                  | What It Simulates                          |
|-----------------------|--------------------------------------------|
| `bean_factory.c`      | Core registry: store/retrieve beans by name|
| `bean_definition.c`   | Metadata: class name, scope, lifecycle     |
| `bean_scope.c`        | Singleton vs Prototype behavior            |

---

## Build and Run

```bash
# Compile each file independently
gcc -Wall -Wextra -o bean_factory bean_factory.c && ./bean_factory
gcc -Wall -Wextra -o bean_definition bean_definition.c && ./bean_definition
gcc -Wall -Wextra -o bean_scope bean_scope.c && ./bean_scope
```

---

## Key Insight

```text
Spring's BeanFactory is fundamentally just:

    HashMap<String, Object> singletonObjects;

Everything else — lifecycle, injection, proxying —
is built on top of this simple map.

The complexity is not in the data structure.
It is in the ORCHESTRATION:
who creates what, when, in what order.
```
