# 🌱 Nature of Spring Framework — Learn by Building in C

> **Goal**: Understand the TRUE internal mechanics of Spring Framework
> by implementing every core concept in C — the language of the machine.
>
> If you can build it in C, you understand it at the deepest level.

---

## 🧠 The Core Philosophy

Spring is NOT magic. It is:

```text
IoC Container
    = HashMap<Class, Object> + Reflection + Lifecycle management

Dependency Injection
    = Field scanning + Object wiring + Proxy creation

Spring MVC
    = HTTP Socket + Request routing + Reflection method invoke

AOP
    = Dynamic Proxy + Bytecode manipulation + Interceptor chain

Spring Data
    = Connection Pool + SQL generation + ResultSet mapping

Spring Security
    = Filter chain + Authentication + Authorization + Token validation
```

Every single feature can be implemented in C.
When you implement it, you OWN the knowledge.

---

## 🗺️ Learning Path

```text
Phase 1: IoC Container & Bean Factory
    ↓
Phase 2: Dependency Injection Engine
    ↓
Phase 3: Annotation Scanner & Reflection
    ↓
Phase 4: Spring MVC — DispatcherServlet
    ↓
Phase 5: AOP — Aspect Oriented Programming
    ↓
Phase 6: Spring Data — Database Layer
    ↓
Phase 7: Spring Security
    ↓
Phase 8: Spring Boot Auto-Configuration
    ↓
Phase 9: Spring Transaction Management
    ↓
Phase 10: Spring Event System
```

---

## 📁 Folder Structure

```text
Spring/
├── README.md                   ← This file
├── OVERVIEW.md                 ← Spring big picture
│
├── phase1_ioc_container/
│   ├── KNOWLEAGE.md
│   ├── bean_factory.c          ← Core IoC: HashMap<Class,Object>
│   ├── bean_definition.c       ← Bean metadata & lifecycle
│   └── bean_scope.c            ← Singleton vs Prototype
│
├── phase2_dependency_injection/
│   ├── KNOWLEAGE.md
│   ├── field_injection.c       ← @Autowired field injection
│   ├── constructor_injection.c ← Constructor-based DI
│   └── circular_dependency.c   ← How Spring solves circular deps
│
├── phase3_annotation_reflection/
│   ├── KNOWLEAGE.md
│   ├── annotation_scanner.c    ← Classpath scanning simulation
│   ├── reflection_engine.c     ← Method/field invocation
│   └── proxy_factory.c         ← JDK Proxy simulation
│
├── phase4_spring_mvc/
│   ├── KNOWLEAGE.md
│   ├── dispatcher_servlet.c    ← Central request dispatcher
│   ├── handler_mapping.c       ← URL → Controller mapping
│   ├── handler_adapter.c       ← Method invocation adapter
│   └── view_resolver.c         ← Response serialization
│
├── phase5_aop/
│   ├── KNOWLEAGE.md
│   ├── proxy_chain.c           ← Interceptor chain
│   ├── pointcut_matcher.c      ← Method matching
│   └── advice_types.c          ← Before/After/Around
│
├── phase6_spring_data/
│   ├── KNOWLEAGE.md
│   ├── connection_pool.c       ← HikariCP simulation
│   ├── jdbc_template.c         ← SQL execution engine
│   └── result_mapper.c         ← ResultSet → Object mapping
│
├── phase7_security/
│   ├── KNOWLEAGE.md
│   ├── security_filter_chain.c ← Filter chain simulation
│   ├── authentication_manager.c
│   └── jwt_validator.c
│
├── phase8_boot/
│   ├── KNOWLEAGE.md
│   ├── auto_configuration.c    ← Condition-based bean creation
│   ├── embedded_server.c       ← Embedded Tomcat simulation
│   └── application_context.c  ← ApplicationContext bootstrap
│
├── phase9_transaction/
│   ├── KNOWLEAGE.md
│   ├── transaction_manager.c   ← ACID simulation
│   └── propagation.c           ← Transaction propagation
│
└── phase10_events/
    ├── KNOWLEAGE.md
    ├── event_publisher.c        ← ApplicationEventPublisher
    └── event_listener.c         ← @EventListener simulation
```

---

## 🔥 How to Study Each Phase

For each phase, follow this pattern:

```text
1. Read KNOWLEAGE.md
   → Understand the concept at Java level

2. Read the .c file
   → See how it works at memory level

3. Build and run:
   gcc -Wall -o output file.c && ./output

4. Modify the code
   → Change values, break things, understand why

5. Map back to Spring Java API
   → "Now I know what @Autowired actually does"
```

---

## 🎯 Final Goal

After completing all phases, you will be able to:

```text
✓ Explain exactly what happens when Spring starts
✓ Trace any Spring request from HTTP → Response
✓ Debug Spring issues at memory level
✓ Build a mini Spring in any language
✓ Interview at senior/principal level
```

---

## 🚀 Start Here

→ [Phase 1: IoC Container](./phase1_ioc_container/KNOWLEAGE.md)
