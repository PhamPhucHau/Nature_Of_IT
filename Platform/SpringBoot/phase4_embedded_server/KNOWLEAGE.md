# 🖥️ Phase 4: Embedded Server (Tomcat/Undertow/Netty)

> **Spring Boot Concept**: `EmbeddedWebServerFactory`, `TomcatServletWebServerFactory`
> **C Equivalent**: Creating a TCP server socket inside the application process

---

## The Revolution: No More WAR Deployment

```text
BEFORE Spring Boot (traditional Java web):
  1. Build your code → produce MyApp.war
  2. Install Tomcat separately on server
  3. Copy MyApp.war into Tomcat's webapps/ folder
  4. Start Tomcat (separate process)
  5. Tomcat loads your WAR

  WAR structure:
    MyApp.war
    ├── WEB-INF/
    │   ├── web.xml          ← servlet mappings
    │   ├── classes/         ← your compiled classes
    │   └── lib/             ← your dependencies
    └── static/

AFTER Spring Boot (embedded server):
  1. Build your code → produce MyApp.jar (fat JAR / uber JAR)
  2. java -jar MyApp.jar

  JAR structure:
    MyApp.jar
    ├── BOOT-INF/
    │   ├── classes/         ← your compiled classes
    │   └── lib/             ← your dependencies INCLUDING Tomcat jars
    ├── META-INF/
    └── org/springframework/boot/loader/  ← Spring Boot launcher
```

The Tomcat server lives INSIDE the JAR. Your app IS the server.

---

## How Embedded Tomcat Starts

```text
SpringApplication.run()
  ↓
refreshContext()
  ↓
ServletWebServerApplicationContext.onRefresh()
  ↓
createWebServer()
  ↓
TomcatServletWebServerFactory.getWebServer(ServletContextInitializer...)
  ↓
  ├── new Tomcat()
  ├── tomcat.setBaseDir(createTempDir("tomcat"))
  ├── Connector connector = new Connector("HTTP/1.1")
  ├── connector.setPort(8080)   ← from server.port
  ├── tomcat.getService().addConnector(connector)
  ├── prepareContext(tomcat.getHost(), initializers)
  │     ├── TomcatEmbeddedWebappClassLoader
  │     ├── DispatcherServletRegistrationBean.onStartup()
  │     │     → ctx.addServlet("dispatcherServlet", dispatcherServlet)
  │     │     → registration.addMapping("/")
  │     └── FilterRegistrationBeans.onStartup()
  ├── tomcat.start()    ← TCP socket bound, now accepting connections
  └── return new TomcatWebServer(tomcat)
```

---

## Server Choices

Spring Boot supports 3 embedded servers:

```xml
<!-- Default: Tomcat (included with spring-boot-starter-web) -->
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-web</artifactId>
    <!-- Tomcat is transitively included -->
</dependency>

<!-- Switch to Undertow (JBoss, higher performance) -->
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-web</artifactId>
    <exclusions>
        <exclusion>
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-starter-tomcat</artifactId>
        </exclusion>
    </exclusions>
</dependency>
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-undertow</artifactId>
</dependency>

<!-- Reactive: Netty (non-blocking, event-loop) -->
<dependency>
    <groupId>org.springframework.boot</groupId>
    <artifactId>spring-boot-starter-webflux</artifactId>
    <!-- Netty is transitively included -->
</dependency>
```

---

## Server Tuning via Properties

```yaml
server:
  port: 8080
  servlet:
    context-path: /api
  
  # Tomcat-specific
  tomcat:
    threads:
      max: 200          # max worker threads (default: 200)
      min-spare: 10     # min idle threads (default: 10)
    max-connections: 8192   # max simultaneous connections
    accept-count: 100       # queue for when all threads busy
    connection-timeout: 20s
    
  # Compression
  compression:
    enabled: true
    mime-types: text/html,text/css,application/json
    min-response-size: 2048   # only compress responses ≥ 2KB
    
  # SSL/TLS
  ssl:
    enabled: true
    key-store: classpath:keystore.p12
    key-store-password: changeit
    key-store-type: PKCS12
```

---

## Random Port (for testing)

```yaml
server:
  port: 0   # 0 = let OS pick a random available port
```

```java
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.RANDOM_PORT)
class MyTest {
    @LocalServerPort
    private int port;   // injected with actual port number
}
```

---

## Port Conflict → FailureAnalyzer

```text
If port 8080 is already in use:

APPLICATION FAILED TO START

Description:
Web server failed to start. Port 8080 was already in use.

Action:
Identify and stop the process that is listening on port 8080 or
configure this application to listen on another port.

Powered by: PortInUseFailureAnalyzer
```

---

## SSL Configuration

```java
// Programmatic SSL setup (alternative to application.yml):
@Bean
public WebServerFactoryCustomizer<TomcatServletWebServerFactory> sslCustomizer() {
    return factory -> {
        factory.addConnectorCustomizers(connector -> {
            connector.setScheme("https");
            connector.setSecure(true);
            connector.setPort(8443);
        });
    };
}
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `embedded_tomcat.c` | Full Tomcat lifecycle: create → configure → bind port → start → stop |
| `servlet_context_init.c` | DispatcherServlet registration in Tomcat context |
| `server_port_binding.c` | Port binding, conflict detection, random port |

---

## Build and Run

```bash
gcc -Wall -Wextra -o embedded_tomcat embedded_tomcat.c && ./embedded_tomcat
gcc -Wall -Wextra -o server_port_binding server_port_binding.c && ./server_port_binding
```

---

## Key Insight

```text
Embedded Tomcat = Tomcat's Java API called programmatically.
Tomcat has always had an API for embedding.
Spring Boot just calls that API for you, configured from application.yml.

The fat JAR trick: Spring Boot's JarLauncher reads BOOT-INF/lib/*.jar
from INSIDE the outer JAR using a custom ClassLoader.
Normal Java doesn't support "jars inside jars" — Spring Boot's launcher
is the bridge that makes it work.
```
