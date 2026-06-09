# 🚀 Phase 1: SpringApplication Lifecycle

> **Spring Boot Concept**: `SpringApplication.run()`, startup events, context type selection
> **C Equivalent**: A state-machine that drives init → configure → start → ready

---

## The Entry Point

```java
@SpringBootApplication
public class MyApp {
    public static void main(String[] args) {
        SpringApplication.run(MyApp.class, args);
        // This method returns the running ApplicationContext
        // But normally you don't need it — app just keeps running
    }
}
```

`SpringApplication.run()` is the most important method in Spring Boot.
Everything that makes Boot "magic" flows through here.

---

## Customizing SpringApplication Before Run

```java
// Instead of static run(), build then run:
SpringApplication app = new SpringApplication(MyApp.class);

app.setBannerMode(Banner.Mode.OFF);           // disable banner
app.setWebApplicationType(WebApplicationType.NONE);  // non-web app
app.addListeners(new MyStartupListener());    // add early listener
app.setDefaultProperties(Map.of(            // programmatic defaults
    "server.port", "9090"
));

app.run(args);
```

---

## Application Types

Spring Boot deduces application type from classpath:

```text
SERVLET type  (most common)
  → Requires: spring-webmvc OR spring-web on classpath
  → Creates: AnnotationConfigServletWebServerApplicationContext
  → Starts: embedded Tomcat/Jetty/Undertow

REACTIVE type
  → Requires: spring-webflux on classpath (AND NOT spring-webmvc)
  → Creates: AnnotationConfigReactiveWebServerApplicationContext
  → Starts: embedded Netty (or Undertow in reactive mode)

NONE type
  → Neither web library on classpath
  → Creates: AnnotationConfigApplicationContext
  → No server started — runs then exits (unless kept alive)
  → Used for: batch jobs, CLI tools, scheduled tasks
```

---

## ApplicationRunner vs CommandLineRunner

Both run AFTER context is fully ready (after ApplicationReadyEvent):

```java
// ApplicationRunner: gets structured args
@Component
public class DataInitializer implements ApplicationRunner {
    @Override
    public void run(ApplicationArguments args) throws Exception {
        if (args.containsOption("seed")) {
            seedDatabase();
        }
    }
}

// CommandLineRunner: gets raw String[] args
@Component
public class CacheWarmer implements CommandLineRunner {
    @Override
    public void run(String... args) throws Exception {
        warmCache();
    }
}

// Multiple runners: control order
@Component @Order(1) class FirstRunner implements ApplicationRunner { }
@Component @Order(2) class SecondRunner implements ApplicationRunner { }
```

---

## Startup Failure — FailureAnalyzers

When startup fails, Spring Boot provides human-readable diagnosis:

```text
***************************
APPLICATION FAILED TO START
***************************

Description:
Web server failed to start. Port 8080 was already in use.

Action:
Identify and stop the process that's listening on port 8080 or
configure this application to listen on another port.
```

Powered by `FailureAnalyzer` implementations that pattern-match exceptions
to produce helpful messages. You can write your own:

```java
@Component
public class MyFailureAnalyzer extends AbstractFailureAnalyzer<MyException> {
    @Override
    protected FailureAnalysis analyze(Throwable cause, MyException ex) {
        return new FailureAnalysis(
            "description of what went wrong",
            "what the user should do",
            ex
        );
    }
}
```

---

## SpringApplicationBuilder (fluent API)

```java
// For more complex setups (parent/child contexts, web + batch):
new SpringApplicationBuilder(MyApp.class)
    .parent(SharedConfig.class)
    .child(WebConfig.class)
    .run(args);
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `spring_application.c` | Full SpringApplication.run() state machine |
| `application_context_factory.c` | Context type deduction + creation |
| `startup_events.c` | Event publishing timeline simulation |

---

## Build and Run

```bash
gcc -Wall -Wextra -o spring_application spring_application.c && ./spring_application
gcc -Wall -Wextra -o startup_events startup_events.c && ./startup_events
```

---

## Key Insight

```text
SpringApplication.run() is a state machine with 16 steps.
The most important step is step 12: refreshContext().
Everything BEFORE step 12 is setup.
Everything AFTER step 12 is notification.

Step 12 is the entire Spring Framework (see Spring/ folder).
Spring Boot's contribution is steps 1–11 and 13–16.
```
