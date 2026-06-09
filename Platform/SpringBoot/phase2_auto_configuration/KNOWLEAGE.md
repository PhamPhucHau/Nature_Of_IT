# ⚙️ Phase 2: Auto-Configuration Engine

> **Spring Boot Concept**: `@EnableAutoConfiguration`, `@Conditional`, `AutoConfiguration.imports`
> **C Equivalent**: A table of `{condition_fn, factory_fn}` pairs — run factory only if condition passes

---

## The Core Problem Auto-Configuration Solves

```text
Every Spring Boot project needs DataSource, JdbcTemplate, TransactionManager.
Every project needs DispatcherServlet, HandlerMapping, MessageConverters.
Every project needs the same ~50 beans in the same configuration.

Without auto-config: you write 400 lines of @Bean methods.
With auto-config: Spring Boot writes them for you if conditions pass.
```

---

## How It Works — The 3-File Chain

```text
1. @SpringBootApplication
       ↓ contains @EnableAutoConfiguration
       ↓
2. AutoConfigurationImportSelector.getAutoConfigurationEntry()
       ↓ reads:
3. META-INF/spring/org.springframework.boot.autoconfigure.AutoConfiguration.imports
       ↓ lists 150+ class names like:
         org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration
         org.springframework.boot.autoconfigure.web.servlet.WebMvcAutoConfiguration
         ...
       ↓ for each class:
4. Evaluates @Conditional annotations on the class
       ↓ if ALL conditions pass:
5. Registers the class as a @Configuration source
       ↓ Spring processes its @Bean methods
6. Beans are created and added to ApplicationContext
```

---

## @Conditional Annotation Types

### @ConditionalOnClass
```java
@ConditionalOnClass(HikariDataSource.class)
// Pass: HikariCP jar is on classpath
// Fail: HikariCP jar not present → skip this entire auto-config
```

### @ConditionalOnMissingBean
```java
@ConditionalOnMissingBean(DataSource.class)
// Pass: NO bean of type DataSource exists yet
// Fail: User already defined their own DataSource → don't override!
// This is how YOU can override any auto-configuration
```

### @ConditionalOnProperty
```java
@ConditionalOnProperty(
    name = "spring.datasource.url",
    havingValue = "some-value",    // optional: check specific value
    matchIfMissing = false          // default: if property absent → FAIL
)
// Pass: spring.datasource.url is set in application.yml
// Fail: property not set → skip auto-config
```

### @ConditionalOnWebApplication
```java
@ConditionalOnWebApplication(type = Type.SERVLET)
// Pass: running as a SERVLET web app (spring-webmvc present)
// Fail: reactive app or non-web app → skip WebMVC auto-config
```

### @ConditionalOnBean
```java
@ConditionalOnBean(DataSource.class)
// Pass: at least one DataSource bean exists
// Fail: no DataSource present → JdbcTemplate cannot be created
// Used for: "only configure X if prerequisite Y exists"
```

### @ConditionalOnExpression
```java
@ConditionalOnExpression("${feature.x.enabled:false}")
// Evaluate a SpEL expression against environment
// Pass: expression evaluates to true
```

---

## Override Auto-Configuration

```java
// Override DataSource auto-config by providing your own:
@Configuration
public class MyDataSourceConfig {

    @Bean    // ← your bean takes priority over auto-config
    public DataSource dataSource() {
        HikariDataSource ds = new HikariDataSource();
        ds.setJdbcUrl("jdbc:mysql://custom-host:3306/db");
        ds.setMaximumPoolSize(50);   // custom pool size
        return ds;
    }
}
// Result: DataSourceAutoConfiguration skips because @ConditionalOnMissingBean(DataSource)
// fails — YOUR bean satisfies the condition
```

---

## Exclude Auto-Configuration

```java
// Completely disable specific auto-config classes:
@SpringBootApplication(exclude = {
    DataSourceAutoConfiguration.class,    // no DB configured
    SecurityAutoConfiguration.class,      // custom security setup
    JmxAutoConfiguration.class            // JMX not needed
})
public class MyApp { }

// Or via properties:
// spring.autoconfigure.exclude=org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration
```

---

## Auto-Configuration Order

```java
// Ensure MyConfig runs AFTER DataSourceAutoConfiguration:
@AutoConfiguration(after = DataSourceAutoConfiguration.class)
public class MyCustomDataConfig { }

// Ensure MyConfig runs BEFORE SecurityAutoConfiguration:
@AutoConfiguration(before = SecurityAutoConfiguration.class)
public class MySecurityConfig { }
```

---

## Writing Your Own Auto-Configuration (Custom Starter)

```java
// 1. Create the auto-config class
@AutoConfiguration
@ConditionalOnClass(MyLibrary.class)
@ConditionalOnMissingBean(MyLibraryClient.class)
@EnableConfigurationProperties(MyLibraryProperties.class)
public class MyLibraryAutoConfiguration {

    @Bean
    public MyLibraryClient myLibraryClient(MyLibraryProperties props) {
        return new MyLibraryClient(props.getHost(), props.getPort());
    }
}

// 2. Register in:
// src/main/resources/META-INF/spring/
//   org.springframework.boot.autoconfigure.AutoConfiguration.imports
// Add:
//   com.example.MyLibraryAutoConfiguration

// 3. Create the starter's pom.xml that pulls in your lib + autoconfigure module
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `conditional_engine.c` | Full @Conditional evaluation: all condition types |
| `auto_config_registry.c` | AutoConfiguration.imports loading + filtering |
| `condition_types.c` | Each @Conditional type with pass/fail examples |

---

## Build and Run

```bash
gcc -Wall -Wextra -o conditional_engine conditional_engine.c && ./conditional_engine
gcc -Wall -Wextra -o auto_config_registry auto_config_registry.c && ./auto_config_registry
```

---

## Key Insight

```text
Auto-configuration is a FILTER pipeline:

  150 auto-config candidates
    → each evaluated against conditions
    → ~10 PASS (based on your classpath + properties)
    → those 10 create ~50 beans

@ConditionalOnMissingBean is the escape hatch:
  Spring Boot ALWAYS checks if YOU have defined the bean first.
  If you have → Boot's version is skipped.
  If you haven't → Boot's version is used.
  "Opinionated defaults, easy override."
```
