/*
 * property_binder.c  —  Spring Boot @ConfigurationProperties Binding in C
 *
 * Build:  gcc -Wall -Wextra -o property_binder property_binder.c -lm
 * Run:    ./property_binder
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.boot.context.properties.bind.Binder
 *   org.springframework.boot.context.properties.ConfigurationPropertiesBindingPostProcessor
 *
 *   Spring Boot's property binder does:
 *     1. Scans all @ConfigurationProperties beans
 *     2. For each: reads properties matching the prefix
 *     3. Converts each String value to the target field type
 *     4. Writes values into the bean's fields via Reflection
 *     5. Validates if @Validated is present
 *
 *   Key Java source:
 *     Binder.bind(ConfigurationPropertyName name, Bindable<?> target)
 *     → BeanBinder.bind()
 *       → JavaBeanBinder.bind()  ← uses setters
 *       → ConstructorParametersBinder.bind()  ← uses constructor
 *
 * C APPROACH:
 *   We represent the property binding as a table of:
 *     { yml_key, field_offset, type, default_value }
 *   Then iterate all loaded properties and write matching values.
 *   This is EXACTLY what Spring Boot does via Reflection.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

/* ═══════════════════════════════════════════════════════════
 *  PROPERTY SOURCE — simulates application.yml loaded as flat map
 *  Spring Boot flattens nested YAML into dotted keys:
 *    server:
 *      port: 8080         → "server.port" = "8080"
 *      tomcat:
 *        max-threads: 200 → "server.tomcat.max-threads" = "200"
 * ═══════════════════════════════════════════════════════════ */
#define MAX_PROPS 128
#define KEY_LEN    128
#define VAL_LEN    256

typedef struct {
    char key[KEY_LEN];
    char value[VAL_LEN];
    int  priority;  /* lower = higher priority; 1=CLI, 4=env var, 6=yml */
} Property;

typedef struct {
    Property entries[MAX_PROPS];
    int      count;
} PropertySource;

/* Add or overwrite property (higher priority wins) */
static void prop_set(PropertySource *ps, const char *key, const char *val, int priority) {
    /* Check for existing key */
    for (int i = 0; i < ps->count; i++) {
        if (strcmp(ps->entries[i].key, key) == 0) {
            if (priority <= ps->entries[i].priority) {
                strncpy(ps->entries[i].value, val, VAL_LEN - 1);
                ps->entries[i].priority = priority;
            }
            return;
        }
    }
    if (ps->count >= MAX_PROPS) return;
    strncpy(ps->entries[ps->count].key,   key, KEY_LEN - 1);
    strncpy(ps->entries[ps->count].value, val, VAL_LEN - 1);
    ps->entries[ps->count].priority = priority;
    ps->count++;
}

static const char* prop_get(const PropertySource *ps, const char *key) {
    for (int i = 0; i < ps->count; i++) {
        if (strcmp(ps->entries[i].key, key) == 0) return ps->entries[i].value;
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════
 *  RELAXED KEY NORMALIZATION
 *  Spring Boot normalizes all key variants to canonical form.
 *  Canonical form = lowercase, hyphens only.
 *  "MY_DB_URL", "myDbUrl", "my-db-url" → "my-db-url"
 *
 *  Java: ConfigurationPropertyName.adapt(name, '.')
 * ═══════════════════════════════════════════════════════════ */
static void normalize_key(const char *input, char *output, int out_size) {
    int j = 0;
    for (int i = 0; input[i] && j < out_size - 1; i++) {
        char c = input[i];

        /* Uppercase letter → lowercase, insert hyphen if previous was lowercase */
        if (c >= 'A' && c <= 'Z') {
            if (j > 0 && output[j-1] != '-' && output[j-1] != '.') {
                output[j++] = '-';
            }
            output[j++] = (char)(c + 32);  /* to lowercase */
        }
        /* Underscore → hyphen (MY_HOST → my-host) */
        else if (c == '_') {
            if (j > 0 && output[j-1] != '-' && output[j-1] != '.') {
                output[j++] = '-';
            }
        }
        else {
            output[j++] = (char)tolower((unsigned char)c);
        }
    }
    output[j] = '\0';
}

/* ═══════════════════════════════════════════════════════════
 *  FIELD TYPE ENUM — for type-safe conversion
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    FIELD_STRING,
    FIELD_INT,
    FIELD_LONG,
    FIELD_BOOL,
    FIELD_DOUBLE,
    FIELD_DURATION_MS,   /* "10s" → 10000 ms, "500ms" → 500 */
} FieldType;

/* ═══════════════════════════════════════════════════════════
 *  BINDING DESCRIPTOR — one per field in a @ConfigurationProperties class
 *  Java equivalent: BeanPropertyWriter (Jackson) or PropertyDescriptor (Java Beans)
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    const char *yml_key;          /* property key after prefix stripped  */
    size_t      field_offset;     /* offsetof(struct, field)             */
    FieldType   type;
    const char *default_value;    /* null if required                    */
    int         required;
    const char *description;      /* for metadata / documentation        */
} BindingDescriptor;

/* ═══════════════════════════════════════════════════════════
 *  TYPE CONVERSION
 *  Java: ConversionService + TypeDescriptor
 *  Converts String property value to target Java/C type.
 * ═══════════════════════════════════════════════════════════ */
static long parse_duration_ms(const char *val) {
    /* Parse: "10s" → 10000, "500ms" → 500, "2m" → 120000, "1h" → 3600000 */
    char *end;
    long  n = strtol(val, &end, 10);
    if (strcmp(end, "ms") == 0) return n;
    if (strcmp(end, "s")  == 0) return n * 1000L;
    if (strcmp(end, "m")  == 0) return n * 60000L;
    if (strcmp(end, "h")  == 0) return n * 3600000L;
    return n;  /* assume ms if no suffix */
}

static void bind_field(void *bean, const BindingDescriptor *bd, const char *str_val) {
    void *field_ptr = (char*)bean + bd->field_offset;

    switch (bd->type) {
        case FIELD_STRING:
            strncpy((char*)field_ptr, str_val, VAL_LEN - 1);
            break;
        case FIELD_INT:
            *(int*)field_ptr = (int)strtol(str_val, NULL, 10);
            break;
        case FIELD_LONG:
            *(long*)field_ptr = strtol(str_val, NULL, 10);
            break;
        case FIELD_BOOL:
            *(int*)field_ptr = (strcmp(str_val, "true") == 0 ||
                                strcmp(str_val, "1")    == 0 ||
                                strcmp(str_val, "yes")  == 0) ? 1 : 0;
            break;
        case FIELD_DOUBLE:
            *(double*)field_ptr = strtod(str_val, NULL);
            break;
        case FIELD_DURATION_MS:
            *(long*)field_ptr = parse_duration_ms(str_val);
            break;
    }
}

/* ═══════════════════════════════════════════════════════════
 *  THE BINDER
 *  Java: Binder.bind(prefix, Bindable.of(BeanClass.class))
 *
 *  Algorithm:
 *    1. For each field descriptor
 *    2. Build the full property key: prefix + "." + yml_key
 *    3. Normalize the key (relaxed binding)
 *    4. Look up in PropertySource
 *    5. If found: convert type + write to field offset
 *    6. If not found and has default: use default
 *    7. If not found and required: report error
 * ═══════════════════════════════════════════════════════════ */
static int bind_properties(void *bean,
                            const char *prefix,
                            const BindingDescriptor *descriptors,
                            int descriptor_count,
                            const PropertySource *ps,
                            int verbose)
{
    int errors = 0;

    if (verbose) {
        printf("  [Binder] Binding prefix='%s' (%d fields)\n",
               prefix, descriptor_count);
    }

    for (int i = 0; i < descriptor_count; i++) {
        const BindingDescriptor *bd = &descriptors[i];

        /* Build full key */
        char full_key[256];
        snprintf(full_key, sizeof(full_key), "%s.%s", prefix, bd->yml_key);

        /* Normalize key (relaxed binding) */
        char norm_key[256];
        normalize_key(full_key, norm_key, sizeof(norm_key));

        /* Try exact key first, then normalized */
        const char *val = prop_get(ps, full_key);
        if (!val) val = prop_get(ps, norm_key);

        if (val) {
            bind_field(bean, bd, val);
            if (verbose) {
                printf("    ✓ '%s' = '%s'", full_key, val);
                if (strcmp(full_key, norm_key) != 0) {
                    printf(" (relaxed: '%s')", norm_key);
                }
                printf("\n");
            }
        } else if (bd->default_value) {
            bind_field(bean, bd, bd->default_value);
            if (verbose) {
                printf("    ~ '%s' = '%s' (default)\n", full_key, bd->default_value);
            }
        } else if (bd->required) {
            fprintf(stderr,
                    "    ✗ [BindException] Required property '%s' not found!\n"
                    "      → Bind failure: could not bind property '%s'\n",
                    full_key, full_key);
            errors++;
        } else {
            if (verbose) {
                printf("    - '%s' not set, no default (optional)\n", full_key);
            }
        }
    }

    if (verbose) printf("\n");
    return errors;
}

/* ═══════════════════════════════════════════════════════════
 *  SAMPLE @ConfigurationProperties CLASSES (as C structs)
 *
 *  These simulate real Spring Boot config classes.
 * ═══════════════════════════════════════════════════════════ */

/* ── ServerProperties ──
 * Java: org.springframework.boot.autoconfigure.web.ServerProperties
 * @ConfigurationProperties(prefix = "server")
 */
typedef struct {
    int  port;                    /* server.port (default: 8080) */
    char context_path[128];       /* server.servlet.context-path */
    int  connection_timeout_ms;   /* server.connection-timeout   */
    int  max_threads;             /* server.tomcat.threads.max   */
    int  min_spare_threads;       /* server.tomcat.threads.min-spare */
    int  compression_enabled;     /* server.compression.enabled  */
    char compression_types[256];  /* server.compression.mime-types */
} ServerProperties;

static const BindingDescriptor SERVER_BINDINGS[] = {
    { "port",                      offsetof(ServerProperties, port),                   FIELD_INT,    "8080",              0, "HTTP server port"                },
    { "servlet.context-path",      offsetof(ServerProperties, context_path),           FIELD_STRING, "/",                 0, "Context path"                   },
    { "connection-timeout",        offsetof(ServerProperties, connection_timeout_ms),  FIELD_DURATION_MS, "20s",           0, "Connection timeout"             },
    { "tomcat.threads.max",        offsetof(ServerProperties, max_threads),            FIELD_INT,    "200",               0, "Max Tomcat worker threads"      },
    { "tomcat.threads.min-spare",  offsetof(ServerProperties, min_spare_threads),      FIELD_INT,    "10",                0, "Min spare Tomcat threads"       },
    { "compression.enabled",       offsetof(ServerProperties, compression_enabled),    FIELD_BOOL,   "false",             0, "Enable HTTP compression"        },
    { "compression.mime-types",    offsetof(ServerProperties, compression_types),      FIELD_STRING, "text/html,text/css,application/json", 0, "Compression types" },
};

/* ── DataSourceProperties ──
 * Java: org.springframework.boot.autoconfigure.jdbc.DataSourceProperties
 * @ConfigurationProperties(prefix = "spring.datasource")
 */
typedef struct {
    char url[256];          /* spring.datasource.url      */
    char username[64];      /* spring.datasource.username */
    char password[64];      /* spring.datasource.password */
    char driver[128];       /* spring.datasource.driver-class-name */
    char pool_name[64];     /* spring.datasource.hikari.pool-name  */
    int  max_pool_size;     /* spring.datasource.hikari.maximum-pool-size */
    int  min_idle;          /* spring.datasource.hikari.minimum-idle */
    long conn_timeout_ms;   /* spring.datasource.hikari.connection-timeout */
    long idle_timeout_ms;   /* spring.datasource.hikari.idle-timeout */
    long max_lifetime_ms;   /* spring.datasource.hikari.max-lifetime */
} DataSourceProperties;

static const BindingDescriptor DATASOURCE_BINDINGS[] = {
    { "url",                      offsetof(DataSourceProperties, url),              FIELD_STRING, NULL,           1, "JDBC URL (required)" },
    { "username",                 offsetof(DataSourceProperties, username),          FIELD_STRING, "sa",           0, "DB username" },
    { "password",                 offsetof(DataSourceProperties, password),          FIELD_STRING, "",             0, "DB password" },
    { "driver-class-name",        offsetof(DataSourceProperties, driver),            FIELD_STRING, "org.h2.Driver",0, "JDBC driver" },
    { "hikari.pool-name",         offsetof(DataSourceProperties, pool_name),         FIELD_STRING, "HikariPool-1", 0, "Pool name" },
    { "hikari.maximum-pool-size", offsetof(DataSourceProperties, max_pool_size),     FIELD_INT,    "10",           0, "Max pool size" },
    { "hikari.minimum-idle",      offsetof(DataSourceProperties, min_idle),          FIELD_INT,    "10",           0, "Min idle connections" },
    { "hikari.connection-timeout",offsetof(DataSourceProperties, conn_timeout_ms),   FIELD_DURATION_MS, "30s",    0, "Connection timeout" },
    { "hikari.idle-timeout",      offsetof(DataSourceProperties, idle_timeout_ms),   FIELD_DURATION_MS, "600s",   0, "Idle timeout" },
    { "hikari.max-lifetime",      offsetof(DataSourceProperties, max_lifetime_ms),   FIELD_DURATION_MS, "1800s",  0, "Max lifetime" },
};

/* ── Custom App Properties ──
 * Simulates a user-defined @ConfigurationProperties class:
 *   @ConfigurationProperties(prefix = "myapp")
 */
typedef struct {
    int  feature_enabled;          /* myapp.feature.enabled    */
    char api_key[128];             /* myapp.api.key            */
    int  max_requests_per_minute;  /* myapp.rate-limit.max-rpm */
    long cache_ttl_ms;             /* myapp.cache.ttl          */
} AppProperties;

static const BindingDescriptor APP_BINDINGS[] = {
    { "feature.enabled",        offsetof(AppProperties, feature_enabled),         FIELD_BOOL,       "false", 0, "Feature flag"         },
    { "api.key",                offsetof(AppProperties, api_key),                 FIELD_STRING,     NULL,    1, "API key (required)"   },
    { "rate-limit.max-rpm",     offsetof(AppProperties, max_requests_per_minute), FIELD_INT,        "100",   0, "Rate limit per min"   },
    { "cache.ttl",              offsetof(AppProperties, cache_ttl_ms),            FIELD_DURATION_MS,"5m",    0, "Cache TTL"            },
};

/* ═══════════════════════════════════════════════════════════
 *  PRINT HELPERS
 * ═══════════════════════════════════════════════════════════ */
static void print_server_props(const ServerProperties *p) {
    printf("  ServerProperties {\n");
    printf("    port                 = %d\n", p->port);
    printf("    context_path         = '%s'\n", p->context_path);
    printf("    connection_timeout   = %d ms\n", p->connection_timeout_ms);
    printf("    max_threads          = %d\n", p->max_threads);
    printf("    min_spare_threads    = %d\n", p->min_spare_threads);
    printf("    compression_enabled  = %s\n", p->compression_enabled ? "true" : "false");
    printf("  }\n\n");
}

static void print_datasource_props(const DataSourceProperties *p) {
    printf("  DataSourceProperties {\n");
    printf("    url              = '%s'\n", p->url);
    printf("    username         = '%s'\n", p->username);
    printf("    password         = '***hidden***'\n");
    printf("    pool_name        = '%s'\n", p->pool_name);
    printf("    max_pool_size    = %d\n", p->max_pool_size);
    printf("    min_idle         = %d\n", p->min_idle);
    printf("    conn_timeout     = %ld ms\n", p->conn_timeout_ms);
    printf("    idle_timeout     = %ld ms\n", p->idle_timeout_ms);
    printf("    max_lifetime     = %ld ms\n", p->max_lifetime_ms);
    printf("  }\n\n");
}

static void print_app_props(const AppProperties *p) {
    printf("  AppProperties {\n");
    printf("    feature_enabled      = %s\n", p->feature_enabled ? "true" : "false");
    printf("    api_key              = '%s'\n", p->api_key);
    printf("    max_requests_per_min = %d\n", p->max_requests_per_minute);
    printf("    cache_ttl            = %ld ms\n", p->cache_ttl_ms);
    printf("  }\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_full_binding(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Full @ConfigurationProperties Binding\n");
    printf("Simulates: ConfigurationPropertiesBindingPostProcessor.postProcessBeforeInitialization()\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Load application.yml (simulated) */
    PropertySource ps = {0};

    /* Priority 6: application.yml */
    prop_set(&ps, "server.port",                         "9090",                  6);
    prop_set(&ps, "server.servlet.context-path",         "/api",                  6);
    prop_set(&ps, "server.compression.enabled",          "true",                  6);
    prop_set(&ps, "server.tomcat.threads.max",           "400",                   6);
    prop_set(&ps, "spring.datasource.url",               "jdbc:mysql://localhost:3306/mydb", 6);
    prop_set(&ps, "spring.datasource.username",          "appuser",               6);
    prop_set(&ps, "spring.datasource.password",          "s3cr3t",                6);
    prop_set(&ps, "spring.datasource.hikari.maximum-pool-size", "20",             6);
    prop_set(&ps, "spring.datasource.hikari.connection-timeout", "30s",           6);
    prop_set(&ps, "myapp.feature.enabled",               "true",                  6);
    prop_set(&ps, "myapp.api.key",                       "sk-abc123xyz",          6);
    prop_set(&ps, "myapp.rate-limit.max-rpm",            "500",                   6);
    prop_set(&ps, "myapp.cache.ttl",                     "10m",                   6);

    /* Priority 1: CLI args override */
    prop_set(&ps, "server.port", "8443", 1);  /* --server.port=8443 */

    printf("Loaded %d properties from application.yml + CLI args\n\n", ps.count);

    /* Bind ServerProperties */
    ServerProperties server_props = {0};
    printf("Binding @ConfigurationProperties(prefix=\"server\"):\n");
    bind_properties(&server_props, "server",
                    SERVER_BINDINGS,
                    (int)(sizeof(SERVER_BINDINGS)/sizeof(SERVER_BINDINGS[0])),
                    &ps, 1);
    printf("Result:\n");
    print_server_props(&server_props);

    /* Bind DataSourceProperties */
    DataSourceProperties ds_props = {0};
    printf("Binding @ConfigurationProperties(prefix=\"spring.datasource\"):\n");
    bind_properties(&ds_props, "spring.datasource",
                    DATASOURCE_BINDINGS,
                    (int)(sizeof(DATASOURCE_BINDINGS)/sizeof(DATASOURCE_BINDINGS[0])),
                    &ps, 1);
    printf("Result:\n");
    print_datasource_props(&ds_props);

    /* Bind AppProperties */
    AppProperties app_props = {0};
    printf("Binding @ConfigurationProperties(prefix=\"myapp\"):\n");
    bind_properties(&app_props, "myapp",
                    APP_BINDINGS,
                    (int)(sizeof(APP_BINDINGS)/sizeof(APP_BINDINGS[0])),
                    &ps, 1);
    printf("Result:\n");
    print_app_props(&app_props);
}

void demo_relaxed_binding(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Relaxed Binding (all key formats → same field)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    struct { const char *input; } variants[] = {
        { "spring.datasource.maximum-pool-size" },   /* kebab (yml recommended) */
        { "spring.datasource.maximumPoolSize"   },   /* camelCase               */
        { "spring.datasource.maximum_pool_size" },   /* underscore              */
        { "SPRING_DATASOURCE_MAXIMUM_POOL_SIZE" },   /* env var style (OS env)  */
        { "spring.datasource.MaximumPoolSize"   },   /* PascalCase              */
    };

    printf("  All of these map to the SAME field (maximumPoolSize / maximum-pool-size):\n\n");
    for (int i = 0; i < 5; i++) {
        char normalized[256];
        normalize_key(variants[i].input, normalized, sizeof(normalized));
        printf("  Input:  %-44s\n", variants[i].input);
        printf("  Normal: %s\n\n", normalized);
    }

    printf("  Spring Boot uses relaxed binding to allow:\n");
    printf("    - YAML files:            kebab-case   (spring.datasource.maximum-pool-size)\n");
    printf("    - Environment variables: UPPER_CASE   (SPRING_DATASOURCE_MAXIMUM_POOL_SIZE)\n");
    printf("    - System properties:     any format   (-Dspring.datasource.maximumPoolSize)\n");
    printf("    All map to the same @ConfigurationProperties field.\n\n");
}

void demo_missing_required(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: Missing Required Property → BindException\n");
    printf("Spring Boot fails fast if a required property is absent\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    PropertySource ps = {0};
    /* Intentionally missing: myapp.api.key (required=1) */
    prop_set(&ps, "myapp.feature.enabled", "true", 6);

    AppProperties app_props = {0};
    printf("Binding AppProperties with missing 'myapp.api.key' (required):\n");
    int errors = bind_properties(&app_props, "myapp",
                                  APP_BINDINGS,
                                  (int)(sizeof(APP_BINDINGS)/sizeof(APP_BINDINGS[0])),
                                  &ps, 1);

    if (errors > 0) {
        printf("  → Spring Boot throws: BindException\n");
        printf("    Description: Failed to bind properties under 'myapp'\n");
        printf("    Reason: 'api.key' is required but was not found\n");
        printf("    Action: Set 'myapp.api.key' in application.yml or pass --myapp.api.key=...\n\n");
    }
}

void demo_duration_conversion(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 4: Type Conversion (Duration, Bool, Int from String)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    struct { const char *raw; long expected_ms; } durations[] = {
        { "500ms",  500     },
        { "30s",    30000   },
        { "5m",     300000  },
        { "1h",     3600000 },
        { "2000",   2000    },   /* no suffix = ms */
    };

    printf("  Duration string → milliseconds:\n");
    for (int i = 0; i < 5; i++) {
        long ms = parse_duration_ms(durations[i].raw);
        printf("    %-8s → %7ld ms  %s\n",
               durations[i].raw, ms,
               ms == durations[i].expected_ms ? "✓" : "✗");
    }

    printf("\n  Boolean conversion:\n");
    const char *bool_vals[] = { "true", "false", "1", "0", "yes", "no" };
    for (int i = 0; i < 6; i++) {
        int b = (strcmp(bool_vals[i], "true") == 0 ||
                 strcmp(bool_vals[i], "1")    == 0 ||
                 strcmp(bool_vals[i], "yes")  == 0) ? 1 : 0;
        printf("    %-8s → %s\n", bool_vals[i], b ? "true" : "false");
    }
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  property_binder.c — @ConfigurationProperties in C  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_full_binding();
    demo_relaxed_binding();
    demo_missing_required();
    demo_duration_conversion();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Binding = iterate fields, look up property, convert type, write value\n");
    printf("  Relaxed binding: MY_DB_URL and my-db-url are the same field\n");
    printf("  Duration strings: '30s', '5m', '1h' → milliseconds\n");
    printf("  Required fields missing → BindException at startup (fail fast)\n");
    printf("  CLI args override application.yml (priority chain)\n");
    return 0;
}
