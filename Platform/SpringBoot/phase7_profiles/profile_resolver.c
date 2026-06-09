/*
 * profile_resolver.c  —  Spring Boot Profiles in C
 *
 * Build:  gcc -Wall -Wextra -o profile_resolver profile_resolver.c
 * Run:    ./profile_resolver
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.core.env.AbstractEnvironment
 *   org.springframework.boot.context.config.ConfigDataEnvironmentPostProcessor
 *
 *   Spring Boot profile resolution:
 *     1. Check command-line: --spring.profiles.active=prod
 *     2. Check OS env var:   SPRING_PROFILES_ACTIVE=prod
 *     3. Check JVM prop:     -Dspring.profiles.active=prod
 *     4. Check application.yml: spring.profiles.active: dev
 *     5. Use default: "default"
 *
 *   Then for active profile(s):
 *     - Load base application.yml
 *     - Load application-{profile}.yml (overrides base)
 *     - Filter @Profile beans: only create beans matching active profile
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  PROPERTY MAP (flat key=value store)
 * ═══════════════════════════════════════════════════════════ */
#define MAX_PROPS 64

typedef struct {
    char key[128];
    char value[256];
    const char *source;   /* "base", "dev", "prod", "env-var", "cli" */
} Prop;

typedef struct {
    Prop entries[MAX_PROPS];
    int  count;
} PropMap;

static void prop_put(PropMap *m, const char *k, const char *v, const char *src) {
    /* Overwrite if key exists */
    for (int i = 0; i < m->count; i++) {
        if (strcmp(m->entries[i].key, k) == 0) {
            strncpy(m->entries[i].value, v, 255);
            m->entries[i].source = src;
            return;
        }
    }
    if (m->count >= MAX_PROPS) return;
    strncpy(m->entries[m->count].key,   k, 127);
    strncpy(m->entries[m->count].value, v, 255);
    m->entries[m->count].source = src;
    m->count++;
}

static const char* prop_get(const PropMap *m, const char *k, const char *def) {
    for (int i = 0; i < m->count; i++) {
        if (strcmp(m->entries[i].key, k) == 0) return m->entries[i].value;
    }
    return def;
}

/* ═══════════════════════════════════════════════════════════
 *  PROFILE CONFIG — represents one application-{profile}.yml
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    const char *profile_name;
    const char *keys[MAX_PROPS];
    const char *values[MAX_PROPS];
    int         count;
} ProfileConfig;

/* ═══════════════════════════════════════════════════════════
 *  PROFILE BEAN — represents a @Profile annotated bean
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    const char *bean_name;
    const char *class_name;
    const char *profiles[4];  /* required profiles (OR logic if multiple) */
    int         profile_count;
    int         negated;       /* 1 = !profile (active when profile NOT active) */
} ProfileBean;

/* ─────────────────────────────────────────────────────────────
 *  Profile match check
 *  Java: AbstractEnvironment.matchesProfiles()
 * ──────────────────────────────────────────────────────────── */
static int profile_matches(const char *active_profile,
                            const ProfileBean *bean)
{
    for (int i = 0; i < bean->profile_count; i++) {
        int matches = strcmp(bean->profiles[i], active_profile) == 0;
        if (!bean->negated && matches) return 1;     /* @Profile("dev") → match */
        if (bean->negated  && !matches) return 1;    /* @Profile("!prod") → match */
    }
    return 0;
}

/* ─────────────────────────────────────────────────────────────
 *  Load profile-specific properties (simulates reading application-{profile}.yml)
 * ──────────────────────────────────────────────────────────── */
static void load_profile_config(PropMap *env,
                                 const char *profile,
                                 const ProfileConfig configs[],
                                 int config_count)
{
    for (int i = 0; i < config_count; i++) {
        if (strcmp(configs[i].profile_name, profile) == 0) {
            printf("  [ConfigData] Loading application-%s.yml (%d properties)\n",
                   profile, configs[i].count);
            for (int j = 0; j < configs[i].count; j++) {
                prop_put(env, configs[i].keys[j], configs[i].values[j], profile);
                printf("    override: %-35s = '%s'\n",
                       configs[i].keys[j], configs[i].values[j]);
            }
            return;
        }
    }
    printf("  [ConfigData] application-%s.yml not found (no overrides)\n", profile);
}

/* ─────────────────────────────────────────────────────────────
 *  Print the final resolved environment
 * ──────────────────────────────────────────────────────────── */
static void print_environment(const PropMap *env, const char *profile) {
    printf("\n  ━━━ Resolved Environment (profile: %s) ━━━━━━━━━━━━━━\n",
           profile);
    printf("    %-35s %-25s %s\n", "Key", "Value", "Source");
    printf("    %-35s %-25s %s\n",
           "───────────────────────────────────",
           "─────────────────────────",
           "──────");
    for (int i = 0; i < env->count; i++) {
        /* Mask passwords */
        const char *display_val = env->entries[i].value;
        if (strstr(env->entries[i].key, "password") ||
            strstr(env->entries[i].key, "secret")   ||
            strstr(env->entries[i].key, "key")) {
            display_val = "***";
        }
        printf("    %-35s %-25s [%s]\n",
               env->entries[i].key,
               display_val,
               env->entries[i].source);
    }
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Evaluate which beans are active for the given profile
 * ──────────────────────────────────────────────────────────── */
static void evaluate_profile_beans(const char *active_profile,
                                    const ProfileBean beans[],
                                    int bean_count)
{
    printf("  ━━━ @Profile Bean Evaluation (active: '%s') ━━━━━━━\n",
           active_profile);
    printf("    %-32s %-32s %s\n", "Bean", "Class", "Result");
    printf("    %-32s %-32s %s\n",
           "────────────────────────────────",
           "────────────────────────────────",
           "──────");

    for (int i = 0; i < bean_count; i++) {
        int active = profile_matches(active_profile, &beans[i]);
        printf("    %-32s %-32s %s\n",
               beans[i].bean_name,
               beans[i].class_name,
               active ? "✅ CREATED" : "⏭  SKIPPED");
    }
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO DATA
 * ═══════════════════════════════════════════════════════════ */

/* Base application.yml properties */
static void load_base_config(PropMap *env) {
    prop_put(env, "spring.application.name",    "my-service",              "base");
    prop_put(env, "server.port",                "8080",                    "base");
    prop_put(env, "logging.level.root",         "INFO",                    "base");
    prop_put(env, "spring.datasource.url",      "jdbc:h2:mem:testdb",      "base");
    prop_put(env, "spring.datasource.username", "sa",                      "base");
    prop_put(env, "spring.datasource.password", "",                        "base");
    prop_put(env, "spring.jpa.show-sql",        "false",                   "base");
    prop_put(env, "management.endpoint.health.show-details", "never",      "base");
    prop_put(env, "myapp.feature.enabled",      "false",                   "base");
    prop_put(env, "myapp.email.mock",           "true",                    "base");
}

/* Profile-specific configs (application-{profile}.yml) */
static const ProfileConfig PROFILE_CONFIGS[] = {
    {
        .profile_name = "dev",
        .keys   = { "server.port",
                    "logging.level.root",
                    "logging.level.com.example",
                    "spring.jpa.show-sql",
                    "management.endpoint.health.show-details",
                    "myapp.feature.enabled",
                    "spring.devtools.restart.enabled" },
        .values = { "8080",
                    "DEBUG",
                    "TRACE",
                    "true",
                    "always",
                    "true",
                    "true" },
        .count = 7
    },
    {
        .profile_name = "test",
        .keys   = { "spring.datasource.url",
                    "spring.datasource.username",
                    "spring.jpa.hibernate.ddl-auto",
                    "logging.level.root",
                    "myapp.email.mock" },
        .values = { "jdbc:h2:mem:testdb;MODE=MySQL",
                    "test",
                    "create-drop",
                    "WARN",
                    "true" },
        .count = 5
    },
    {
        .profile_name = "prod",
        .keys   = { "server.port",
                    "spring.datasource.url",
                    "spring.datasource.username",
                    "spring.datasource.password",
                    "spring.datasource.hikari.maximum-pool-size",
                    "logging.level.root",
                    "server.ssl.enabled",
                    "management.server.port",
                    "myapp.email.mock",
                    "myapp.feature.enabled" },
        .values = { "443",
                    "${DB_URL}",           /* reads from env var at runtime */
                    "${DB_USERNAME}",
                    "${DB_PASSWORD}",
                    "50",
                    "WARN",
                    "true",
                    "8081",               /* separate management port */
                    "false",              /* use real email in prod */
                    "true" },
        .count = 10
    },
};
static int profile_config_count = 3;

/* Beans annotated with @Profile */
static const ProfileBean PROFILE_BEANS[] = {
    /* @Profile("dev") — in-memory H2 data source for development */
    { "devDataSourceConfig",    "DevDataSourceConfig",    {"dev"},   1, 0 },

    /* @Profile("prod") — production RDS data source */
    { "prodDataSourceConfig",   "ProdDataSourceConfig",   {"prod"},  1, 0 },

    /* @Profile("!prod") — mock email service (dev + test) */
    { "mockEmailService",       "MockEmailServiceImpl",   {"prod"},  1, 1 },  /* negated */

    /* @Profile("prod") — real email service */
    { "realEmailService",       "SmtpEmailServiceImpl",   {"prod"},  1, 0 },

    /* @Profile({"dev","test"}) — stub payment gateway */
    { "stubPaymentService",     "StubPaymentGateway",     {"dev","test"}, 2, 0 },

    /* @Profile("prod") — real Stripe payment gateway */
    { "stripePaymentService",   "StripePaymentGateway",   {"prod"},  1, 0 },

    /* No profile annotation = always active */
    { "userService",            "UserServiceImpl",         {},        0, 0 },
};
static int profile_bean_count = 7;

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

static void run_for_profile(const char *profile) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Profile: '%s'\n", profile);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    PropMap env = {0};
    printf("  [ConfigData] Loading base application.yml (10 properties)\n");
    load_base_config(&env);

    load_profile_config(&env, profile, PROFILE_CONFIGS, profile_config_count);
    print_environment(&env, profile);
    evaluate_profile_beans(profile, PROFILE_BEANS, profile_bean_count);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  profile_resolver.c — Spring Boot Profiles in C     ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    run_for_profile("dev");
    run_for_profile("test");
    run_for_profile("prod");

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Base application.yml always loaded first\n");
    printf("  application-{profile}.yml overrides base values\n");
    printf("  @Profile('prod') beans only created when 'prod' is active\n");
    printf("  @Profile('!prod') beans created when 'prod' is NOT active\n");
    printf("  SPRING_PROFILES_ACTIVE env var controls profile in Docker/K8s\n");
    printf("  Never hardcode prod credentials — use ${DB_PASSWORD} env refs\n");
    return 0;
}
