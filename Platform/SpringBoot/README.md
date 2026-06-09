# ⚡ Nature of Spring Boot — Learn by Building in C

> **Goal**: Understand what Spring Boot ADDS on top of Spring Framework.
> Every Boot feature implemented in C — from startup sequence to production readiness.
>
> **Prerequisite**: Complete the `Spring/` folder first.
> Spring Boot = Spring Framework + a very smart wrapper.

---

## 🧠 Spring vs Spring Boot — The Fundamental Difference

```text
SPRING FRAMEWORK (Spring/ folder):
  Provides the TOOLS:
    IoC Container, DI, AOP, MVC, Security, Data, Transactions
  YOU must wire everything together manually.

SPRING BOOT (this folder):
  Provides the OPINIONS:
    "Given your dependencies, here's what I'll configure for you."
  Reduces 500 lines of XML/Java config → 5 lines of application.yml
```

The two questions this folder answers:

```text
Q1: How does Spring Boot KNOW what to configure automatically?
    → Auto-Configuration + @Conditional evaluation

Q2: How does Spring Boot START so fast and cleanly?
    → SpringApplication lifecycle + ApplicationContext bootstrap
```

---

## 🗺️ What Spring Boot Adds

```text
Spring Framework Core
    +
┌───────────────────────────────────────────────────────┐
│  Spring Boot Layer                                     │
│                                                        │
│  1. SpringApplication         ← entry point bootstrap  │
│  2. Auto-Configuration        ← condition-based config │
│  3. Starters                  ← dependency bundles     │
│  4. @ConfigurationProperties  ← type-safe config bind  │
│  5. Embedded Server           ← Tomcat inside the JAR  │
│  6. Actuator                  ← production monitoring  │
│  7. Graceful Shutdown         ← clean SIGTERM handling │
│  8. Banner + Startup Logging  ← startup diagnostics    │
│  9. Profiles                  ← environment switching  │
│  10. Devtools                 ← hot reload in dev      │
└───────────────────────────────────────────────────────┘
```

---

## 🗺️ Learning Path

```text
Phase 1:  SpringApplication Lifecycle
          → What SpringApplication.run() does step by step

Phase 2:  Auto-Configuration Engine
          → How @Conditional annotations control bean creation

Phase 3:  @ConfigurationProperties Binding
          → How application.yml values reach your beans

Phase 4:  Embedded Server (Tomcat/Undertow/Netty)
          → How the server lives inside your JAR

Phase 5:  Spring Boot Starters
          → How starters bundle dependencies + auto-config

Phase 6:  Spring Boot Actuator
          → /health /metrics /info /env endpoints

Phase 7:  Spring Profiles
          → application-dev.yml vs application-prod.yml

Phase 8:  Graceful Shutdown & Lifecycle
          → SIGTERM → drain connections → shutdown hooks

Phase 9:  Spring Boot Testing
          → @SpringBootTest, @MockBean, @DataJpaTest slices

Phase 10: Spring Boot Observability
          → Micrometer metrics, distributed tracing, logging
```

---

## 📁 Folder Structure

```text
SpringBoot/
├── README.md                            ← This file
├── OVERVIEW.md                          ← Boot vs Spring comparison
├── STARTUP_SEQUENCE.md                  ← Full startup trace
│
├── phase1_spring_application/
│   ├── KNOWLEAGE.md
│   ├── spring_application.c             ← SpringApplication.run() lifecycle
│   ├── application_context_factory.c    ← Context type selection
│   └── startup_events.c                 ← ApplicationStartingEvent sequence
│
├── phase2_auto_configuration/
│   ├── KNOWLEAGE.md
│   ├── conditional_engine.c             ← @Conditional evaluation engine
│   ├── auto_config_registry.c           ← AutoConfiguration.imports loader
│   └── condition_types.c                ← OnClass/OnBean/OnProperty/OnWeb
│
├── phase3_config_properties/
│   ├── KNOWLEAGE.md
│   ├── property_binder.c                ← yml/properties → struct binding
│   ├── relaxed_binding.c                ← MY_DB_URL → myDbUrl mapping
│   └── property_source_chain.c         ← Property source priority chain
│
├── phase4_embedded_server/
│   ├── KNOWLEAGE.md
│   ├── embedded_tomcat.c                ← Tomcat lifecycle inside JVM
│   ├── servlet_context_init.c           ← DispatcherServlet registration
│   └── server_port_binding.c            ← Port conflict + random port
│
├── phase5_starters/
│   ├── KNOWLEAGE.md
│   ├── starter_resolution.c             ← How starters trigger auto-config
│   └── dependency_graph.c              ← Starter → deps → auto-config chain
│
├── phase6_actuator/
│   ├── KNOWLEAGE.md
│   ├── health_endpoint.c                ← HealthIndicator aggregation
│   ├── metrics_registry.c               ← Counter/Gauge/Timer in C
│   └── endpoint_exposure.c             ← Endpoint security + exposure
│
├── phase7_profiles/
│   ├── KNOWLEAGE.md
│   ├── profile_resolver.c               ← Active profile detection
│   └── profile_config_merge.c          ← Profile-specific property merge
│
├── phase8_graceful_shutdown/
│   ├── KNOWLEAGE.md
│   ├── shutdown_hook.c                  ← JVM shutdown hook simulation
│   ├── request_drain.c                  ← In-flight request waiting
│   └── lifecycle_phases.c              ← SmartLifecycle phases
│
├── phase9_testing/
│   ├── KNOWLEAGE.md
│   ├── spring_boot_test.c               ← Full context test simulation
│   ├── mock_bean.c                      ← @MockBean replacement
│   └── test_slices.c                   ← @DataJpaTest / @WebMvcTest
│
└── phase10_observability/
    ├── KNOWLEAGE.md
    ├── micrometer_registry.c            ← Metrics collection
    ├── tracing_context.c                ← Trace/Span propagation
    └── structured_logging.c            ← JSON log format
```

---

## 🔥 How to Study Each Phase

```text
1. Read KNOWLEAGE.md       → Java concept + Spring Boot source reference
2. Read the .c file        → machine-level implementation
3. Build:  gcc -Wall -Wextra -o output file.c && ./output
4. Compare: trace back to Spring Boot source code
5. Experiment: change config values, trigger different conditions
```

---

## 🎯 Final Goal

```text
After completing this folder + the Spring/ folder, you will:

✓ Trace exactly what happens between main() and "Started App in 2.3s"
✓ Understand WHY @ConditionalOnClass exists and how it prevents conflicts
✓ Know how Tomcat lives inside a JAR file (no external server needed)
✓ Debug production issues using Actuator endpoints with full understanding
✓ Write tests that load only the beans they need (@DataJpaTest slice)
✓ Build your own Spring Boot starter from scratch
```

---

## 🚀 Start Here

→ [STARTUP_SEQUENCE.md](./STARTUP_SEQUENCE.md) — read this before Phase 1
→ [Phase 1: SpringApplication Lifecycle](./phase1_spring_application/KNOWLEAGE.md)
