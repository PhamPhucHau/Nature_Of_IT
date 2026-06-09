# 📦 Phase 5: Spring Boot Starters

> **Spring Boot Concept**: `spring-boot-starter-*`, transitive dependency bundling
> **C Equivalent**: A linker flag that pulls in a group of libraries + their init config

---

## What is a Starter?

A starter is just a **Maven/Gradle dependency with no code**.
Its entire purpose is to pull in the right transitive dependencies.

```xml
<!-- You add ONE dependency -->
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-web</artifactId>
</dependency>

<!-- Spring Boot pulls in ALL of these for you: -->
<!--   spring-webmvc          → DispatcherServlet, @Controller           -->
<!--   spring-web             → HTTP client, RestTemplate                 -->
<!--   spring-boot-autoconfigure → WebMvcAutoConfiguration               -->
<!--   jackson-databind        → JSON serialization                       -->
<!--   jackson-datatype-jsr310 → Java 8 date/time JSON support           -->
<!--   spring-boot-starter     → core Spring Boot + logging               -->
<!--   spring-boot-starter-tomcat → embedded Tomcat                       -->
<!--   tomcat-embed-core       → Tomcat core                              -->
<!--   tomcat-embed-websocket  → WebSocket support                        -->
<!--   slf4j-api               → logging facade                           -->
<!--   logback-classic         → logging implementation                   -->
```

One line of dependency = 15+ jars correctly versioned and wired.

---

## Starter Architecture

Every starter = 2 modules (best practice):

```text
my-library-spring-boot-starter/         ← the starter (no code, just pom.xml)
    pom.xml: depends on:
        my-library                      ← your actual library
        my-library-spring-boot-autoconfigure ← auto-config module

my-library-spring-boot-autoconfigure/   ← the auto-config module
    MyLibraryAutoConfiguration.java     ← @AutoConfiguration class
    MyLibraryProperties.java            ← @ConfigurationProperties
    META-INF/spring/                    ← registration file
        AutoConfiguration.imports       ← lists MyLibraryAutoConfiguration
```

---

## Common Starters

| Starter | What It Provides |
|---------|-----------------|
| `spring-boot-starter` | Core: logging, auto-config, YAML support |
| `spring-boot-starter-web` | Spring MVC + Tomcat + Jackson |
| `spring-boot-starter-webflux` | Spring WebFlux + Netty + Jackson |
| `spring-boot-starter-data-jpa` | JPA + Hibernate + HikariCP |
| `spring-boot-starter-data-redis` | Spring Data Redis + Lettuce client |
| `spring-boot-starter-security` | Spring Security filter chain |
| `spring-boot-starter-actuator` | Metrics, health, info endpoints |
| `spring-boot-starter-test` | JUnit 5, Mockito, AssertJ, TestContainers |
| `spring-boot-starter-validation` | Jakarta Bean Validation + Hibernate Validator |
| `spring-boot-starter-mail` | JavaMailSender + SMTP config |
| `spring-boot-starter-cache` | Spring Cache abstraction |
| `spring-boot-starter-aop` | Spring AOP + AspectJ |

---

## Starter Dependency Chain Visualization

```text
spring-boot-starter-web
│
├── spring-boot-starter (core)
│   ├── spring-boot
│   ├── spring-boot-autoconfigure
│   ├── spring-boot-starter-logging
│   │   ├── logback-classic
│   │   └── slf4j-api
│   └── snakeyaml (YAML parser)
│
├── spring-boot-starter-json
│   ├── jackson-databind
│   ├── jackson-datatype-jdk8
│   └── jackson-datatype-jsr310
│
├── spring-boot-starter-tomcat
│   ├── tomcat-embed-core
│   └── tomcat-embed-websocket
│
└── spring-web + spring-webmvc
```

---

## Version Management via BOM

Spring Boot manages versions for ~300 dependencies via its BOM (Bill of Materials):

```xml
<!-- Parent POM brings in the BOM -->
<parent>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-parent</artifactId>
    <version>3.2.0</version>
</parent>

<!-- Now you DON'T need to specify versions: -->
<dependency>
    <groupId>com.fasterxml.jackson.core</groupId>
    <artifactId>jackson-databind</artifactId>
    <!-- version omitted — Boot BOM manages it -->
</dependency>
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `starter_resolution.c` | Starter → dependency chain → auto-config trigger |
| `dependency_graph.c` | BOM version resolution and conflict detection |

---

## Build and Run

```bash
gcc -Wall -Wextra -o starter_resolution starter_resolution.c && ./starter_resolution
```

---

## Key Insight

```text
A starter is a DECLARATION, not code.
"I need web capabilities" → include starter-web
→ Boot handles: which jars, which versions, which auto-configurations.

The power is in the CURATED set of tested, compatible dependency versions.
Before Boot, dependency version conflicts were a daily Java developer headache.
Starters + BOM eliminate that problem.
```
