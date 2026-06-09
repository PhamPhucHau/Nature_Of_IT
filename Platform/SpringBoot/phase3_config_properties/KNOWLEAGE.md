# 🔧 Phase 3: @ConfigurationProperties Binding

> **Spring Boot Concept**: `@ConfigurationProperties`, `@Value`, `@EnableConfigurationProperties`
> **C Equivalent**: Parsing a key=value file and writing values into struct fields by name

---

## The Problem @ConfigurationProperties Solves

```java
// BAD — @Value scatter: hard to see all config, no validation, no IDE support
@Service
public class EmailService {
    @Value("${email.smtp.host}")       private String host;
    @Value("${email.smtp.port:587}")   private int port;
    @Value("${email.smtp.username}")   private String username;
    @Value("${email.smtp.password}")   private String password;
    @Value("${email.smtp.tls:true}")   private boolean tls;
    @Value("${email.retry.max:3}")     private int maxRetries;
    @Value("${email.retry.delay:1000}") private long retryDelay;
}

// GOOD — @ConfigurationProperties: grouped, validated, documented
@ConfigurationProperties(prefix = "email")
@Validated
public class EmailProperties {
    private Smtp smtp = new Smtp();     // nested object
    private Retry retry = new Retry();

    public static class Smtp {
        private String  host;
        private int     port = 587;     // default value
        private String  username;
        private String  password;
        private boolean tls = true;
        // getters/setters...
    }

    public static class Retry {
        @Min(1) @Max(10)
        private int  max = 3;
        private long delayMs = 1000;
        // getters/setters...
    }
}
```

---

## How Binding Works Internally

```text
application.yml:
  email:
    smtp:
      host: smtp.gmail.com
      port: 465
      tls: true
    retry:
      max: 5

Spring Boot binding process:
  1. Reads all properties with prefix "email"
  2. Strips prefix: "smtp.host", "smtp.port", "retry.max"
  3. Converts dotted path → field navigation: smtp → Smtp object, then host field
  4. Type conversion: "465" (String) → 465 (int)
  5. Sets via setter: emailProperties.getSmtp().setPort(465)
  6. Validates @Min/@Max constraints (if @Validated present)
```

---

## Relaxed Binding Rules

Spring Boot is flexible about property key formats — all map to the same field:

```text
Field: private String myDbUrl;
All of these map to it:
  my-db-url         ← kebab-case (recommended in .yml)
  my_db_url         ← underscore
  myDbUrl           ← camelCase
  MY_DB_URL         ← UPPER_CASE (environment variable style)
  MY-DB-URL         ← mixed
```

This is called **Relaxed Binding** — critical for 12-factor apps where
config comes from environment variables (`MY_DB_URL=...`).

---

## Property Source Priority Chain

```text
1. Command-line: --server.port=9090       (HIGHEST)
2. SPRING_APPLICATION_JSON env variable
3. JVM system properties: -Dserver.port=9090
4. OS environment variables: SERVER_PORT=9090
5. application-{profile}.properties/yml
6. application.properties / application.yml
7. @PropertySource("classpath:custom.properties")
8. Default values in code: @Value("${x:default}")  (LOWEST)
```

Same key in multiple sources → highest priority wins.

---

## Type Conversion

Spring Boot converts Strings to Java types automatically:

```text
"8080"           → int
"true" / "false" → boolean
"10s"            → Duration (java.time.Duration.ofSeconds(10))
"512MB"          → DataSize (512 * 1024 * 1024 bytes)
"INFO"           → LogLevel enum
"2024-01-01"     → LocalDate
"[a,b,c]"        → List<String>
```

---

## @Value vs @ConfigurationProperties

| | @Value | @ConfigurationProperties |
|---|---|---|
| Unit | Single field | Entire group |
| Nesting | No | Yes (object hierarchy) |
| Validation | No | Yes (via @Validated) |
| IDE completion | Limited | Full (with metadata) |
| SpEL | Yes | No |
| Binding relaxation | No | Yes |
| Use in `@Bean` method | Yes | Awkward |
| Recommended for | One-off values | Config groups |

---

## Metadata Generation

Spring Boot generates `spring-configuration-metadata.json` from `@ConfigurationProperties`.
This powers IDE autocomplete in `application.yml`.

```json
{
  "properties": [
    {
      "name": "email.smtp.host",
      "type": "java.lang.String",
      "description": "SMTP server hostname",
      "defaultValue": "localhost"
    }
  ]
}
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `property_binder.c` | Full binding: yml key → struct field via name matching |
| `relaxed_binding.c` | All naming variations mapping to the same field |
| `property_source_chain.c` | Priority chain: args > env > yml > defaults |

---

## Build and Run

```bash
gcc -Wall -Wextra -o property_binder property_binder.c && ./property_binder
gcc -Wall -Wextra -o property_source_chain property_source_chain.c && ./property_source_chain
```

---

## Key Insight

```text
@ConfigurationProperties binding is:
  1. Collect all properties matching the prefix
  2. Strip the prefix
  3. Normalize the key (relaxed binding)
  4. Match to a struct field name
  5. Convert the String value to the field's type
  6. Set via setter (or direct field write)
  7. Validate if @Validated is present

In C: this is exactly what config file parsers do.
struct field name matching + type conversion from strings.
```
