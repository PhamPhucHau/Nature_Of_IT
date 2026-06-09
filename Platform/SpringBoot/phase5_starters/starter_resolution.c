/*
 * starter_resolution.c  —  Spring Boot Starter Resolution in C
 *
 * Build:  gcc -Wall -Wextra -o starter_resolution starter_resolution.c
 * Run:    ./starter_resolution
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   The process by which a Spring Boot starter:
 *     1. Declares transitive dependencies (pom.xml)
 *     2. Those dependencies land on the classpath
 *     3. Classpath presence triggers @ConditionalOnClass auto-configs
 *     4. Auto-configs create the right beans automatically
 *
 *   This is the CHAIN that makes "just add a starter and it works":
 *
 *     User adds: spring-boot-starter-web
 *       ↓ maven pulls in: spring-webmvc, tomcat-embed, jackson
 *       ↓ classpath now has: DispatcherServlet.class, HikariDataSource.class
 *       ↓ auto-config evaluates: @ConditionalOnClass(DispatcherServlet.class) → PASS
 *       ↓ WebMvcAutoConfiguration activates → creates ~20 beans
 *       ↓ TomcatServletWebServerFactory activates → starts server
 *
 * C SIMULATION:
 *   Each starter = a list of "library IDs" it provides.
 *   Each auto-config = a list of "library IDs" it requires on classpath.
 *   The engine: collect all library IDs from selected starters,
 *               then activate all auto-configs whose requirements are met.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  LIBRARY IDs — represent classes on the classpath
 *  When a starter is included, its libraries land on the classpath.
 *  Auto-configs use @ConditionalOnClass to check for these.
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    LIB_SPRING_CORE         = 1 << 0,
    LIB_SPRING_CONTEXT      = 1 << 1,
    LIB_SPRING_WEB          = 1 << 2,
    LIB_SPRING_WEBMVC       = 1 << 3,
    LIB_TOMCAT              = 1 << 4,
    LIB_JACKSON             = 1 << 5,
    LIB_HIBERNATE_JPA       = 1 << 6,
    LIB_HIKARICP            = 1 << 7,
    LIB_SPRING_DATA_JPA     = 1 << 8,
    LIB_SPRING_SECURITY     = 1 << 9,
    LIB_SPRING_ACTUATOR     = 1 << 10,
    LIB_MICROMETER          = 1 << 11,
    LIB_REDIS_CLIENT        = 1 << 12,
    LIB_SPRING_DATA_REDIS   = 1 << 13,
    LIB_LOGBACK             = 1 << 14,
    LIB_SPRING_AOP          = 1 << 15,
    LIB_ASPECTJ             = 1 << 16,
    LIB_H2_DATABASE         = 1 << 17,
    LIB_FLYWAY              = 1 << 18,
    LIB_THYMELEAF           = 1 << 19,
    LIB_VALIDATION_API      = 1 << 20,
    LIB_HIBERNATE_VALIDATOR = 1 << 21,
    LIB_SNAKEYAML           = 1 << 22,
} LibraryId;

/* ═══════════════════════════════════════════════════════════
 *  STARTER DEFINITION
 *  Models a spring-boot-starter-* pom.xml.
 *  The key field is 'provides' — bitmask of what ends up on classpath.
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    const char *artifact_id;
    const char *description;
    unsigned int provides;       /* bitmask of LibraryId values */
    const char *auto_configs[8]; /* auto-configuration classes it enables */
    int         auto_config_count;
} Starter;

static const Starter STARTERS[] = {
    {
        .artifact_id  = "spring-boot-starter",
        .description  = "Core starter: logging, auto-config, YAML",
        .provides     = LIB_SPRING_CORE | LIB_SPRING_CONTEXT |
                        LIB_LOGBACK | LIB_SNAKEYAML,
        .auto_configs = { "PropertySourcesAutoConfiguration",
                          "LoggingAutoConfiguration" },
        .auto_config_count = 2,
    },
    {
        .artifact_id  = "spring-boot-starter-web",
        .description  = "Spring MVC + Tomcat + Jackson",
        .provides     = LIB_SPRING_CORE | LIB_SPRING_CONTEXT |
                        LIB_SPRING_WEB  | LIB_SPRING_WEBMVC  |
                        LIB_TOMCAT      | LIB_JACKSON         |
                        LIB_LOGBACK     | LIB_SNAKEYAML,
        .auto_configs = { "WebMvcAutoConfiguration",
                          "DispatcherServletAutoConfiguration",
                          "JacksonAutoConfiguration",
                          "EmbeddedWebServerFactoryCustomizerAutoConfiguration" },
        .auto_config_count = 4,
    },
    {
        .artifact_id  = "spring-boot-starter-data-jpa",
        .description  = "JPA + Hibernate + HikariCP connection pool",
        .provides     = LIB_SPRING_CORE | LIB_SPRING_CONTEXT |
                        LIB_HIBERNATE_JPA | LIB_HIKARICP |
                        LIB_SPRING_DATA_JPA,
        .auto_configs = { "DataSourceAutoConfiguration",
                          "HibernateJpaAutoConfiguration",
                          "JpaRepositoriesAutoConfiguration",
                          "TransactionAutoConfiguration" },
        .auto_config_count = 4,
    },
    {
        .artifact_id  = "spring-boot-starter-security",
        .description  = "Spring Security authentication and authorization",
        .provides     = LIB_SPRING_SECURITY | LIB_SPRING_AOP,
        .auto_configs = { "SecurityAutoConfiguration",
                          "SecurityFilterAutoConfiguration",
                          "UserDetailsServiceAutoConfiguration" },
        .auto_config_count = 3,
    },
    {
        .artifact_id  = "spring-boot-starter-actuator",
        .description  = "Production monitoring endpoints",
        .provides     = LIB_SPRING_ACTUATOR | LIB_MICROMETER,
        .auto_configs = { "HealthEndpointAutoConfiguration",
                          "InfoEndpointAutoConfiguration",
                          "MetricsAutoConfiguration",
                          "EndpointAutoConfiguration" },
        .auto_config_count = 4,
    },
    {
        .artifact_id  = "spring-boot-starter-data-redis",
        .description  = "Spring Data Redis + Lettuce client",
        .provides     = LIB_REDIS_CLIENT | LIB_SPRING_DATA_REDIS,
        .auto_configs = { "RedisAutoConfiguration",
                          "RedisRepositoriesAutoConfiguration" },
        .auto_config_count = 2,
    },
    {
        .artifact_id  = "spring-boot-starter-aop",
        .description  = "Spring AOP + AspectJ",
        .provides     = LIB_SPRING_AOP | LIB_ASPECTJ,
        .auto_configs = { "AopAutoConfiguration" },
        .auto_config_count = 1,
    },
    {
        .artifact_id  = "spring-boot-starter-validation",
        .description  = "Bean Validation (Jakarta) + Hibernate Validator",
        .provides     = LIB_VALIDATION_API | LIB_HIBERNATE_VALIDATOR,
        .auto_configs = { "ValidationAutoConfiguration" },
        .auto_config_count = 1,
    },
    {
        .artifact_id  = "flyway-core",
        .description  = "Database migration (not a Boot starter, but common)",
        .provides     = LIB_FLYWAY,
        .auto_configs = { "FlywayAutoConfiguration" },
        .auto_config_count = 1,
    },
};

static int starter_count = (int)(sizeof(STARTERS) / sizeof(STARTERS[0]));

/* ─────────────────────────────────────────────────────────────
 *  Library name lookup for display
 * ──────────────────────────────────────────────────────────── */
static void print_libraries(unsigned int mask) {
    struct { LibraryId id; const char *name; } libs[] = {
        {LIB_SPRING_CORE,         "spring-core"},
        {LIB_SPRING_CONTEXT,      "spring-context"},
        {LIB_SPRING_WEB,          "spring-web"},
        {LIB_SPRING_WEBMVC,       "spring-webmvc"},
        {LIB_TOMCAT,              "tomcat-embed-core"},
        {LIB_JACKSON,             "jackson-databind"},
        {LIB_HIBERNATE_JPA,       "hibernate-core"},
        {LIB_HIKARICP,            "HikariCP"},
        {LIB_SPRING_DATA_JPA,     "spring-data-jpa"},
        {LIB_SPRING_SECURITY,     "spring-security-core"},
        {LIB_SPRING_ACTUATOR,     "spring-boot-actuator"},
        {LIB_MICROMETER,          "micrometer-core"},
        {LIB_REDIS_CLIENT,        "lettuce-core"},
        {LIB_SPRING_DATA_REDIS,   "spring-data-redis"},
        {LIB_LOGBACK,             "logback-classic"},
        {LIB_SPRING_AOP,          "spring-aop"},
        {LIB_ASPECTJ,             "aspectjweaver"},
        {LIB_H2_DATABASE,         "h2"},
        {LIB_FLYWAY,              "flyway-core"},
        {LIB_THYMELEAF,           "thymeleaf"},
        {LIB_VALIDATION_API,      "jakarta.validation-api"},
        {LIB_HIBERNATE_VALIDATOR, "hibernate-validator"},
        {LIB_SNAKEYAML,           "snakeyaml"},
    };
    for (int i = 0; i < 23; i++) {
        if (mask & libs[i].id) {
            printf("    + %s\n", libs[i].name);
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 *  Resolve: given selected starters → compute classpath + active auto-configs
 * ──────────────────────────────────────────────────────────── */
static void resolve_starters(const char **selected_ids, int selected_count) {
    unsigned int classpath = 0;

    /* Collected auto-configs (deduplicated) */
    const char *active_autoconfigs[64];
    int         autoconfig_count = 0;

    printf("Selected starters:\n");
    for (int s = 0; s < selected_count; s++) {
        /* Find the starter */
        const Starter *st = NULL;
        for (int i = 0; i < starter_count; i++) {
            if (strcmp(STARTERS[i].artifact_id, selected_ids[s]) == 0) {
                st = &STARTERS[i];
                break;
            }
        }
        if (!st) {
            printf("  WARNING: Unknown starter '%s'\n", selected_ids[s]);
            continue;
        }

        printf("  ✓ %-45s — %s\n", st->artifact_id, st->description);

        /* Accumulate classpath */
        classpath |= st->provides;

        /* Collect auto-configs (deduplicate) */
        for (int a = 0; a < st->auto_config_count; a++) {
            int dup = 0;
            for (int k = 0; k < autoconfig_count; k++) {
                if (strcmp(active_autoconfigs[k], st->auto_configs[a]) == 0) {
                    dup = 1; break;
                }
            }
            if (!dup && autoconfig_count < 64) {
                active_autoconfigs[autoconfig_count++] = st->auto_configs[a];
            }
        }
    }

    printf("\nResulting classpath (transitive dependencies):\n");
    print_libraries(classpath);

    printf("\nAuto-configurations triggered (%d):\n", autoconfig_count);
    for (int i = 0; i < autoconfig_count; i++) {
        printf("  ✅ %s\n", active_autoconfigs[i]);
    }
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_typical_rest_api(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Typical REST API Application\n");
    printf("pom.xml: starter-web + starter-data-jpa + starter-actuator\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    const char *selected[] = {
        "spring-boot-starter-web",
        "spring-boot-starter-data-jpa",
        "spring-boot-starter-actuator",
        "spring-boot-starter-validation",
        "spring-boot-starter-aop",
    };
    resolve_starters(selected, 5);
}

void demo_secure_api(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Secured API with Redis Cache\n");
    printf("pom.xml: starter-web + starter-security + starter-data-redis\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    const char *selected[] = {
        "spring-boot-starter-web",
        "spring-boot-starter-security",
        "spring-boot-starter-data-redis",
        "spring-boot-starter-actuator",
    };
    resolve_starters(selected, 4);
}

void demo_starter_detail(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: All Available Starters\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    for (int i = 0; i < starter_count; i++) {
        const Starter *st = &STARTERS[i];
        printf("  %-45s\n", st->artifact_id);
        printf("    %s\n", st->description);
        printf("    Auto-configs: ");
        for (int a = 0; a < st->auto_config_count; a++) {
            printf("%s", st->auto_configs[a]);
            if (a < st->auto_config_count - 1) printf(", ");
        }
        printf("\n\n");
    }
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  starter_resolution.c — Spring Boot Starters in C   ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_typical_rest_api();
    demo_secure_api();
    demo_starter_detail();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  A starter = pom.xml with transitive dependencies, zero code\n");
    printf("  Starters populate the classpath → @ConditionalOnClass passes\n");
    printf("  @ConditionalOnClass passes → auto-configuration activates\n");
    printf("  Auto-configuration activates → beans created automatically\n");
    printf("  BOM manages ~300 dependency versions — no version conflicts\n");
    return 0;
}
