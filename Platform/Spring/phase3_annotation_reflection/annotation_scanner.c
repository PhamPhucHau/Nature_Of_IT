/*
 * annotation_scanner.c  —  Spring @ComponentScan / Classpath Scanning in C
 *
 * Build:  gcc -Wall -Wextra -o annotation_scanner annotation_scanner.c
 * Run:    ./annotation_scanner
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.context.annotation.ClassPathScanningCandidateComponentProvider
 *
 *   When Spring starts with @ComponentScan("com.example"), it:
 *     1. Walks the classpath under "com/example/"
 *     2. Reads every .class file
 *     3. Checks if the class has @Component (or meta-annotated with it)
 *     4. Creates a BeanDefinition for each match
 *     5. Registers in the BeanDefinitionRegistry
 *
 *   In real Spring, step 2-3 uses ASM to read bytecode without loading the class.
 *   In this C simulation, we use a pre-built static registry table to represent
 *   the "discovered" classes — because C doesn't have a classpath.
 *
 * KEY CONCEPTS:
 *   - @Component is a meta-annotation — @Service, @Repository, @Controller
 *     all have @Component on them (annotation inheritance via meta-annotation)
 *   - Spring filters: include @Component, exclude @Configuration (handled separately)
 *   - The scanner produces BeanDefinitions, NOT bean instances
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  ANNOTATION TYPES
 *  Mirrors Spring's annotation hierarchy.
 *  Note: @Service, @Repository, @Controller are ALL @Component.
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    ANNOTATION_NONE         = 0,
    ANNOTATION_COMPONENT    = 1 << 0,    /* @Component                      */
    ANNOTATION_SERVICE      = 1 << 1,    /* @Service      (meta: @Component)*/
    ANNOTATION_REPOSITORY   = 1 << 2,    /* @Repository   (meta: @Component)*/
    ANNOTATION_CONTROLLER   = 1 << 3,    /* @Controller   (meta: @Component)*/
    ANNOTATION_REST_CTRL    = 1 << 4,    /* @RestController (meta: @Controller) */
    ANNOTATION_CONFIGURATION= 1 << 5,   /* @Configuration                  */
    ANNOTATION_BEAN         = 1 << 6,   /* @Bean (method-level)            */
    ANNOTATION_PRIMARY      = 1 << 7,   /* @Primary                        */
    ANNOTATION_LAZY         = 1 << 8,   /* @Lazy                           */
} AnnotationType;

/* Helper: check if a class has @Component or any meta-@Component annotation */
static int is_component_candidate(int annotations) {
    return (annotations & ANNOTATION_COMPONENT)   ||
           (annotations & ANNOTATION_SERVICE)      ||
           (annotations & ANNOTATION_REPOSITORY)   ||
           (annotations & ANNOTATION_CONTROLLER)   ||
           (annotations & ANNOTATION_REST_CTRL);
}

static const char* annotation_string(int annotations) {
    static char buf[256];
    buf[0] = '\0';
    if (annotations & ANNOTATION_COMPONENT)     strcat(buf, "@Component ");
    if (annotations & ANNOTATION_SERVICE)       strcat(buf, "@Service ");
    if (annotations & ANNOTATION_REPOSITORY)    strcat(buf, "@Repository ");
    if (annotations & ANNOTATION_CONTROLLER)    strcat(buf, "@Controller ");
    if (annotations & ANNOTATION_REST_CTRL)     strcat(buf, "@RestController ");
    if (annotations & ANNOTATION_CONFIGURATION) strcat(buf, "@Configuration ");
    if (annotations & ANNOTATION_PRIMARY)       strcat(buf, "@Primary ");
    if (annotations & ANNOTATION_LAZY)          strcat(buf, "@Lazy ");
    if (buf[0] == '\0') strcat(buf, "(none)");
    return buf;
}

/* ═══════════════════════════════════════════════════════════
 *  SCOPE (for @Scope annotation)
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    SCOPE_SINGLETON,
    SCOPE_PROTOTYPE,
} BeanScope;

/* ═══════════════════════════════════════════════════════════
 *  CLASS DESCRIPTOR
 *  Simulates what Spring reads from .class file bytecode.
 *  In Java, this info comes from:
 *    - Class.getName()
 *    - Class.getAnnotations()
 *    - Class.getDeclaredFields()
 *    - Class.getDeclaredMethods()
 * ═══════════════════════════════════════════════════════════ */
#define MAX_NAME 128

typedef struct {
    char  class_name[MAX_NAME];    /* e.g. "com.example.service.UserService" */
    char  simple_name[MAX_NAME];   /* e.g. "UserService"                     */
    char  package[MAX_NAME];       /* e.g. "com.example.service"             */
    int   annotations;             /* bitmask of AnnotationType              */
    BeanScope scope;
} ClassDescriptor;

/* ═══════════════════════════════════════════════════════════
 *  SIMULATED CLASSPATH
 *  In real Spring, this comes from scanning .class files on disk.
 *  Here, we define the "classpath" as a static array — every class
 *  that would exist in a real Spring application.
 *
 *  Java package "com.example" → we simulate its classes here.
 * ═══════════════════════════════════════════════════════════ */
static ClassDescriptor classpath[] = {
    /* ── Config layer ── */
    {
        "com.example.config.AppConfig",       "AppConfig",       "com.example.config",
        ANNOTATION_CONFIGURATION,             SCOPE_SINGLETON
    },
    {
        "com.example.config.DatabaseConfig",  "DatabaseConfig",  "com.example.config",
        ANNOTATION_CONFIGURATION,             SCOPE_SINGLETON
    },
    {
        "com.example.config.SecurityConfig",  "SecurityConfig",  "com.example.config",
        ANNOTATION_CONFIGURATION,             SCOPE_SINGLETON
    },

    /* ── Repository layer (@Repository = @Component for data access) ── */
    {
        "com.example.repository.UserRepository",  "UserRepository",  "com.example.repository",
        ANNOTATION_REPOSITORY,                    SCOPE_SINGLETON
    },
    {
        "com.example.repository.OrderRepository", "OrderRepository", "com.example.repository",
        ANNOTATION_REPOSITORY,                    SCOPE_SINGLETON
    },
    {
        "com.example.repository.ProductRepository","ProductRepository","com.example.repository",
        ANNOTATION_REPOSITORY | ANNOTATION_PRIMARY,  SCOPE_SINGLETON  /* @Primary */
    },

    /* ── Service layer (@Service = @Component for business logic) ── */
    {
        "com.example.service.UserService",    "UserService",    "com.example.service",
        ANNOTATION_SERVICE,                   SCOPE_SINGLETON
    },
    {
        "com.example.service.OrderService",   "OrderService",   "com.example.service",
        ANNOTATION_SERVICE | ANNOTATION_LAZY, SCOPE_PROTOTYPE   /* @Lazy + prototype */
    },
    {
        "com.example.service.EmailService",   "EmailService",   "com.example.service",
        ANNOTATION_SERVICE,                   SCOPE_SINGLETON
    },
    {
        "com.example.service.PaymentService", "PaymentService", "com.example.service",
        ANNOTATION_SERVICE,                   SCOPE_SINGLETON
    },

    /* ── Web layer (@Controller / @RestController = @Component for HTTP) ── */
    {
        "com.example.controller.UserController",  "UserController",  "com.example.controller",
        ANNOTATION_REST_CTRL,                     SCOPE_SINGLETON
    },
    {
        "com.example.controller.OrderController", "OrderController", "com.example.controller",
        ANNOTATION_REST_CTRL,                     SCOPE_SINGLETON
    },
    {
        "com.example.controller.AuthController",  "AuthController",  "com.example.controller",
        ANNOTATION_CONTROLLER,                    SCOPE_SINGLETON
    },

    /* ── Utility classes (NOT annotated — will NOT be scanned) ── */
    {
        "com.example.util.StringUtils",  "StringUtils",  "com.example.util",
        ANNOTATION_NONE,                 SCOPE_SINGLETON   /* no annotation → skipped */
    },
    {
        "com.example.util.DateUtils",    "DateUtils",    "com.example.util",
        ANNOTATION_NONE,                 SCOPE_SINGLETON
    },
    {
        "com.example.model.User",        "User",         "com.example.model",
        ANNOTATION_NONE,                 SCOPE_SINGLETON   /* model/DTO → not a bean */
    },
};

static int classpath_size = (int)(sizeof(classpath) / sizeof(classpath[0]));

/* ═══════════════════════════════════════════════════════════
 *  BEAN DEFINITION (simplified — same as Phase 1)
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    char      bean_name[MAX_NAME];   /* derived from class simple name */
    char      class_name[MAX_NAME];
    int       annotations;
    BeanScope scope;
    int       primary;
    int       lazy;
} BeanDefinition;

/* ═══════════════════════════════════════════════════════════
 *  BEAN DEFINITION REGISTRY
 * ═══════════════════════════════════════════════════════════ */
#define MAX_BEANS 64

typedef struct {
    BeanDefinition defs[MAX_BEANS];
    int count;
} BeanDefinitionRegistry;

/* ─────────────────────────────────────────────────────────────
 *  Helper: derive bean name from class simple name
 *  Spring's default: first letter lowercase
 *  "UserService" → "userService"
 *  Can be overridden with @Component("myName")
 * ──────────────────────────────────────────────────────────── */
static void derive_bean_name(const char *simple_name, char *out, int out_size) {
    strncpy(out, simple_name, out_size - 1);
    out[0] = (char)(out[0] | 32);  /* lowercase first letter (ASCII trick) */
}

/* ─────────────────────────────────────────────────────────────
 *  COMPONENT SCANNER
 *  Java: ClassPathBeanDefinitionScanner.doScan(basePackage)
 *
 *  Scans all classes and registers those with @Component
 *  (or meta-annotated like @Service, @Repository, @Controller)
 * ──────────────────────────────────────────────────────────── */
static void scan_package(BeanDefinitionRegistry *registry,
                          const char *base_package)
{
    printf("[ComponentScan] Scanning package: '%s'\n", base_package);
    int found = 0;

    for (int i = 0; i < classpath_size; i++) {
        ClassDescriptor *cls = &classpath[i];

        /* Filter: only scan classes in the base package (or sub-packages) */
        if (strncmp(cls->package, base_package, strlen(base_package)) != 0) {
            continue;  /* not in target package */
        }

        printf("  [scan] Checking: %s ... ", cls->class_name);

        /* Filter: only classes with @Component or meta-@Component */
        if (!is_component_candidate(cls->annotations)) {
            printf("SKIP (no @Component annotation)\n");
            continue;
        }

        /* Excluded: @Configuration is handled separately by Spring */
        if (cls->annotations & ANNOTATION_CONFIGURATION) {
            printf("SKIP (@Configuration — handled by ConfigurationClassParser)\n");
            continue;
        }

        /* Found a candidate — create BeanDefinition */
        if (registry->count >= MAX_BEANS) {
            fprintf(stderr, "[ERROR] Registry full!\n");
            break;
        }

        BeanDefinition *bd = &registry->defs[registry->count++];
        derive_bean_name(cls->simple_name, bd->bean_name, MAX_NAME);
        strncpy(bd->class_name,   cls->class_name, MAX_NAME - 1);
        bd->annotations = cls->annotations;
        bd->scope       = cls->scope;
        bd->primary     = (cls->annotations & ANNOTATION_PRIMARY) ? 1 : 0;
        bd->lazy        = (cls->annotations & ANNOTATION_LAZY) ? 1 : 0;

        printf("FOUND → '%s'\n", bd->bean_name);
        found++;
    }

    printf("[ComponentScan] Found %d candidate component(s) in '%s'\n\n",
           found, base_package);
}

/* ─────────────────────────────────────────────────────────────
 *  Print registry summary
 * ──────────────────────────────────────────────────────────── */
static void print_registry(BeanDefinitionRegistry *reg) {
    printf("━━━ Registered BeanDefinitions (%d total) ━━━━━━━━━━━━━━━━\n",
           reg->count);
    printf("  %-24s %-12s %-8s %-6s %s\n",
           "Bean Name", "Scope", "Lazy", "Primary", "Annotations");
    printf("  %-24s %-12s %-8s %-6s %s\n",
           "────────────────────────",
           "────────────",
           "────────",
           "──────",
           "───────────────────────────────");

    for (int i = 0; i < reg->count; i++) {
        BeanDefinition *bd = &reg->defs[i];
        printf("  %-24s %-12s %-8s %-6s %s\n",
               bd->bean_name,
               bd->scope == SCOPE_SINGLETON ? "singleton" : "prototype",
               bd->lazy    ? "true" : "false",
               bd->primary ? "true" : "false",
               annotation_string(bd->annotations));
    }
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_component_scan(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: @ComponentScan of entire 'com.example' package\n");
    printf("Java: @SpringBootApplication → @ComponentScan(\"com.example\")\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    BeanDefinitionRegistry reg;
    memset(&reg, 0, sizeof(reg));

    scan_package(&reg, "com.example");
    print_registry(&reg);
}

void demo_selective_scan(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Selective scan — only 'com.example.service'\n");
    printf("Java: @ComponentScan(\"com.example.service\")\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    BeanDefinitionRegistry reg;
    memset(&reg, 0, sizeof(reg));

    scan_package(&reg, "com.example.service");
    print_registry(&reg);
}

void demo_multi_package_scan(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: Multi-package scan (service + repository only)\n");
    printf("Java: @ComponentScan({\"com.example.service\", \"com.example.repository\"})\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    BeanDefinitionRegistry reg;
    memset(&reg, 0, sizeof(reg));

    scan_package(&reg, "com.example.service");
    scan_package(&reg, "com.example.repository");
    print_registry(&reg);
}

void demo_annotation_hierarchy(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 4: Annotation Meta-Annotation Hierarchy\n");
    printf("@Service, @Repository, @Controller are all @Component\n");
    printf("Spring calls this 'meta-annotation' or 'composed annotation'\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    printf("Annotation inheritance chain:\n");
    printf("  @Component\n");
    printf("  └── @Service        (has @Component on it)\n");
    printf("  └── @Repository     (has @Component on it)\n");
    printf("  └── @Controller     (has @Component on it)\n");
    printf("      └── @RestController  (has @Controller on it)\n\n");

    printf("Spring filter check for each class:\n");
    int test_cases[][2] = {
        {ANNOTATION_COMPONENT,  1},
        {ANNOTATION_SERVICE,    1},
        {ANNOTATION_REPOSITORY, 1},
        {ANNOTATION_CONTROLLER, 1},
        {ANNOTATION_REST_CTRL,  1},
        {ANNOTATION_CONFIGURATION, 0},   /* handled separately */
        {ANNOTATION_NONE,       0},
    };
    const char *names[] = {
        "@Component", "@Service", "@Repository", "@Controller",
        "@RestController", "@Configuration", "(no annotation)"
    };
    int n = (int)(sizeof(test_cases)/sizeof(test_cases[0]));
    for (int i = 0; i < n; i++) {
        int is_cand = is_component_candidate(test_cases[i][0]);
        printf("  %-20s → candidate=%s %s\n",
               names[i],
               is_cand ? "YES" : "NO ",
               is_cand != test_cases[i][1] ? "← UNEXPECTED" : "");
    }
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  annotation_scanner.c — Spring @ComponentScan in C  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_component_scan();
    demo_selective_scan();
    demo_multi_package_scan();
    demo_annotation_hierarchy();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  @ComponentScan walks the classpath looking for @Component classes\n");
    printf("  @Service/@Repository/@Controller are @Component with extra meaning\n");
    printf("  @Configuration is NOT registered as a component — processed separately\n");
    printf("  Non-annotated classes (models, utils) are completely ignored\n");
    printf("  Scanner produces BeanDefinitions, NOT bean instances\n");
    return 0;
}
