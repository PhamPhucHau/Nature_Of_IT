# 🌍 Spring Framework — The Big Picture

> Before writing a single line of C, you must understand the full Spring runtime.
> This file maps every Spring feature to its true underlying mechanism.

---

## What Happens When Spring Starts?

```text
main()
  ↓
SpringApplication.run(App.class)
  ↓
  ├── Create ApplicationContext
  ├── Scan classpath for @Component, @Service, @Repository, @Controller
  ├── Parse @Configuration classes
  ├── Create BeanDefinitions (metadata for each bean)
  ├── Instantiate Singleton beans
  ├── Inject dependencies (@Autowired, @Value)
  ├── Run @PostConstruct methods
  ├── Register DispatcherServlet
  ├── Start embedded Tomcat
  └── Server ready on port 8080
```

---

## What Happens on Each HTTP Request?

```text
Browser: GET /api/users HTTP/1.1
  ↓
TCP Socket (kernel accepts)
  ↓
Embedded Tomcat receives bytes
  ↓
HTTP Parser → HttpServletRequest
  ↓
DispatcherServlet.service()
  ↓
HandlerMapping.getHandler()  ← finds UserController.getUsers()
  ↓
HandlerAdapter.handle()      ← invokes method via Reflection
  ↓
  ├── AOP Interceptors run (logging, security, transaction)
  ├── @RequestBody deserialization
  ├── Method executes
  └── Return value
  ↓
HttpMessageConverter → JSON bytes
  ↓
HTTP/1.1 200 OK
Content-Type: application/json
{"users": [...]}
```

---

## Spring Core Architecture Layers

```text
┌────────────────────────────────────────────────────────────┐
│                   Spring Application                        │
├────────────────────────────────────────────────────────────┤
│  Spring Boot Auto-Configuration                             │
│  (Conditions, Starters, Properties binding)                 │
├────────────────────────────────────────────────────────────┤
│  Spring Web MVC                                             │
│  (DispatcherServlet, Controllers, Filters, MessageConverter)│
├────────────────────────────────────────────────────────────┤
│  Spring Security                                            │
│  (FilterChain, AuthenticationManager, SecurityContext)      │
├────────────────────────────────────────────────────────────┤
│  Spring Data / JDBC                                         │
│  (DataSource, ConnectionPool, JdbcTemplate, Repositories)  │
├────────────────────────────────────────────────────────────┤
│  Spring AOP                                                 │
│  (Proxy, Pointcut, Advice, AspectJ)                         │
├────────────────────────────────────────────────────────────┤
│  Spring Core — IoC Container                                │
│  (ApplicationContext, BeanFactory, BeanDefinition)          │
├────────────────────────────────────────────────────────────┤
│  Java Core — Reflection, ClassLoader, Annotation            │
├────────────────────────────────────────────────────────────┤
│  JVM — Heap, Stack, GC, JIT                                 │
├────────────────────────────────────────────────────────────┤
│  OS — Sockets, Threads, Memory, File System                 │
└────────────────────────────────────────────────────────────┘
```

---

## Core Concepts Mapped to C Concepts

| Spring Concept           | Java API                       | C Equivalent                          |
|--------------------------|--------------------------------|---------------------------------------|
| Bean                     | @Component instance            | `malloc(sizeof(struct))` + pointer    |
| BeanFactory              | DefaultListableBeanFactory     | `HashMap<string, void*>`              |
| Singleton scope          | One instance per context       | Global pointer in registry            |
| Prototype scope          | New instance per request       | `calloc()` every call                 |
| Dependency Injection     | @Autowired                     | Copy pointer into struct field        |
| BeanDefinition           | Class metadata object          | `struct BeanDef { name, type, ... }`  |
| ApplicationContext       | Full IoC container             | Context struct with bean registry     |
| Reflection               | `Method.invoke()`              | Function pointer table (vtable)       |
| @Component scan          | ClassPathScanningDetector      | `opendir()` + parse file names        |
| AOP Proxy                | JDK/CGLIB Proxy                | Wrapper struct with function pointers |
| @Transactional           | TransactionInterceptor         | begin/commit/rollback wrappers        |
| Filter Chain             | SecurityFilterChain            | Linked list of function pointers      |
| ApplicationEvent         | ApplicationEventPublisher      | Observer pattern: list of callbacks   |
| @Scheduled               | TaskScheduler                  | `timer_t` + signal handler            |

---

## The 3 Most Important Spring Internals

### 1. IoC Container (Phase 1-2)

```text
Spring does NOT create objects where you call `new`.
Spring controls ALL object creation via BeanFactory.

This inversion is IoC.
Your code: "I need a UserService"
Spring:    "Here is the one I manage. It's already wired."
```

### 2. AOP Proxy (Phase 5)

```text
When you use @Transactional or @Cacheable,
Spring does NOT modify your class.

Instead, Spring creates a PROXY class that wraps your class.
The proxy intercepts every method call, runs the advice,
then calls your actual method.

You never see the proxy. It's transparent.
```

### 3. DispatcherServlet (Phase 4)

```text
ALL HTTP requests go to ONE servlet: DispatcherServlet.
It is the Front Controller pattern.

It:
  1. Finds the right @Controller for the URL
  2. Invokes the method via Reflection
  3. Serializes the return value to JSON/XML
  4. Writes the HTTP response

You write @GetMapping("/users"), Spring wires the rest.
```

---

## Spring vs Raw Java vs C

```text
HTTP request handling:

C (raw):
  accept() → recv() → parse bytes → call function → send bytes

Java raw:
  ServerSocket → Socket.getInputStream() → parse → call method → write

Java Spring:
  @GetMapping("/users") public List<User> getUsers() { ... }
  (Spring handles everything else)

Spring is a 3-layer abstraction on top of C.
If you understand C, you understand Spring at full depth.
```

---

## Where Spring Source Code Starts

If you read Spring source code:

```text
org.springframework.context.support.AbstractApplicationContext
  → refresh()                    ← THE entry point

org.springframework.web.servlet.DispatcherServlet
  → doDispatch()                 ← THE HTTP handler

org.springframework.beans.factory.support.DefaultListableBeanFactory
  → getBean()                    ← THE bean resolver

org.springframework.aop.framework.JdkDynamicAopProxy
  → invoke()                     ← THE AOP interceptor
```

These 4 methods are the backbone of Spring.
Every C file in this project maps to one of them.
