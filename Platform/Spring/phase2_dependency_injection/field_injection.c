/*
 * field_injection.c  —  Spring @Autowired Field Injection in C
 *
 * Build:  gcc -Wall -Wextra -o field_injection field_injection.c
 * Run:    ./field_injection
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   Spring @Autowired field injection — the most common DI style.
 *
 *   Java:
 *     @Service
 *     public class UserService {
 *         @Autowired
 *         private UserRepository userRepository;   ← Spring injects this
 *
 *         @Autowired
 *         private EmailService emailService;       ← Spring injects this
 *     }
 *
 *   Spring implementation internally:
 *     1. After creating UserService instance
 *     2. Scan all fields with @Autowired annotation
 *     3. For each field: find matching bean by TYPE in context
 *     4. Use Reflection: field.setAccessible(true); field.set(bean, dep);
 *
 *   In C, we simulate Reflection using:
 *     - struct field offsets (offsetof())
 *     - void* pointer arithmetic
 *     - function pointer tables (like vtables)
 *
 * KEY CONCEPTS:
 *   - offsetof(struct, field) = byte offset of field within struct
 *   - (char*)bean + offset    = pointer to the field inside the bean
 *   - Dereferencing as void** = write the dependency pointer
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>    /* for offsetof() */

/* ═══════════════════════════════════════════════════════════
 *  BEAN TYPES — our "class hierarchy"
 * ═══════════════════════════════════════════════════════════ */

/* Type IDs — simulate Java class type tokens */
typedef enum {
    TYPE_DATABASE_CONFIG,
    TYPE_USER_REPOSITORY,
    TYPE_EMAIL_SERVICE,
    TYPE_USER_SERVICE,
    TYPE_ORDER_SERVICE,
    TYPE_COUNT
} BeanType;

static const char* type_name(BeanType t) {
    static const char* names[] = {
        "DatabaseConfig", "UserRepository", "EmailService",
        "UserService", "OrderService"
    };
    return t < TYPE_COUNT ? names[t] : "Unknown";
}

/* ── DatabaseConfig ── */
typedef struct {
    char url[128];
    int  pool_size;
} DatabaseConfig;

/* ── UserRepository ── */
typedef struct {
    DatabaseConfig *db_config;   /* @Autowired DatabaseConfig dbConfig; */
    int             query_count;
} UserRepository;

/* ── EmailService ── */
typedef struct {
    char smtp_host[64];
    int  port;
    int  emails_sent;
} EmailService;

/* ── UserService ── */
typedef struct {
    UserRepository *user_repo;    /* @Autowired UserRepository userRepository; */
    EmailService   *email_svc;    /* @Autowired EmailService emailService;     */
    int             user_count;
} UserService;

/* ── OrderService ── */
typedef struct {
    UserService *user_svc;       /* @Autowired UserService userService; */
    int          order_count;
} OrderService;


/* ═══════════════════════════════════════════════════════════
 *  FIELD DESCRIPTOR — metadata about one @Autowired field
 *
 *  Java equivalent: Spring's InjectionMetadata.InjectedElement
 *  which holds:
 *    - Field field        (java.lang.reflect.Field)
 *    - boolean required   (@Autowired(required=true/false))
 *    - Class<?> fieldType (what type to look up in context)
 *
 *  In C, we use:
 *    - offset = offsetof(struct, field)  ← replaces Field.get/set
 *    - dep_type = the BeanType to inject
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    size_t    offset;      /* byte offset of the field in the struct (offsetof) */
    BeanType  dep_type;    /* which type of bean to inject                      */
    int       required;    /* 1 = throw if not found, 0 = leave null            */
    char      field_name[64]; /* for debug/logging                              */
} FieldDescriptor;

/* ═══════════════════════════════════════════════════════════
 *  INJECTION METADATA — all @Autowired fields for one class
 *  Java: InjectionMetadata (Spring caches this per class)
 * ═══════════════════════════════════════════════════════════ */
#define MAX_FIELDS 8

typedef struct {
    BeanType        bean_type;         /* the class this metadata belongs to */
    FieldDescriptor fields[MAX_FIELDS];
    int             field_count;
} InjectionMetadata;

/* ═══════════════════════════════════════════════════════════
 *  BEAN CONTAINER — simplified ApplicationContext
 * ═══════════════════════════════════════════════════════════ */
#define MAX_BEANS 16

typedef struct {
    BeanType  type;
    void     *instance;
    char      name[64];
} BeanEntry;

typedef struct {
    BeanEntry entries[MAX_BEANS];
    int       count;
} BeanContainer;

static void container_put(BeanContainer *c, BeanType type, const char *name, void *inst) {
    if (c->count >= MAX_BEANS) return;
    c->entries[c->count].type     = type;
    c->entries[c->count].instance = inst;
    strncpy(c->entries[c->count].name, name, 63);
    c->count++;
}

static void* container_get_by_type(BeanContainer *c, BeanType type) {
    for (int i = 0; i < c->count; i++) {
        if (c->entries[i].type == type) return c->entries[i].instance;
    }
    return NULL;
}


/* ═══════════════════════════════════════════════════════════
 *  AUTOWIRED INJECTOR
 *
 *  Java equivalent:
 *    AutowiredAnnotationBeanPostProcessor.postProcessProperties()
 *    → InjectionMetadata.inject()
 *       → field.set(bean, resolvedDependency)
 *
 *  This is the core of Spring's DI engine:
 *    1. Get injection metadata for the bean type
 *    2. For each @Autowired field:
 *       a. Find the dependency bean by type in container
 *       b. Write it into the field using offsetof + pointer cast
 * ═══════════════════════════════════════════════════════════ */
static void inject_dependencies(void *bean,
                                 const InjectionMetadata *meta,
                                 BeanContainer *container)
{
    printf("  [AutowiredProcessor] Processing '%s' (%d fields)\n",
           type_name(meta->bean_type), meta->field_count);

    for (int i = 0; i < meta->field_count; i++) {
        const FieldDescriptor *fd = &meta->fields[i];

        /* Step 1: Look up the dependency bean by type */
        void *dep = container_get_by_type(container, fd->dep_type);

        if (dep == NULL) {
            if (fd->required) {
                /* Spring throws: NoSuchBeanDefinitionException */
                fprintf(stderr,
                        "  [ERROR] Required dependency '%s' (type=%s) not found!\n"
                        "         → Spring throws: NoSuchBeanDefinitionException\n",
                        fd->field_name, type_name(fd->dep_type));
            } else {
                /* @Autowired(required=false) → leave field as NULL */
                printf("  [inject] field '%s' = NULL (required=false, bean not found)\n",
                       fd->field_name);
            }
            continue;
        }

        /* Step 2: Write the pointer into the field
         *
         * This simulates Java's:
         *   field.setAccessible(true);
         *   field.set(bean, dep);
         *
         * In C:
         *   - (char*)bean = treat bean as byte array
         *   - + fd->offset = move to the field's byte position
         *   - (void**) cast = treat those bytes as a pointer slot
         *   - *(...) = dep  = write the dependency pointer
         *
         * This is EXACTLY what Java Reflection does internally via JNI.
         */
        void **field_ptr = (void**)((char*)bean + fd->offset);
        *field_ptr = dep;

        printf("  [inject] field '%s' ← %s instance @ %p\n",
               fd->field_name, type_name(fd->dep_type), dep);
    }
}


/* ═══════════════════════════════════════════════════════════
 *  BUILD INJECTION METADATA TABLES
 *  (what Spring builds by scanning @Autowired annotations at startup)
 *
 *  Java: AutowiredAnnotationBeanPostProcessor reads annotations
 *        and caches InjectionMetadata per class.
 * ═══════════════════════════════════════════════════════════ */

static InjectionMetadata build_user_repository_metadata(void) {
    InjectionMetadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.bean_type = TYPE_USER_REPOSITORY;

    /* @Autowired DatabaseConfig dbConfig; */
    meta.fields[0].offset   = offsetof(UserRepository, db_config);
    meta.fields[0].dep_type = TYPE_DATABASE_CONFIG;
    meta.fields[0].required = 1;
    strncpy(meta.fields[0].field_name, "dbConfig", 63);

    meta.field_count = 1;
    return meta;
}

static InjectionMetadata build_user_service_metadata(void) {
    InjectionMetadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.bean_type = TYPE_USER_SERVICE;

    /* @Autowired UserRepository userRepository; */
    meta.fields[0].offset   = offsetof(UserService, user_repo);
    meta.fields[0].dep_type = TYPE_USER_REPOSITORY;
    meta.fields[0].required = 1;
    strncpy(meta.fields[0].field_name, "userRepository", 63);

    /* @Autowired(required=false) EmailService emailService; */
    meta.fields[1].offset   = offsetof(UserService, email_svc);
    meta.fields[1].dep_type = TYPE_EMAIL_SERVICE;
    meta.fields[1].required = 0;    /* optional — won't throw if missing */
    strncpy(meta.fields[1].field_name, "emailService", 63);

    meta.field_count = 2;
    return meta;
}

static InjectionMetadata build_order_service_metadata(void) {
    InjectionMetadata meta;
    memset(&meta, 0, sizeof(meta));
    meta.bean_type = TYPE_ORDER_SERVICE;

    /* @Autowired UserService userService; */
    meta.fields[0].offset   = offsetof(OrderService, user_svc);
    meta.fields[0].dep_type = TYPE_USER_SERVICE;
    meta.fields[0].required = 1;
    strncpy(meta.fields[0].field_name, "userService", 63);

    meta.field_count = 1;
    return meta;
}


/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_field_injection(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Field Injection (@Autowired)\n");
    printf("Java:\n");
    printf("  @Service class UserService {\n");
    printf("    @Autowired UserRepository userRepository;\n");
    printf("    @Autowired(required=false) EmailService emailService;\n");
    printf("  }\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Step 1: Create instances (raw allocation, no DI yet) */
    printf("── Step 1: Create raw instances (constructors) ─────────\n");

    DatabaseConfig *db = calloc(1, sizeof(DatabaseConfig));
    snprintf(db->url, sizeof(db->url), "jdbc:mysql://localhost:3306/app");
    db->pool_size = 10;
    printf("  DatabaseConfig created @ %p\n", (void*)db);

    UserRepository *repo = calloc(1, sizeof(UserRepository));
    repo->query_count = 0;
    printf("  UserRepository created @ %p  (db_config=NULL — not injected yet)\n", (void*)repo);

    EmailService *email = calloc(1, sizeof(EmailService));
    snprintf(email->smtp_host, sizeof(email->smtp_host), "smtp.example.com");
    email->port = 587;
    printf("  EmailService created @ %p\n", (void*)email);

    UserService *svc = calloc(1, sizeof(UserService));
    svc->user_count = 0;
    printf("  UserService created @ %p  (user_repo=NULL, email_svc=NULL — not injected yet)\n",
           (void*)svc);

    /* Step 2: Register all beans in container */
    printf("\n── Step 2: Register beans in ApplicationContext ─────────\n");
    BeanContainer container;
    memset(&container, 0, sizeof(container));
    container_put(&container, TYPE_DATABASE_CONFIG, "databaseConfig", db);
    container_put(&container, TYPE_USER_REPOSITORY,  "userRepository",  repo);
    container_put(&container, TYPE_EMAIL_SERVICE,    "emailService",    email);
    container_put(&container, TYPE_USER_SERVICE,     "userService",     svc);
    printf("  %d beans registered\n", container.count);

    /* Step 3: Build injection metadata (Spring scans @Autowired at startup) */
    printf("\n── Step 3: Build Injection Metadata (scan @Autowired) ───\n");
    InjectionMetadata repo_meta  = build_user_repository_metadata();
    InjectionMetadata svc_meta   = build_user_service_metadata();
    printf("  Metadata built for: UserRepository (%d fields), UserService (%d fields)\n",
           repo_meta.field_count, svc_meta.field_count);

    /* Step 4: Run field injection (Spring's AutowiredAnnotationBeanPostProcessor) */
    printf("\n── Step 4: Run Field Injection (PostProcessor) ──────────\n");
    inject_dependencies(repo, &repo_meta,  &container);
    inject_dependencies(svc,  &svc_meta,   &container);

    /* Step 5: Verify — are the pointers correctly wired? */
    printf("\n── Step 5: Verify Injection Results ─────────────────────\n");

    printf("  UserRepository.db_config  = %p (expected %p) → %s\n",
           (void*)repo->db_config, (void*)db,
           repo->db_config == db ? "✓ CORRECT" : "✗ WRONG");

    printf("  UserService.user_repo     = %p (expected %p) → %s\n",
           (void*)svc->user_repo, (void*)repo,
           svc->user_repo == repo ? "✓ CORRECT" : "✗ WRONG");

    printf("  UserService.email_svc     = %p (expected %p) → %s\n",
           (void*)svc->email_svc, (void*)email,
           svc->email_svc == email ? "✓ CORRECT" : "✗ WRONG");

    /* Step 6: Use the injected beans — they work! */
    printf("\n── Step 6: Using the wired service ──────────────────────\n");
    printf("  svc->user_repo->db_config->url = '%s'\n",
           svc->user_repo->db_config->url);
    printf("  svc->email_svc->smtp_host      = '%s'\n",
           svc->email_svc->smtp_host);

    free(db); free(repo); free(email); free(svc);
}

void demo_missing_required_dependency(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Missing Required Dependency\n");
    printf("Java: @Autowired (required=true) bean not found\n");
    printf("      → Spring throws: NoSuchBeanDefinitionException\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    BeanContainer container;
    memset(&container, 0, sizeof(container));

    /* Create UserService but DON'T register UserRepository */
    UserService *svc = calloc(1, sizeof(UserService));
    container_put(&container, TYPE_USER_SERVICE, "userService", svc);

    InjectionMetadata svc_meta = build_user_service_metadata();

    printf("Attempting injection with missing UserRepository:\n");
    inject_dependencies(svc, &svc_meta, &container);

    printf("\n  svc->user_repo = %p (NULL = injection failed, as expected)\n",
           (void*)svc->user_repo);

    free(svc);
}

void demo_injection_chain(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: Deep Injection Chain\n");
    printf("Java: OrderService → UserService → UserRepository → DatabaseConfig\n");
    printf("      Each layer gets its dependencies injected automatically\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Create all instances */
    DatabaseConfig *db    = calloc(1, sizeof(DatabaseConfig));
    snprintf(db->url, sizeof(db->url), "jdbc:postgresql://prod:5432/orders");
    db->pool_size = 20;

    UserRepository *repo  = calloc(1, sizeof(UserRepository));
    UserService    *svc   = calloc(1, sizeof(UserService));
    OrderService   *order = calloc(1, sizeof(OrderService));

    /* Register in container */
    BeanContainer container;
    memset(&container, 0, sizeof(container));
    container_put(&container, TYPE_DATABASE_CONFIG, "databaseConfig", db);
    container_put(&container, TYPE_USER_REPOSITORY,  "userRepository",  repo);
    container_put(&container, TYPE_USER_SERVICE,     "userService",     svc);
    container_put(&container, TYPE_ORDER_SERVICE,    "orderService",    order);

    /* Build metadata for each layer */
    InjectionMetadata repo_meta  = build_user_repository_metadata();
    InjectionMetadata svc_meta   = build_user_service_metadata();
    InjectionMetadata order_meta = build_order_service_metadata();

    /* Inject in dependency order (Spring resolves this automatically) */
    printf("Injecting in dependency order:\n");
    inject_dependencies(repo,  &repo_meta,  &container);  /* repo needs db */
    inject_dependencies(svc,   &svc_meta,   &container);  /* svc needs repo */
    inject_dependencies(order, &order_meta, &container);  /* order needs svc */

    /* Verify full chain */
    printf("\nVerifying full injection chain:\n");
    printf("  order.user_svc             → %s\n", order->user_svc ? "✓ wired" : "✗ NULL");
    printf("  order.user_svc.user_repo   → %s\n", order->user_svc->user_repo ? "✓ wired" : "✗ NULL");
    printf("  order.user_svc.user_repo.db_config → %s\n",
           order->user_svc->user_repo->db_config ? "✓ wired" : "✗ NULL");
    printf("  db url: %s\n", order->user_svc->user_repo->db_config->url);

    free(db); free(repo); free(svc); free(order);
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  field_injection.c — Spring @Autowired in C         ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_field_injection();
    demo_missing_required_dependency();
    demo_injection_chain();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  @Autowired = find bean by type + write pointer into field\n");
    printf("  Spring uses Reflection (field.set()) — we use offsetof() + pointer cast\n");
    printf("  required=true  → exception if not found\n");
    printf("  required=false → leave null, no exception\n");
    printf("  Injection order matters: inject dependencies before their dependents\n");
    return 0;
}
