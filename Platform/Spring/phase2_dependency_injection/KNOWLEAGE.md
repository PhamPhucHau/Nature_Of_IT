# 💉 Phase 2: Dependency Injection Engine

> **Spring Concept**: `@Autowired`, `@Inject`, Constructor DI, Field DI
> **C Equivalent**: Copying pointers from the bean registry into struct fields.

---

## What is Dependency Injection?

```text
WITHOUT DI:
  class UserService {
      UserRepository repo = new UserRepository();   ← YOU create the dependency
  }
  Problem: tightly coupled, hard to test, hard to change

WITH DI (Spring):
  @Service class UserService {
      @Autowired UserRepository repo;  ← Spring INJECTS it for you
  }
  Benefit: decoupled, testable, swappable
```

Spring's job: **read the @Autowired annotation → find the right bean → copy its pointer into the field**.

---

## Three Types of Injection

### 1. Field Injection (most common, but not recommended)

```java
@Service
public class UserService {
    @Autowired                     // Spring reflects into this field
    private UserRepository repo;   // and assigns the repo bean
}
```

Pros: concise  
Cons: can't inject in tests without Spring, can hide dependencies

### 2. Constructor Injection (recommended)

```java
@Service
public class UserService {
    private final UserRepository repo;

    @Autowired                              // Spring calls this constructor
    public UserService(UserRepository repo) {
        this.repo = repo;
    }
}
```

Pros: dependencies are explicit, `final` fields, testable without Spring  
Cons: more verbose

### 3. Setter Injection (optional dependencies)

```java
@Service
public class UserService {
    private EmailService emailService;  // optional

    @Autowired(required = false)        // Spring calls this if bean exists
    public void setEmailService(EmailService svc) {
        this.emailService = svc;
    }
}
```

---

## How Spring Resolves @Autowired

```text
@Autowired UserRepository repo;
     ↓
Spring checks:
  1. Find all beans of type UserRepository in context
  2. If exactly one → inject it
  3. If zero and required=true → throw NoSuchBeanDefinitionException
  4. If zero and required=false → leave field null
  5. If multiple → check @Primary, then @Qualifier
     If still ambiguous → throw NoUniqueBeanDefinitionException
```

---

## The Circular Dependency Problem

```text
UserService → depends on → OrderService
OrderService → depends on → UserService
```

Spring CANNOT create them in normal order (chicken-and-egg).

### Spring's Solution: Three-Level Cache

```text
Level 1: singletonObjects         = fully created, initialized beans
Level 2: earlySingletonObjects    = created but NOT yet initialized
Level 3: singletonFactories       = factory lambdas to create early refs

Algorithm:
  1. Start creating UserService
  2. Before injecting, put UserService factory in Level 3
  3. Encounter "need OrderService"
  4. Start creating OrderService
  5. OrderService needs UserService
  6. Find UserService in Level 3 (early reference)
  7. Move UserService to Level 2 (early, not yet initialized)
  8. Inject early UserService into OrderService
  9. OrderService fully created → move to Level 1
  10. Resume UserService creation → inject OrderService
  11. UserService fully initialized → move to Level 1
```

Note: Constructor injection CANNOT resolve circular deps (Spring throws error).

---

## C Files in This Phase

| File                     | What It Simulates                         |
|--------------------------|-------------------------------------------|
| `field_injection.c`      | @Autowired on fields via reflection       |
| `constructor_injection.c`| Constructor-based DI                      |
| `circular_dependency.c`  | 3-level cache for circular dep resolution |

---

## Build and Run

```bash
gcc -Wall -Wextra -o field_injection field_injection.c && ./field_injection
gcc -Wall -Wextra -o constructor_injection constructor_injection.c && ./constructor_injection
gcc -Wall -Wextra -o circular_dependency circular_dependency.c && ./circular_dependency
```

---

## Key Insight

```text
@Autowired is just:

    void* dep = registry_lookup("userRepository");
    target_bean->repo_field = dep;

The "magic" is Spring knowing:
  - WHICH field to inject into (via Reflection)
  - WHICH bean to inject (via type matching)
  - IN WHAT ORDER to do it (via dependency graph)
```
