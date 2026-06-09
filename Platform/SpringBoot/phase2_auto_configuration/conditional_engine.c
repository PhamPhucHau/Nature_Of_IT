/*
 * conditional_engine.c  —  Spring Boot @Conditional Evaluation Engine in C
 *
 * Build:  gcc -Wall -Wextra -o conditional_engine conditional_engine.c
 * Run:    ./conditional_engine
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.boot.autoconfigure.condition.*
 *
 *   Every @ConditionalOnXxx annotation is evaluated by a Condition class.
 *   Spring calls Condition.matches(context, metadata) for each condition.
 *   ALL conditions on a class must pass for the auto-config to activate.
 *
 *   The evaluation happens during:
 *     ConfigurationClassPostProcessor.processConfigurationClass()
 *       → ConditionEvaluator.shouldSkip()
 *         → for each condition: condition.matches() → true/false
 *
 *   Key condition types (each a separate Java class in Spring Boot):
 *     OnClassCondition        → @ConditionalOnClass / @ConditionalOnMissingClass
 *     OnBeanCondition         → @ConditionalOnBean / @ConditionalOnMissingBean
 *     OnPropertyCondition     → @ConditionalOnProperty
 *     OnWebApplicationCondition → @ConditionalOnWebApplication
 *     OnExpressionCondition   → @ConditionalOnExpression
 *     ResourceCondition       → @ConditionalOnResource
 *
 * C SIMULATION APPROACH:
 *   Each condition = a struct with type + parameters + a match function.
 *   An auto-config = a list of conditions that ALL must pass.
 *   The engine iterates auto-configs, evaluates all conditions,
 *   reports MATCH or SKIP with full reasoning.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  SIMULATED CLASSPATH
 *  In real Spring Boot, this is the actual JVM classpath.
 *  OnClassCondition uses ClassLoader.loadClass() to check.
 *  We simulate with a string array.
 * ═══════════════════════════════════════════════════════════ */
static const char *CLASSPATH[] = {
    /* Spring Boot core */
    "org.springframework.boot.SpringApplication",
    "org.springframework.boot.autoconfigure.SpringBootApplication",
    /* Web MVC */
    "org.springframework.web.servlet.DispatcherServlet",
    "org.springframework.web.servlet.config.annotation.WebMvcConfigurer",
    /* JDBC / DataSource */
    "javax.sql.DataSource",
    "org.springframework.jdbc.core.JdbcTemplate",
    /* HikariCP */
    "com.zaxxer.hikari.HikariDataSource",
    /* Jackson JSON */
    "com.fasterxml.jackson.databind.ObjectMapper",
    /* AOP */
    "org.aopalliance.intercept.MethodInterceptor",
    "org.springframework.aop.framework.AopContext",
    /* Transactions */
    "org.springframework.transaction.PlatformTransactionManager",
    /* Actuator */
    "org.springframework.boot.actuate.health.HealthEndpoint",
    /* Validation */
    "jakarta.validation.Validator",
    /* NOTE: Thymeleaf NOT present — template engine auto-config will skip */
    /* NOTE: Redis NOT present — Redis auto-config will skip */
    /* NOTE: Kafka NOT present — Kafka auto-config will skip */
};
static int classpath_size = (int)(sizeof(CLASSPATH) / sizeof(CLASSPATH[0]));

/* ═══════════════════════════════════════════════════════════
 *  SIMULATED ENVIRONMENT (properties)
 * ═══════════════════════════════════════════════════════════ */
static const char *PROPERTIES[][2] = {
    { "spring.datasource.url",       "jdbc:h2:mem:testdb"    },
    { "spring.datasource.username",  "sa"                    },
    { "spring.application.name",     "my-spring-app"         },
    { "server.port",                 "8080"                  },
    { "management.endpoints.web.exposure.include", "health,info" },
    { "spring.jmx.enabled",         "false"                  },
    /* NOTE: spring.security.user.name NOT set */
    /* NOTE: spring.redis.host NOT set */
};
static int properties_size = (int)(sizeof(PROPERTIES) / sizeof(PROPERTIES[0]));

/* ═══════════════════════════════════════════════════════════
 *  SIMULATED BEAN REGISTRY
 *  Tracks which beans already exist (for @ConditionalOnMissingBean).
 *  As auto-configs activate, they add beans here.
 * ═══════════════════════════════════════════════════════════ */
#define MAX_BEANS 64
static char registered_beans[MAX_BEANS][128];
static int  bean_count = 0;

static void register_bean(const char *bean_type) {
    if (bean_count < MAX_BEANS) {
        strncpy(registered_beans[bean_count++], bean_type, 127);
    }
}

static int bean_exists(const char *bean_type) {
    for (int i = 0; i < bean_count; i++) {
        if (strcmp(registered_beans[i], bean_type) == 0) return 1;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════
 *  CONDITION TYPES
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    COND_ON_CLASS,            /* @ConditionalOnClass           */
    COND_ON_MISSING_CLASS,    /* @ConditionalOnMissingClass    */
    COND_ON_BEAN,             /* @ConditionalOnBean            */
    COND_ON_MISSING_BEAN,     /* @ConditionalOnMissingBean     */
    COND_ON_PROPERTY,         /* @ConditionalOnProperty        */
    COND_ON_WEB_APP,          /* @ConditionalOnWebApplication  */
    COND_ON_NOT_WEB_APP,      /* @ConditionalOnNotWebApplication */
    COND_ON_RESOURCE,         /* @ConditionalOnResource        */
} ConditionType;

/* ═══════════════════════════════════════════════════════════
 *  WEB APPLICATION TYPE (for OnWebApplicationCondition)
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    WEB_TYPE_ANY,
    WEB_TYPE_SERVLET,
    WEB_TYPE_REACTIVE,
} WebType;

/* ═══════════════════════════════════════════════════════════
 *  CONDITION STRUCT
 *  Represents one @ConditionalOnXxx annotation instance.
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    ConditionType type;
    const char   *param;          /* class name, bean type, property key, etc. */
    const char   *expected_value; /* for OnProperty: havingValue               */
    int           match_if_missing; /* OnProperty: matchIfMissing               */
    WebType       web_type;       /* for OnWebApplication                       */
} Condition;

/* ═══════════════════════════════════════════════════════════
 *  AUTO-CONFIGURATION DESCRIPTOR
 *  Represents one entry in AutoConfiguration.imports
 * ═══════════════════════════════════════════════════════════ */
#define MAX_CONDITIONS 6
#define MAX_BEANS_CREATED 4

typedef struct {
    const char *config_class;              /* fully qualified class name         */
    const char *short_name;               /* for display                        */
    Condition   conditions[MAX_CONDITIONS];
    int         condition_count;
    const char *beans_created[MAX_BEANS_CREATED]; /* beans registered on match  */
    int         beans_created_count;
} AutoConfig;

/* ─────────────────────────────────────────────────────────────
 *  CONDITION EVALUATORS
 *  Each evaluates one condition against current classpath/env/beans.
 *  Returns 1 = PASS, 0 = FAIL, with reason string filled.
 * ──────────────────────────────────────────────────────────── */

static int check_on_class(const char *class_name, char *reason) {
    for (int i = 0; i < classpath_size; i++) {
        if (strcmp(CLASSPATH[i], class_name) == 0) {
            snprintf(reason, 256, "@ConditionalOnClass(%s) → FOUND on classpath",
                     class_name);
            return 1;
        }
    }
    snprintf(reason, 256, "@ConditionalOnClass(%s) → NOT FOUND on classpath",
             class_name);
    return 0;
}

static int check_on_missing_class(const char *class_name, char *reason) {
    for (int i = 0; i < classpath_size; i++) {
        if (strcmp(CLASSPATH[i], class_name) == 0) {
            snprintf(reason, 256, "@ConditionalOnMissingClass(%s) → class IS present (fail)",
                     class_name);
            return 0;
        }
    }
    snprintf(reason, 256, "@ConditionalOnMissingClass(%s) → class absent (pass)",
             class_name);
    return 1;
}

static int check_on_bean(const char *bean_type, char *reason) {
    if (bean_exists(bean_type)) {
        snprintf(reason, 256, "@ConditionalOnBean(%s) → bean EXISTS", bean_type);
        return 1;
    }
    snprintf(reason, 256, "@ConditionalOnBean(%s) → bean NOT found", bean_type);
    return 0;
}

static int check_on_missing_bean(const char *bean_type, char *reason) {
    if (!bean_exists(bean_type)) {
        snprintf(reason, 256,
                 "@ConditionalOnMissingBean(%s) → no bean defined (Boot can create default)",
                 bean_type);
        return 1;
    }
    snprintf(reason, 256,
             "@ConditionalOnMissingBean(%s) → bean ALREADY EXISTS (user override, skip Boot default)",
             bean_type);
    return 0;
}

static int check_on_property(const char *key, const char *expected,
                              int match_if_missing, char *reason) {
    for (int i = 0; i < properties_size; i++) {
        if (strcmp(PROPERTIES[i][0], key) == 0) {
            if (expected == NULL || strcmp(PROPERTIES[i][1], expected) == 0) {
                snprintf(reason, 256,
                         "@ConditionalOnProperty(\"%s\") → set to '%s' (pass)",
                         key, PROPERTIES[i][1]);
                return 1;
            } else {
                snprintf(reason, 256,
                         "@ConditionalOnProperty(\"%s\", havingValue=\"%s\") → actual='%s' (fail)",
                         key, expected, PROPERTIES[i][1]);
                return 0;
            }
        }
    }
    /* property not found */
    if (match_if_missing) {
        snprintf(reason, 256,
                 "@ConditionalOnProperty(\"%s\") → not set, matchIfMissing=true (pass)", key);
        return 1;
    }
    snprintf(reason, 256,
             "@ConditionalOnProperty(\"%s\") → not set, matchIfMissing=false (fail)", key);
    return 0;
}

static int check_on_web_app(WebType wt, char *reason) {
    /* Simulate: we ARE a SERVLET web application */
    int is_servlet  = 1;
    int is_reactive = 0;

    if (wt == WEB_TYPE_ANY) {
        snprintf(reason, 256, "@ConditionalOnWebApplication(ANY) → web app present (pass)");
        return is_servlet || is_reactive;
    }
    if (wt == WEB_TYPE_SERVLET) {
        if (is_servlet) {
            snprintf(reason, 256,
                     "@ConditionalOnWebApplication(SERVLET) → SERVLET app detected (pass)");
            return 1;
        }
        snprintf(reason, 256,
                 "@ConditionalOnWebApplication(SERVLET) → not a SERVLET app (fail)");
        return 0;
    }
    if (wt == WEB_TYPE_REACTIVE) {
        snprintf(reason, 256,
                 "@ConditionalOnWebApplication(REACTIVE) → not reactive (fail)");
        return 0;
    }
    return 0;
}

static int evaluate_condition(const Condition *c, char *reason) {
    switch (c->type) {
        case COND_ON_CLASS:
            return check_on_class(c->param, reason);
        case COND_ON_MISSING_CLASS:
            return check_on_missing_class(c->param, reason);
        case COND_ON_BEAN:
            return check_on_bean(c->param, reason);
        case COND_ON_MISSING_BEAN:
            return check_on_missing_bean(c->param, reason);
        case COND_ON_PROPERTY:
            return check_on_property(c->param, c->expected_value,
                                     c->match_if_missing, reason);
        case COND_ON_WEB_APP:
            return check_on_web_app(c->web_type, reason);
        case COND_ON_NOT_WEB_APP:
            snprintf(reason, 256, "@ConditionalOnNotWebApplication → IS a web app (fail)");
            return 0;
        case COND_ON_RESOURCE:
            snprintf(reason, 256, "@ConditionalOnResource(%s) → assumed present", c->param);
            return 1;
        default:
            snprintf(reason, 256, "Unknown condition type");
            return 0;
    }
}

/* ─────────────────────────────────────────────────────────────
 *  EVALUATE ONE AUTO-CONFIG
 *  Returns 1 = ACTIVATED, 0 = SKIPPED
 * ──────────────────────────────────────────────────────────── */
static int evaluate_auto_config(const AutoConfig *ac, int verbose) {
    if (verbose) {
        printf("  ┌─ %s\n", ac->short_name);
    }

    int all_pass = 1;
    char reason[256];

    for (int i = 0; i < ac->condition_count; i++) {
        int pass = evaluate_condition(&ac->conditions[i], reason);
        if (verbose) {
            printf("  │  %s %s\n", pass ? "✓" : "✗", reason);
        }
        if (!pass) {
            all_pass = 0;
            if (!verbose) break;  /* short-circuit if not verbose */
        }
    }

    if (all_pass) {
        /* Register the beans this config creates */
        for (int i = 0; i < ac->beans_created_count; i++) {
            register_bean(ac->beans_created[i]);
        }
        if (verbose) {
            printf("  │\n");
            printf("  │  ✅ ACTIVATED → creating beans:\n");
            for (int i = 0; i < ac->beans_created_count; i++) {
                printf("  │     → %s\n", ac->beans_created[i]);
            }
        }
    } else {
        if (verbose) {
            printf("  │\n");
            printf("  │  ⏭  SKIPPED (condition failed)\n");
        }
    }

    if (verbose) printf("  └──────────────────────────────────────────────────\n\n");

    return all_pass;
}

/* ═══════════════════════════════════════════════════════════
 *  AUTO-CONFIGURATION REGISTRY
 *  Simulates the 150+ entries in AutoConfiguration.imports.
 *  We define 12 representative ones.
 *
 *  Java source reference for each:
 *    spring-boot-autoconfigure/src/main/java/org/springframework/
 *    boot/autoconfigure/<module>/<Name>AutoConfiguration.java
 * ═══════════════════════════════════════════════════════════ */
static AutoConfig AUTO_CONFIGS[] = {

    /* ── 1. DataSourceAutoConfiguration ── */
    {
        .config_class = "org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration",
        .short_name   = "DataSourceAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS, "javax.sql.DataSource", NULL, 0 },
            { COND_ON_MISSING_BEAN, "javax.sql.DataSource", NULL, 0 },
        },
        .condition_count    = 2,
        .beans_created      = { "javax.sql.DataSource", "com.zaxxer.hikari.HikariDataSource" },
        .beans_created_count = 2,
    },

    /* ── 2. HikariDataSourceConfiguration ── */
    {
        .config_class = "org.springframework.boot.autoconfigure.jdbc.DataSourceConfiguration$Hikari",
        .short_name   = "HikariDataSourceConfiguration",
        .conditions   = {
            { COND_ON_CLASS,    "com.zaxxer.hikari.HikariDataSource", NULL, 0 },
            { COND_ON_PROPERTY, "spring.datasource.url", NULL, 0 },
        },
        .condition_count    = 2,
        .beans_created      = { "hikariDataSource" },
        .beans_created_count = 1,
    },

    /* ── 3. JdbcTemplateAutoConfiguration ── */
    {
        .config_class = "org.springframework.boot.autoconfigure.jdbc.JdbcTemplateAutoConfiguration",
        .short_name   = "JdbcTemplateAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS,   "org.springframework.jdbc.core.JdbcTemplate", NULL, 0 },
            { COND_ON_BEAN,    "javax.sql.DataSource", NULL, 0 },
            { COND_ON_MISSING_BEAN, "org.springframework.jdbc.core.JdbcTemplate", NULL, 0 },
        },
        .condition_count    = 3,
        .beans_created      = { "jdbcTemplate", "namedParameterJdbcTemplate" },
        .beans_created_count = 2,
    },

    /* ── 4. WebMvcAutoConfiguration ── */
    {
        .config_class = "org.springframework.boot.autoconfigure.web.servlet.WebMvcAutoConfiguration",
        .short_name   = "WebMvcAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS,   "org.springframework.web.servlet.DispatcherServlet", NULL, 0 },
            { COND_ON_WEB_APP, NULL, NULL, 0, WEB_TYPE_SERVLET },
            { COND_ON_MISSING_BEAN, "org.springframework.web.servlet.config.annotation.WebMvcConfigurer", NULL, 0 },
        },
        .condition_count    = 3,
        .beans_created      = { "requestMappingHandlerMapping", "requestMappingHandlerAdapter",
                                 "handlerExceptionResolver", "viewResolver" },
        .beans_created_count = 4,
    },

    /* ── 5. DispatcherServletAutoConfiguration ── */
    {
        .config_class = "org.springframework.boot.autoconfigure.web.servlet.DispatcherServletAutoConfiguration",
        .short_name   = "DispatcherServletAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS,   "org.springframework.web.servlet.DispatcherServlet", NULL, 0 },
            { COND_ON_WEB_APP, NULL, NULL, 0, WEB_TYPE_SERVLET },
        },
        .condition_count    = 2,
        .beans_created      = { "dispatcherServlet", "dispatcherServletRegistration" },
        .beans_created_count = 2,
    },

    /* ── 6. JacksonAutoConfiguration ── */
    {
        .config_class = "org.springframework.boot.autoconfigure.jackson.JacksonAutoConfiguration",
        .short_name   = "JacksonAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS, "com.fasterxml.jackson.databind.ObjectMapper", NULL, 0 },
        },
        .condition_count    = 1,
        .beans_created      = { "objectMapper", "jacksonObjectMapper" },
        .beans_created_count = 2,
    },

    /* ── 7. TransactionAutoConfiguration ── */
    {
        .config_class = "org.springframework.boot.autoconfigure.transaction.TransactionAutoConfiguration",
        .short_name   = "TransactionAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS, "org.springframework.transaction.PlatformTransactionManager", NULL, 0 },
        },
        .condition_count    = 1,
        .beans_created      = { "transactionManager", "transactionTemplate" },
        .beans_created_count = 2,
    },

    /* ── 8. AopAutoConfiguration ── */
    {
        .config_class = "org.springframework.boot.autoconfigure.aop.AopAutoConfiguration",
        .short_name   = "AopAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS,    "org.aopalliance.intercept.MethodInterceptor", NULL, 0 },
            { COND_ON_PROPERTY, "spring.aop.auto", NULL, 1 /* matchIfMissing=true */ },
        },
        .condition_count    = 2,
        .beans_created      = { "org.springframework.aop.config.internalAutoProxyCreator" },
        .beans_created_count = 1,
    },

    /* ── 9. ThymeleafAutoConfiguration ── WILL SKIP (Thymeleaf not on classpath) */
    {
        .config_class = "org.springframework.boot.autoconfigure.thymeleaf.ThymeleafAutoConfiguration",
        .short_name   = "ThymeleafAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS, "org.thymeleaf.spring6.SpringTemplateEngine", NULL, 0 },
            { COND_ON_WEB_APP, NULL, NULL, 0, WEB_TYPE_ANY },
        },
        .condition_count    = 2,
        .beans_created      = { "templateEngine", "thymeleafViewResolver" },
        .beans_created_count = 2,
    },

    /* ── 10. RedisAutoConfiguration ── WILL SKIP (Redis not on classpath) */
    {
        .config_class = "org.springframework.boot.autoconfigure.data.redis.RedisAutoConfiguration",
        .short_name   = "RedisAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS, "org.springframework.data.redis.core.RedisOperations", NULL, 0 },
        },
        .condition_count    = 1,
        .beans_created      = { "redisTemplate", "stringRedisTemplate" },
        .beans_created_count = 2,
    },

    /* ── 11. ActuatorAutoConfiguration ── */
    {
        .config_class = "org.springframework.boot.actuate.autoconfigure.health.HealthEndpointAutoConfiguration",
        .short_name   = "HealthEndpointAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS, "org.springframework.boot.actuate.health.HealthEndpoint", NULL, 0 },
        },
        .condition_count    = 1,
        .beans_created      = { "healthEndpoint", "healthEndpointWebExtension" },
        .beans_created_count = 2,
    },

    /* ── 12. JmxAutoConfiguration ── WILL SKIP (spring.jmx.enabled=false) */
    {
        .config_class = "org.springframework.boot.autoconfigure.jmx.JmxAutoConfiguration",
        .short_name   = "JmxAutoConfiguration",
        .conditions   = {
            { COND_ON_CLASS,    "org.springframework.jmx.export.MBeanExporter", NULL, 0 },
            { COND_ON_PROPERTY, "spring.jmx.enabled", "true", 0 },
        },
        .condition_count    = 2,
        .beans_created      = { "mbeanServer", "mbeanExporter" },
        .beans_created_count = 2,
    },
};

static int auto_config_count = (int)(sizeof(AUTO_CONFIGS) / sizeof(AUTO_CONFIGS[0]));

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_full_evaluation(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Full Auto-Configuration Evaluation\n");
    printf("Simulating: invokeBeanFactoryPostProcessors → @EnableAutoConfiguration\n");
    printf("            → ConditionEvaluator.shouldSkip() per auto-config\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    printf("Classpath has: spring-webmvc, HikariCP, Jackson, H2, Actuator\n");
    printf("Missing from classpath: Thymeleaf, Redis, Kafka\n");
    printf("Properties set: spring.datasource.url, server.port\n");
    printf("spring.jmx.enabled = false (explicitly)\n\n");

    int activated = 0, skipped = 0;

    for (int i = 0; i < auto_config_count; i++) {
        int result = evaluate_auto_config(&AUTO_CONFIGS[i], 1);
        if (result) activated++;
        else        skipped++;
    }

    printf("━━━ Summary ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Total evaluated:  %d\n", auto_config_count);
    printf("  Activated (MATCH): %d\n", activated);
    printf("  Skipped (FAIL):    %d\n", skipped);
    printf("  Beans created:    %d\n", bean_count);
    printf("\n  Registered beans:\n");
    for (int i = 0; i < bean_count; i++) {
        printf("    ✓ %s\n", registered_beans[i]);
    }
    printf("\n");
}

void demo_user_override(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: User Override via @ConditionalOnMissingBean\n");
    printf("User defined their own DataSource → Boot skips DataSourceAutoConfiguration\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Reset beans and pre-register a user-defined DataSource */
    bean_count = 0;
    register_bean("javax.sql.DataSource");   /* user's custom DataSource @Bean */

    printf("User already registered: javax.sql.DataSource\n\n");
    printf("Evaluating DataSourceAutoConfiguration:\n");
    evaluate_auto_config(&AUTO_CONFIGS[0], 1);

    printf("Result: Boot's DataSource was NOT created (user's DataSource takes priority)\n");
    printf("This is @ConditionalOnMissingBean in action — Boot never overrides you.\n\n");

    bean_count = 0;  /* reset for next demo */
}

void demo_condition_report(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: Condition Evaluation Report (like --debug flag)\n");
    printf("Spring Boot: java -jar app.jar --debug\n");
    printf("Outputs: 'CONDITIONS EVALUATION REPORT' in console\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    printf("CONDITIONS EVALUATION REPORT\n");
    printf("============================\n\n");

    printf("Positive matches (activated):\n");
    printf("─────────────────────────────\n");
    bean_count = 0;
    for (int i = 0; i < auto_config_count; i++) {
        /* Quick pass to find activated ones */
        int all_pass = 1;
        char reason[256];
        for (int j = 0; j < AUTO_CONFIGS[i].condition_count; j++) {
            if (!evaluate_condition(&AUTO_CONFIGS[i].conditions[j], reason)) {
                all_pass = 0;
                break;
            }
        }
        if (all_pass) {
            printf("   %s\n", AUTO_CONFIGS[i].short_name);
            for (int j = 0; j < AUTO_CONFIGS[i].condition_count; j++) {
                evaluate_condition(&AUTO_CONFIGS[i].conditions[j], reason);
                printf("      - %s\n", reason);
            }
            for (int k = 0; k < AUTO_CONFIGS[i].beans_created_count; k++) {
                register_bean(AUTO_CONFIGS[i].beans_created[k]);
            }
            printf("\n");
        }
    }

    printf("Negative matches (skipped):\n");
    printf("───────────────────────────\n");
    bean_count = 0;
    for (int i = 0; i < auto_config_count; i++) {
        int all_pass = 1;
        char first_fail[256] = {0};
        for (int j = 0; j < AUTO_CONFIGS[i].condition_count; j++) {
            char reason[256];
            if (!evaluate_condition(&AUTO_CONFIGS[i].conditions[j], reason)) {
                all_pass = 0;
                strncpy(first_fail, reason, 255);
                break;
            }
        }
        if (!all_pass) {
            printf("   %s\n", AUTO_CONFIGS[i].short_name);
            printf("      - %s\n\n", first_fail);
        }
    }

    bean_count = 0;
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  conditional_engine.c — @Conditional Evaluation     ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_full_evaluation();
    demo_user_override();
    demo_condition_report();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Auto-config = a list of conditions that ALL must pass\n");
    printf("  @ConditionalOnClass     → checks classpath (fast, no class loading)\n");
    printf("  @ConditionalOnMissingBean → user override wins over Boot default\n");
    printf("  @ConditionalOnProperty  → enables features via application.yml\n");
    printf("  Run with --debug to see the full condition evaluation report\n");
    printf("  Conditions are evaluated in bean definition phase (before instantiation)\n");
    return 0;
}
