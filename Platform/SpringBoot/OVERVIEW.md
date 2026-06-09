# 🌍 Spring Boot — The Big Picture

> This file maps every Spring Boot feature to its underlying mechanism.
> Read this before diving into any phase.

---

## Spring Boot = Spring + Auto-wiring of Spring's own config

```text
Without Spring Boot — you write this manually:

  @Configuration
  public class WebMvcConfig implements WebMvcConfigurer { ... }    // 50 lines

  @Configuration
  public class DataSourceConfig {
      @Bean DataSource dataSource() { ... }                        // 30 lines
      @Bean JdbcTemplate jdbcTemplate(DataSource ds) { ... }
      @Bean TransactionManager txManager(DataSource ds) { ... }
  }

  @Configuration
  public class SecurityConfig extends WebSecurityConfigurerAdapter { ... }  // 80 lines

  Servlet web.xml registration...
  properties loading...
  Total: ~400 lines of wiring config

With Spring Boot — Spring Boot writes that config FOR you:

  # application.yml
  spring.datasource.url: jdbc:mysql://localhost:3306/mydb    ← 3 lines
  spring.datasource.username: user
  spring.datasource.password: pass

  → Spring Boot creates DataSource + JdbcTemplate + TransactionManager automatically
```

---

## The 4 Core Spring Boot Innovations

### 1. Auto-Configuration
```text
"If class X is on classpath AND no bean Y exists → create bean Y"

Powered by:
  META-INF/spring/org.springframework.boot.autoconfigure.AutoConfiguration.imports
  → 150+ configuration classes, each gated by @Conditional
```

### 2. Starters
```text
"Include ONE starter → get ALL related auto-configurations"

spring-boot-starter-web pulls in:
  spring-webmvc, jackson, tomcat-embed, spring-boot-autoconfigure
  → all trigger their own @Conditional configurations
```

### 3. Embedded Server
```text
"Your app IS the server. No WAR deployment needed."

Main class → SpringApplication.run() → creates Tomcat instance
→ registers DispatcherServlet → binds to port 8080
→ JAR contains Tomcat itself (not just your code)
```

### 4. Production-Ready Features (Actuator)
```text
"Out-of-the-box: health checks, metrics, info, env, beans"

/actuator/health → { status: UP, components: { db: UP, disk: UP } }
/actuator/metrics/jvm.memory.used → current JVM heap usage
/actuator/beans  → all registered Spring beans
```

---

## Spring Boot Startup Sequence (condensed)

```text
main()
  │
  └─► SpringApplication.run(MyApp.class, args)
        │
        ├─ 1. Deduce application type (SERVLET / REACTIVE / NONE)
        ├─ 2. Load ApplicationContextInitializers (from spring.factories)
        ├─ 3. Load ApplicationListeners (from spring.factories)
        ├─ 4. Find main application class
        │
        ├─ 5. Publish: ApplicationStartingEvent
        ├─ 6. Prepare Environment
        │       ├─ Load application.properties / application.yml
        │       ├─ Load command-line args
        │       ├─ Load system properties + env variables
        │       └─ Publish: ApplicationEnvironmentPreparedEvent
        │
        ├─ 7. Print Banner ("Spring Boot" ASCII art)
        │
        ├─ 8. Create ApplicationContext
        │       └─ AnnotationConfigServletWebServerApplicationContext (for web apps)
        │
        ├─ 9. Prepare ApplicationContext
        │       ├─ Run ApplicationContextInitializers
        │       ├─ Register main @SpringBootApplication class
        │       └─ Publish: ApplicationContextInitializedEvent
        │
        ├─ 10. Refresh ApplicationContext   ← THE BIG STEP (see Spring/phase1)
        │        ├─ Load all @Configuration classes
        │        ├─ Process @ComponentScan
        │        ├─ Load AutoConfiguration candidates
        │        ├─ Evaluate all @Conditional annotations
        │        ├─ Create all singleton beans
        │        ├─ Start embedded Tomcat
        │        └─ Register DispatcherServlet
        │
        ├─ 11. Publish: ApplicationStartedEvent
        ├─ 12. Run ApplicationRunner / CommandLineRunner beans
        └─ 13. Publish: ApplicationReadyEvent
                └─ "Started MyApp in 2.341 seconds"
```

---

## @SpringBootApplication Unpacked

```java
@SpringBootApplication
public class MyApp {
    public static void main(String[] args) {
        SpringApplication.run(MyApp.class, args);
    }
}
```

`@SpringBootApplication` = composed annotation of three:

```text
@SpringBootConfiguration
    = @Configuration
    → marks this class as a Spring configuration source

@EnableAutoConfiguration
    → triggers loading of AutoConfiguration.imports
    → each auto-config class evaluated via @Conditional
    → matching ones run and create beans

@ComponentScan
    → scans the same package and sub-packages for @Component
    → registers all found components as beans
```

---

## Auto-Configuration vs Component Scan

```text
These are DIFFERENT mechanisms, often confused:

@ComponentScan scans YOUR code:
  com.myapp.service.UserService   ← YOUR class with @Service
  com.myapp.controller.UserCtrl  ← YOUR class with @RestController

Auto-Configuration loads SPRING BOOT's classes:
  DataSourceAutoConfiguration    ← SPRING BOOT's class (in spring-boot-autoconfigure.jar)
  WebMvcAutoConfiguration        ← SPRING BOOT's class
  SecurityAutoConfiguration      ← SPRING BOOT's class

They run at DIFFERENT times and through DIFFERENT mechanisms.
Your @Component classes are scanned first.
Auto-config runs AFTER — so it can check if you've defined a bean already.
```

---

## Property Resolution Order (highest → lowest priority)

```text
1. Command-line args:             --server.port=9090
2. SPRING_APPLICATION_JSON env:   {"server.port":9090}
3. System properties:             -Dserver.port=9090
4. OS environment variables:      SERVER_PORT=9090
5. application-{profile}.yml:     server.port: 9090  (profile-specific)
6. application.yml:               server.port: 9090  (default)
7. @PropertySource files:         @PropertySource("classpath:custom.properties")
8. Default values in code:        @Value("${server.port:8080}")
```

Higher priority WINS — later entries OVERRIDE earlier ones.

---

## Spring Boot vs Raw Spring — Configuration Comparison

| Feature | Raw Spring | Spring Boot |
|---------|-----------|-------------|
| DataSource | @Bean DataSource (30 lines) | spring.datasource.url in yml |
| Web MVC | WebApplicationInitializer (50 lines) | Include starter-web |
| Security | WebSecurityConfigurerAdapter (80 lines) | SecurityAutoConfiguration |
| Embedded server | Deploy WAR to Tomcat | Tomcat inside JAR, runs with `java -jar` |
| Properties | PropertyPlaceholderConfigurer (20 lines) | application.yml auto-loaded |
| Actuator | Build your own monitoring | Include starter-actuator |
| Testing | SpringJUnit4ClassRunner (manual) | @SpringBootTest (one annotation) |

---

## Where Boot Source Code Starts

```text
org.springframework.boot.SpringApplication
  → run()                          ← THE entry point

org.springframework.boot.autoconfigure.AutoConfigurationImportSelector
  → getAutoConfigurationEntry()    ← loads AutoConfiguration.imports

org.springframework.boot.autoconfigure.condition.OnClassCondition
  → getMatchOutcome()              ← evaluates @ConditionalOnClass

org.springframework.boot.web.embedded.tomcat.TomcatServletWebServerFactory
  → getWebServer()                 ← creates + configures embedded Tomcat

org.springframework.boot.actuate.health.HealthEndpoint
  → health()                       ← aggregates health indicators
```

These 5 entry points cover 80% of Spring Boot internals.
Every C file in this folder maps to one of them.
