# 🎭 Phase 5: AOP — Aspect Oriented Programming

> **Spring Concept**: `@Aspect`, `@Around`, `@Before`, `@After`, `@Transactional`, `@Cacheable`
> **C Equivalent**: Wrapper structs with function pointer tables (interceptor chains)

---

## What Problem Does AOP Solve?

```text
WITHOUT AOP — cross-cutting concerns pollute every method:

  public User getUserById(Long id) {
      log.info("Starting getUserById: " + id);      ← logging (clutter)
      checkPermission("READ_USER");                  ← security (clutter)
      Transaction tx = txManager.begin();            ← transaction (clutter)
      try {
          User user = userRepo.findById(id);         ← actual logic
          cache.put("user:" + id, user);             ← caching (clutter)
          tx.commit();
          return user;
      } catch (Exception e) {
          tx.rollback();
          throw e;
      } finally {
          log.info("Finished getUserById");          ← logging (clutter)
      }
  }

WITH AOP — clean business logic + declarative cross-cutting:

  @Transactional
  @Cacheable("users")
  @PreAuthorize("hasRole('READ_USER')")
  public User getUserById(Long id) {
      return userRepo.findById(id);   ← ONLY business logic here
  }
  (All the surrounding code runs automatically via AOP proxy)
```

---

## How Spring AOP Works

```text
1. Spring detects @Aspect class at startup
2. Spring builds a PROXY class wrapping the target bean
3. When you @Autowire UserService, you actually get the PROXY
4. Proxy intercepts every method call:
   a. Runs @Before advice
   b. Calls the real method (via Reflection/proceed())
   c. Runs @After / @AfterReturning / @AfterThrowing advice

Target bean:  UserService.getUserById()
Proxy class:  UserService$$SpringCGLIB$$0.getUserById()
              → runs all interceptors → calls super.getUserById()
```

---

## AOP Terminology

| Term | Meaning | Example |
|------|---------|---------|
| Aspect | A class containing cross-cutting logic | `LoggingAspect` |
| Advice | The code that runs at a join point | `@Around`, `@Before`, `@After` |
| Join Point | A point in execution (method call) | `getUserById()` called |
| Pointcut | Expression that selects join points | `execution(* *.Service.*(..))` |
| Weaving | Applying aspects to target code | At startup (proxy creation) |
| Target | The original object being proxied | Your `UserService` bean |

---

## Advice Types

```java
@Before("pointcut")       // runs BEFORE target method
@After("pointcut")        // runs AFTER (always, even if exception)
@AfterReturning(...)      // runs AFTER SUCCESSFUL return
@AfterThrowing(...)       // runs AFTER EXCEPTION
@Around("pointcut")       // wraps ENTIRE method — most powerful
```

`@Around` example:
```java
@Around("execution(* *.Service.*(..))")
public Object aroundAdvice(ProceedingJoinPoint pjp) throws Throwable {
    long start = System.currentTimeMillis();
    try {
        Object result = pjp.proceed();   // ← call the real method
        log("OK in " + (System.currentTimeMillis() - start) + "ms");
        return result;
    } catch (Exception e) {
        log("FAILED: " + e.getMessage());
        throw e;
    }
}
```

---

## JDK Proxy vs CGLIB (revisited for AOP)

```text
JDK Proxy:
  Target must implement interface.
  Spring creates a new Class implementing the same interface.
  Calls are routed through InvocationHandler.invoke()

CGLIB:
  Target does NOT need an interface.
  Spring generates a SUBCLASS of the target at runtime.
  Overrides all methods to add interceptor chain.
  Used by @Transactional on concrete classes.
```

---

## C Files in This Phase

| File               | What It Simulates                       |
|--------------------|-----------------------------------------|
| `proxy_chain.c`    | Full AOP proxy with interceptor chain   |
| `advice_types.c`   | All advice types: Before/After/Around   |
| `pointcut_matcher.c`| Pointcut expressions matching          |

---

## Build and Run

```bash
gcc -Wall -Wextra -o proxy_chain proxy_chain.c && ./proxy_chain
gcc -Wall -Wextra -o advice_types advice_types.c && ./advice_types
```

---

## Key Insight

```text
AOP = Proxy Pattern + Chain of Responsibility

Every @Around advice is a "before + proceed + after" wrapper.
Nesting multiple advices = nesting wrappers.

In C:
  struct Advice { fn_ptr before; fn_ptr target; fn_ptr after; }

In Spring:
  class UserService$$CGLIB extends UserService {
      @Override
      public User getUserById(Long id) {
          loggingAdvice.before();
          securityAdvice.before();
          User result = super.getUserById(id);   // real method
          securityAdvice.after();
          loggingAdvice.after();
          return result;
      }
  }
```
