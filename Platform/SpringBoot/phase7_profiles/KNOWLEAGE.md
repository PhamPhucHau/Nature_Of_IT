# 🌍 Phase 7: Spring Profiles

> **Spring Boot Concept**: `@Profile`, `spring.profiles.active`, profile-specific config files
> **C Equivalent**: Conditional compilation / runtime config switching based on an environment variable

---

## What Profiles Solve

```text
Problem: same code, different config per environment:
  DEV:  local DB, debug logging, mock services, no SSL
  TEST: in-memory DB (H2), test credentials, stub external APIs
  PROD: cloud DB, WARN logging, real services, SSL required

Solution: Spring Profiles let you declare config per environment
and switch the active profile at startup.
```

---

## Profile-Specific Config Files

Spring Boot automatically loads profile-specific property files:

```text
application.yml             ← always loaded (base config)
application-dev.yml         ← loaded ONLY when "dev" is active
application-prod.yml        ← loaded ONLY when "prod" is active
application-test.yml        ← loaded ONLY when "test" is active
application-dev,local.yml   ← loaded when BOTH "dev" AND "local" are active
```

Profile-specific values OVERRIDE the base `application.yml` values.

---

## Activation Methods

```bash
# 1. Command-line argument (highest priority)
java -jar app.jar --spring.profiles.active=prod

# 2. Environment variable (common in Docker/K8s)
export SPRING_PROFILES_ACTIVE=prod
java -jar app.jar

# 3. JVM system property
java -Dspring.profiles.active=prod -jar app.jar

# 4. application.yml (default profile)
spring:
  profiles:
    active: dev   # ← can be overridden by any of the above
```

---

## Profile-Specific Beans

```java
// Only created when "dev" profile is active
@Configuration
@Profile("dev")
public class DevDataConfig {
    @Bean
    public DataSource dataSource() {
        return new EmbeddedDatabaseBuilder()
            .setType(H2)
            .build();
    }
}

// Only created when "prod" profile is active
@Configuration
@Profile("prod")
public class ProdDataConfig {
    @Bean
    public DataSource dataSource() {
        HikariDataSource ds = new HikariDataSource();
        ds.setJdbcUrl(System.getenv("DB_URL"));  // from env var
        return ds;
    }
}

// Negative profile: active when "prod" is NOT active
@Service
@Profile("!prod")
public class MockEmailService implements EmailService { }

// Multiple profiles: active when "dev" OR "test" is active
@Service
@Profile({"dev", "test"})
public class StubPaymentService implements PaymentService { }
```

---

## Profile Groups (Spring Boot 2.4+)

```yaml
spring:
  profiles:
    group:
      production:        # activating "production" activates all of these:
        - proddb
        - prodmq
        - prodcache
      local:
        - devdb
        - devtools
```

---

## Default Profile

```yaml
spring:
  profiles:
    default: dev   # used if no profile is explicitly activated
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `profile_resolver.c` | Active profile detection + property file selection |
| `profile_config_merge.c` | Property merging: base + profile-specific override |

---

## Build and Run

```bash
gcc -Wall -Wextra -o profile_resolver profile_resolver.c && ./profile_resolver
```

---

## Key Insight

```text
Profiles are just a NAME that Spring uses as a FILTER:
  - Which config files to load
  - Which @Profile beans to create
  - Which auto-configurations to skip

The profile name itself means nothing to Spring Boot.
YOU define what "dev" means vs "prod".
Convention: dev, test, staging, prod — but any name works.

For security: NEVER hardcode production credentials in application-prod.yml.
Use: environment variables, Vault, AWS Secrets Manager, etc.
```
