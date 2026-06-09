# ⚙️ Phase 8: Spring Boot Auto-Configuration

> **Spring Concept**: `@SpringBootApplication`, `@EnableAutoConfiguration`, `@Conditional`
> **C Equivalent**: Condition-based factory function selection at startup

---

## What is Auto-Configuration?

Before Spring Boot (pure Spring):
```xml
<!-- You had to write ALL of this manually -->
<bean id="dataSource" class="com.zaxxer.hikari.HikariDataSource">
    <property name="jdbcUrl" value="jdbc:mysql://..." />
    <property name="username" value="user" />
    ...
</bean>
<bean id="entityManagerFactory" class="...LocalContainerEntityManagerFactoryBean">
    <property name="dataSource" ref="dataSource" />
    ...
</bean>
<bean id="transactionManager" class="...JpaTransactionManager">
    <property name="entityManagerFactory" ref="entityManagerFactory" />
    ...
</bean>
```

With Spring Boot:
```yaml
# application.yml — that's it
spring:
  datasource:
    url: jdbc:mysql://localhost:3306/mydb
    username: user
    password: pass
```

Spring Boot auto-creates all those beans for you.

---

## How Auto-Configuration Works

```text
@SpringBootApplication
    = @SpringBootConfiguration
    + @EnableAutoConfiguration
    + @ComponentScan

@EnableAutoConfiguration
    → Loads: META-INF/spring/org.springframework.boot.autoconfigure.AutoConfiguration.imports
    → File contains ~150 auto-configuration class names
    → Each class has @Conditional annotations
    → Spring evaluates conditions at startup
    → If condition passes → run the @Configuration class → beans created
```

---

## @Conditional Annotations

| Annotation | Condition |
|------------|-----------|
| `@ConditionalOnClass(HikariDataSource.class)` | HikariCP jar is on classpath |
| `@ConditionalOnMissingBean(DataSource.class)` | No DataSource bean already defined |
| `@ConditionalOnProperty("spring.datasource.url")` | Property is set |
| `@ConditionalOnWebApplication` | Running in web context |
| `@ConditionalOnMissingBean` | User hasn't defined their own bean |

Example auto-configuration class:
```java
@AutoConfiguration
@ConditionalOnClass({ DataSource.class, EmbeddedDatabaseType.class })
@ConditionalOnMissingBean(DataSource.class)        // don't create if user has one
@EnableConfigurationProperties(DataSourceProperties.class)
public class DataSourceAutoConfiguration {

    @Bean
    @ConditionalOnMissingBean
    public DataSource dataSource(DataSourceProperties props) {
        return props.initializeDataSourceBuilder().build();
    }
}
```

---

## The Auto-Configuration File

`META-INF/spring/org.springframework.boot.autoconfigure.AutoConfiguration.imports`:
```text
org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration
org.springframework.boot.autoconfigure.orm.jpa.HibernateJpaAutoConfiguration
org.springframework.boot.autoconfigure.web.servlet.WebMvcAutoConfiguration
org.springframework.boot.autoconfigure.security.servlet.SecurityAutoConfiguration
org.springframework.boot.autoconfigure.data.redis.RedisAutoConfiguration
... (150+ more)
```

---

## @ConfigurationProperties

```java
// Bind all spring.datasource.* to this class
@ConfigurationProperties(prefix = "spring.datasource")
public class DataSourceProperties {
    private String url;
    private String username;
    private String password;
    private int maxPoolSize = 10;   // ← default value
    // getters/setters...
}
```

Spring reads `application.yml`, finds all `spring.datasource.*` keys,
and populates this class via Reflection.

---

## Embedded Server (Tomcat)

```text
SpringApplication.run(App.class)
  ↓
Deduces: "this is a web application"
  ↓
TomcatServletWebServerFactory.getWebServer()
  ↓
Creates embedded Tomcat instance
  ↓
Registers DispatcherServlet
  ↓
Starts Tomcat on port 8080
  ↓
Application ready
```

No `web.xml`, no separate Tomcat installation — all embedded in the JAR.

---

## C Files in This Phase

| File                     | What It Simulates                              |
|--------------------------|------------------------------------------------|
| `auto_configuration.c`   | Condition evaluation + conditional bean create |
| `application_context.c`  | Full bootstrap sequence                        |
| `embedded_server.c`      | Embedded Tomcat initialization                 |

---

## Build and Run

```bash
gcc -Wall -Wextra -o auto_configuration auto_configuration.c && ./auto_configuration
```

---

## Key Insight

```text
Auto-configuration is just:
  IF (class X is on classpath)
  AND (property Y is set)
  AND (no existing bean of type Z)
  THEN create bean Z

Spring Boot provides 150+ pre-written @Configuration classes.
Each one uses @Conditional to avoid overriding what you've defined.
"Opinionated defaults with easy override" — that's the Spring Boot promise.
```
