/*
 * bean_definition.c  —  Spring BeanDefinition metadata in C
 *
 * Build:  gcc -Wall -Wextra -o bean_definition bean_definition.c
 * Run:    ./bean_definition
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.beans.factory.config.BeanDefinition
 *
 *   Before Spring creates any bean, it builds a BeanDefinition —
 *   a metadata object that describes EVERYTHING about how to create
 *   and configure that bean.
 *
 *   Spring Java fields (simplified):
 *     String  beanClassName        → fully qualified class name
 *     String  scope                → "singleton" / "prototype"
 *     boolean lazyInit             → defer creation until first use
 *     String  initMethodName       → @PostConstruct method name
 *     String  destroyMethodName    → @PreDestroy method name
 *     String[] dependsOn           → explicit dependency ordering
 *     boolean primary              → @Primary: preferred candidate
 *     boolean autowireCandidate    → eligible for @Autowired injection
 *     MutablePropertyValues props  → property values to inject
 *     ConstructorArgumentValues    → constructor args
 *
 * WHY THIS MATTERS:
 *   BeanDefinition is the "blueprint" for a bean.
 *   Spring reads annotations (@Component, @Scope, @Lazy...) and
 *   CONVERTS them into BeanDefinition objects.
 *   The actual bean instance is created from this definition later.
 *
 *   This separation (definition vs instance) allows Spring to:
 *     - Defer creation (lazy initialization)
 *     - Override definitions (test vs production config)
 *     - Support multiple scopes
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  BEAN SCOPE
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    SCOPE_SINGLETON,
    SCOPE_PROTOTYPE,
    SCOPE_REQUEST,    /* web only: one per HTTP request */
    SCOPE_SESSION,    /* web only: one per HTTP session */
} BeanScope;

/* ═══════════════════════════════════════════════════════════
 *  AUTOWIRE MODE
 *  Controls how Spring decides what to inject.
 *  Spring Java: AutowireCapableBeanFactory.AUTOWIRE_*
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    AUTOWIRE_NO,            /* no auto-wiring (use @Autowired explicitly)  */
    AUTOWIRE_BY_NAME,       /* inject by matching field name               */
    AUTOWIRE_BY_TYPE,       /* inject by matching class type               */
    AUTOWIRE_CONSTRUCTOR,   /* inject via constructor                      */
} AutowireMode;

/* ═══════════════════════════════════════════════════════════
 *  PROPERTY VALUE
 *  Represents a single injected property.
 *  Java: org.springframework.beans.PropertyValue
 *
 *  Example:
 *    <bean id="db" class="DatabaseConfig">
 *        <property name="url" value="jdbc:mysql://..." />
 *    </bean>
 *  OR:
 *    @Value("${db.url}") String url;
 * ═══════════════════════════════════════════════════════════ */
#define MAX_PROPERTY_NAME   64
#define MAX_PROPERTY_VALUE  256
#define MAX_PROPERTIES      16

typedef struct {
    char name[MAX_PROPERTY_NAME];
    char value[MAX_PROPERTY_VALUE];
    int  is_ref;    /* 1 if value is a bean reference (inject another bean) */
} PropertyValue;

/* ═══════════════════════════════════════════════════════════
 *  DEPENDENCY DESCRIPTOR
 *  Represents one @Autowired dependency.
 *  Java: org.springframework.beans.factory.config.DependencyDescriptor
 * ═══════════════════════════════════════════════════════════ */
#define MAX_DEPENDENCIES    8
#define MAX_NAME_LEN        128

typedef struct {
    char bean_name[MAX_NAME_LEN];    /* name of the bean to inject   */
    char field_name[MAX_NAME_LEN];   /* field in the target class    */
    int  required;                   /* @Autowired(required=true/false) */
} DependencyDescriptor;

/* ═══════════════════════════════════════════════════════════
 *  BEAN DEFINITION  — the blueprint for a Spring bean
 *  Java: org.springframework.beans.factory.config.BeanDefinition
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    /* Identity */
    char bean_name[MAX_NAME_LEN];       /* "userService"                          */
    char class_name[MAX_NAME_LEN];      /* "com.example.service.UserService"      */

    /* Scope & Lazy */
    BeanScope    scope;                 /* singleton (default), prototype, etc.   */
    int          lazy_init;            /* 0=eager (default), 1=lazy              */

    /* Priority & Qualification */
    int          primary;              /* @Primary: preferred when multiple match */
    int          autowire_candidate;   /* eligible for @Autowired (default: 1)   */

    /* Lifecycle methods */
    char         init_method[MAX_NAME_LEN];      /* @PostConstruct method name */
    char         destroy_method[MAX_NAME_LEN];   /* @PreDestroy method name    */

    /* Dependencies (wired via @Autowired) */
    DependencyDescriptor dependencies[MAX_DEPENDENCIES];
    int                  dependency_count;

    /* Property values (@Value, XML properties) */
    PropertyValue        properties[MAX_PROPERTIES];
    int                  property_count;

    /* Ordering: explicit bean creation order */
    char         depends_on[MAX_DEPENDENCIES][MAX_NAME_LEN];
    int          depends_on_count;

    /* Autowire mode */
    AutowireMode autowire_mode;

    /* Description (like Javadoc for the bean) */
    char         description[256];
} BeanDefinition;

/* ═══════════════════════════════════════════════════════════
 *  BEAN DEFINITION REGISTRY
 *  Spring Java: BeanDefinitionRegistry (implemented by DefaultListableBeanFactory)
 * ═══════════════════════════════════════════════════════════ */
#define MAX_BEAN_DEFS 64

typedef struct {
    BeanDefinition defs[MAX_BEAN_DEFS];
    int count;
} BeanDefinitionRegistry;

/* ─────────────────────────────────────────────────────────────
 *  Helper: create a default BeanDefinition
 *  (applies Spring's default values)
 * ──────────────────────────────────────────────────────────── */
static BeanDefinition bd_create(const char *name, const char *class_name) {
    BeanDefinition bd;
    memset(&bd, 0, sizeof(bd));

    strncpy(bd.bean_name,  name,       MAX_NAME_LEN - 1);
    strncpy(bd.class_name, class_name, MAX_NAME_LEN - 1);

    /* Spring defaults */
    bd.scope             = SCOPE_SINGLETON;     /* default scope */
    bd.lazy_init         = 0;                   /* eager by default */
    bd.primary           = 0;                   /* not @Primary */
    bd.autowire_candidate = 1;                  /* eligible for injection */
    bd.autowire_mode     = AUTOWIRE_NO;         /* use @Autowired explicitly */
    bd.dependency_count  = 0;
    bd.property_count    = 0;
    bd.depends_on_count  = 0;

    return bd;
}

/* Helper: add a @Autowired dependency to a BeanDefinition */
static void bd_add_dependency(BeanDefinition *bd,
                               const char *bean_name,
                               const char *field_name,
                               int required) {
    if (bd->dependency_count >= MAX_DEPENDENCIES) return;
    DependencyDescriptor *dep = &bd->dependencies[bd->dependency_count++];
    strncpy(dep->bean_name,  bean_name,  MAX_NAME_LEN - 1);
    strncpy(dep->field_name, field_name, MAX_NAME_LEN - 1);
    dep->required = required;
}

/* Helper: add a @Value property to a BeanDefinition */
static void bd_add_property(BeanDefinition *bd,
                             const char *name,
                             const char *value,
                             int is_ref) {
    if (bd->property_count >= MAX_PROPERTIES) return;
    PropertyValue *pv = &bd->properties[bd->property_count++];
    strncpy(pv->name,  name,  MAX_PROPERTY_NAME - 1);
    strncpy(pv->value, value, MAX_PROPERTY_VALUE - 1);
    pv->is_ref = is_ref;
}

/* Helper: register into registry */
static void registry_register(BeanDefinitionRegistry *reg, BeanDefinition *bd) {
    if (reg->count >= MAX_BEAN_DEFS) return;
    reg->defs[reg->count++] = *bd;
    printf("[registry] Registered BeanDefinition: '%s' (class=%s)\n",
           bd->bean_name, bd->class_name);
}

/* Helper: find a BeanDefinition by name */
static BeanDefinition* registry_find(BeanDefinitionRegistry *reg, const char *name) {
    for (int i = 0; i < reg->count; i++) {
        if (strcmp(reg->defs[i].bean_name, name) == 0) {
            return &reg->defs[i];
        }
    }
    return NULL;
}

/* Helper: print a BeanDefinition in detail */
static void bd_print(const BeanDefinition *bd) {
    const char *scope_str[] = {"singleton", "prototype", "request", "session"};
    const char *aw_str[]    = {"no", "byName", "byType", "constructor"};

    printf("\n  ┌─ BeanDefinition: '%s' ─────────────────────────────\n", bd->bean_name);
    printf("  │  class         : %s\n", bd->class_name);
    printf("  │  scope         : %s\n", scope_str[bd->scope]);
    printf("  │  lazyInit      : %s\n", bd->lazy_init ? "true" : "false");
    printf("  │  primary       : %s\n", bd->primary   ? "true" : "false");
    printf("  │  autowireMode  : %s\n", aw_str[bd->autowire_mode]);
    printf("  │  initMethod    : %s\n", bd->init_method[0]    ? bd->init_method    : "(none)");
    printf("  │  destroyMethod : %s\n", bd->destroy_method[0] ? bd->destroy_method : "(none)");

    if (bd->dependency_count > 0) {
        printf("  │  @Autowired dependencies (%d):\n", bd->dependency_count);
        for (int i = 0; i < bd->dependency_count; i++) {
            printf("  │    field '%s' ← bean '%s' (required=%s)\n",
                   bd->dependencies[i].field_name,
                   bd->dependencies[i].bean_name,
                   bd->dependencies[i].required ? "true" : "false");
        }
    }

    if (bd->property_count > 0) {
        printf("  │  @Value / properties (%d):\n", bd->property_count);
        for (int i = 0; i < bd->property_count; i++) {
            printf("  │    %s = '%s'%s\n",
                   bd->properties[i].name,
                   bd->properties[i].value,
                   bd->properties[i].is_ref ? " (bean reference)" : "");
        }
    }

    if (bd->depends_on_count > 0) {
        printf("  │  dependsOn: ");
        for (int i = 0; i < bd->depends_on_count; i++) {
            printf("'%s' ", bd->depends_on[i]);
        }
        printf("\n");
    }
    printf("  └──────────────────────────────────────────────────────\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_bean_definitions(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Building BeanDefinitions (what @Component scan does)\n");
    printf("Java annotations → BeanDefinition metadata objects\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    BeanDefinitionRegistry reg;
    memset(&reg, 0, sizeof(reg));

    /* ── DatabaseConfig bean definition ──
     * Java:
     *   @Configuration
     *   public class DatabaseConfig {
     *       @Value("${db.url}")
     *       private String url;
     *       @Value("${db.pool.size:10}")
     *       private int poolSize;
     *   }
     */
    BeanDefinition db_def = bd_create("databaseConfig",
                                      "com.example.config.DatabaseConfig");
    db_def.scope      = SCOPE_SINGLETON;
    db_def.lazy_init  = 0;
    strncpy(db_def.init_method, "initialize", MAX_NAME_LEN - 1);
    bd_add_property(&db_def, "url",      "jdbc:mysql://localhost:3306/mydb", 0);
    bd_add_property(&db_def, "poolSize", "10",                               0);
    bd_add_property(&db_def, "timeout",  "30000",                            0);
    strncpy(db_def.description, "@Configuration class for database", 255);
    registry_register(&reg, &db_def);

    /* ── UserRepository bean definition ──
     * Java:
     *   @Repository
     *   public class UserRepository {
     *       @Autowired
     *       private DatabaseConfig databaseConfig;
     *   }
     */
    BeanDefinition repo_def = bd_create("userRepository",
                                        "com.example.repository.UserRepository");
    repo_def.scope = SCOPE_SINGLETON;
    strncpy(repo_def.init_method,    "init",    MAX_NAME_LEN - 1);
    strncpy(repo_def.destroy_method, "cleanup", MAX_NAME_LEN - 1);
    bd_add_dependency(&repo_def, "databaseConfig", "databaseConfig", 1);
    strncpy(repo_def.description, "@Repository for User data access", 255);
    registry_register(&reg, &repo_def);

    /* ── UserService bean definition ──
     * Java:
     *   @Service
     *   public class UserService {
     *       @Autowired
     *       private UserRepository userRepository;
     *       @Autowired
     *       private EmailService emailService;
     *   }
     */
    BeanDefinition svc_def = bd_create("userService",
                                       "com.example.service.UserService");
    svc_def.scope   = SCOPE_SINGLETON;
    svc_def.primary = 1;    /* @Primary — preferred when multiple UserService exist */
    strncpy(svc_def.init_method,    "onStart",  MAX_NAME_LEN - 1);
    strncpy(svc_def.destroy_method, "onStop",   MAX_NAME_LEN - 1);
    bd_add_dependency(&svc_def, "userRepository", "userRepository", 1);
    bd_add_dependency(&svc_def, "emailService",   "emailService",   0);  /* required=false */
    strncpy(svc_def.description, "@Service for user business logic", 255);
    registry_register(&reg, &svc_def);

    /* ── OrderService bean definition (prototype + lazy) ──
     * Java:
     *   @Service
     *   @Scope("prototype")
     *   @Lazy
     *   public class OrderService { ... }
     */
    BeanDefinition order_def = bd_create("orderService",
                                         "com.example.service.OrderService");
    order_def.scope          = SCOPE_PROTOTYPE;
    order_def.lazy_init      = 1;        /* @Lazy: don't create at startup */
    order_def.autowire_mode  = AUTOWIRE_BY_TYPE;
    strncpy(order_def.depends_on[0], "userService", MAX_NAME_LEN - 1);
    order_def.depends_on_count = 1;      /* ensure userService created first */
    strncpy(order_def.description, "@Service (prototype) for order processing", 255);
    registry_register(&reg, &order_def);

    /* Print all bean definitions */
    printf("\n── Bean Definitions in Registry (%d total) ──────────────\n", reg.count);
    for (int i = 0; i < reg.count; i++) {
        bd_print(&reg.defs[i]);
    }
}

void demo_lazy_vs_eager(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Lazy vs Eager Initialization\n");
    printf("Java:\n");
    printf("  @Component           class EagerBean { } // created at startup\n");
    printf("  @Lazy @Component     class LazyBean  { } // created on first use\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    BeanDefinitionRegistry reg;
    memset(&reg, 0, sizeof(reg));

    BeanDefinition eager = bd_create("eagerService", "com.example.EagerService");
    eager.lazy_init = 0;   /* default: create at refresh() */
    registry_register(&reg, &eager);

    BeanDefinition lazy = bd_create("lazyService", "com.example.LazyService");
    lazy.lazy_init = 1;    /* @Lazy: defer until first getBean() call */
    registry_register(&reg, &lazy);

    printf("\nSimulating context.refresh():\n");
    for (int i = 0; i < reg.count; i++) {
        BeanDefinition *bd = &reg.defs[i];
        if (!bd->lazy_init && bd->scope == SCOPE_SINGLETON) {
            printf("  [refresh] CREATING '%s' (eager singleton)\n", bd->bean_name);
        } else {
            printf("  [refresh] SKIPPING '%s' (lazy=%d, scope=%s)\n",
                   bd->bean_name,
                   bd->lazy_init,
                   bd->scope == SCOPE_SINGLETON ? "singleton" : "prototype");
        }
    }

    printf("\nSimulating getBean(\"lazyService\") — first use:\n");
    BeanDefinition *lazy_bd = registry_find(&reg, "lazyService");
    if (lazy_bd) {
        printf("  [getBean] '%s' not yet created → creating now (lazy init)\n",
               lazy_bd->bean_name);
        printf("  [getBean] '%s' instance created and cached as singleton\n",
               lazy_bd->bean_name);
    }
}

void demo_depends_on_ordering(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: Bean Creation Order with dependsOn\n");
    printf("Java:\n");
    printf("  @DependsOn(\"cacheManager\") @Service class UserService { }\n");
    printf("  → Spring ensures 'cacheManager' is created BEFORE UserService\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    BeanDefinitionRegistry reg;
    memset(&reg, 0, sizeof(reg));

    BeanDefinition cache = bd_create("cacheManager", "com.example.CacheManager");
    registry_register(&reg, &cache);

    BeanDefinition svc = bd_create("userService", "com.example.UserService");
    /* @DependsOn("cacheManager") */
    strncpy(svc.depends_on[0], "cacheManager", MAX_NAME_LEN - 1);
    svc.depends_on_count = 1;
    registry_register(&reg, &svc);

    BeanDefinition repo = bd_create("userRepository", "com.example.UserRepository");
    /* @DependsOn("cacheManager", "userService") */
    strncpy(repo.depends_on[0], "cacheManager", MAX_NAME_LEN - 1);
    strncpy(repo.depends_on[1], "userService",  MAX_NAME_LEN - 1);
    repo.depends_on_count = 2;
    registry_register(&reg, &repo);

    printf("Bean creation order (Spring resolves dependsOn graph):\n");
    printf("  1. 'cacheManager' — no dependencies\n");
    printf("  2. 'userService'  — depends on: cacheManager (satisfied)\n");
    printf("  3. 'userRepository' — depends on: cacheManager, userService (satisfied)\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  bean_definition.c — Spring BeanDefinition in C     ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_bean_definitions();
    demo_lazy_vs_eager();
    demo_depends_on_ordering();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  BeanDefinition = blueprint, NOT the actual bean instance\n");
    printf("  Spring reads @Component, @Scope, @Lazy → BeanDefinition\n");
    printf("  BeanDefinition separates DESCRIPTION from CREATION\n");
    printf("  lazy_init=true  → skip during refresh(), create on demand\n");
    printf("  dependsOn       → explicit creation ordering guarantee\n");
    printf("  primary=true    → preferred when multiple beans of same type\n");
    return 0;
}
