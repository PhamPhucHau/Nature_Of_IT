/*
 * bean_factory.c  —  Spring IoC Container (BeanFactory) in C
 *
 * Build:  gcc -Wall -Wextra -o bean_factory bean_factory.c
 * Run:    ./bean_factory
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.beans.factory.support.DefaultListableBeanFactory
 *
 *   The IoC container is the HEART of Spring Framework.
 *   Every bean (object) is created, stored, and retrieved here.
 *
 *   Spring Java equivalent:
 *
 *     ApplicationContext ctx = new AnnotationConfigApplicationContext(AppConfig.class);
 *     UserService svc = ctx.getBean(UserService.class);
 *
 *   Internally, Spring stores all beans in a Map:
 *
 *     Map<String, Object> singletonObjects = new ConcurrentHashMap<>();
 *
 *   This C file implements exactly that map-based registry.
 *
 * KEY INSIGHT:
 *   Spring IoC is NOT magic.
 *   It is a HashMap that maps class names → object instances.
 *   The magic is WHO controls the lifecycle (Spring, not you).
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  CONFIGURATION
 * ═══════════════════════════════════════════════════════════ */

#define MAX_BEANS       64    /* max number of beans in container */
#define BEAN_NAME_LEN   128   /* max length of a bean name        */

/* ═══════════════════════════════════════════════════════════
 *  BEAN SCOPE
 *  Mirrors: org.springframework.beans.factory.config.ConfigurableBeanFactory
 *    SCOPE_SINGLETON = "singleton"
 *    SCOPE_PROTOTYPE = "prototype"
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    SCOPE_SINGLETON,    /* one instance shared by all — default */
    SCOPE_PROTOTYPE,    /* new instance created on every getBean() */
} BeanScope;

/* ═══════════════════════════════════════════════════════════
 *  BEAN LIFECYCLE STATE
 *  Spring tracks each singleton's lifecycle:
 *    REGISTERED  → BeanDefinition added
 *    CREATED     → Constructor called, instance in memory
 *    INITIALIZED → @PostConstruct ran, ready to use
 *    DESTROYED   → @PreDestroy ran, object dead
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    BEAN_REGISTERED,    /* definition stored, not yet created */
    BEAN_CREATED,       /* instance allocated                  */
    BEAN_INITIALIZED,   /* @PostConstruct called, ready        */
    BEAN_DESTROYED,     /* @PreDestroy called                  */
} BeanLifecycle;

/* ═══════════════════════════════════════════════════════════
 *  FUNCTION POINTER TYPES FOR LIFECYCLE HOOKS
 *  Spring Java equivalent:
 *    @PostConstruct: void init()
 *    @PreDestroy:    void destroy()
 *  In C, we store these as function pointers.
 * ═══════════════════════════════════════════════════════════ */
typedef void (*InitCallback)(void *bean);
typedef void (*DestroyCallback)(void *bean);
typedef void* (*BeanFactory_fn)(void);    /* factory method that creates the instance */

/* ═══════════════════════════════════════════════════════════
 *  BEAN ENTRY
 *  In Spring, this is like BeanDefinition + singleton object combined.
 *  Spring's actual split:
 *    BeanDefinition  = metadata (class, scope, init method name...)
 *    singletonObjects Map = actual instances
 *  We combine them here for clarity.
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    char             name[BEAN_NAME_LEN];  /* e.g. "userService"       */
    BeanScope        scope;                /* singleton or prototype    */
    BeanLifecycle    lifecycle;            /* current lifecycle state   */
    void            *instance;            /* pointer to the object     */
    BeanFactory_fn   factory;             /* function that creates it  */
    InitCallback     init_fn;             /* @PostConstruct equivalent */
    DestroyCallback  destroy_fn;          /* @PreDestroy equivalent    */
} BeanEntry;

/* ═══════════════════════════════════════════════════════════
 *  APPLICATION CONTEXT (the IoC Container itself)
 *
 *  Spring Java equivalent:
 *    DefaultListableBeanFactory
 *    → private final Map<String, Object> singletonObjects
 *    → private final Map<String, BeanDefinition> beanDefinitionMap
 *
 *  We combine both maps into one array here.
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    BeanEntry beans[MAX_BEANS];   /* the "singleton objects" map    */
    int       bean_count;         /* how many beans are registered  */
    int       initialized;        /* 1 after refresh() is called    */
} ApplicationContext;

/* ─────────────────────────────────────────────────────────────
 *  Forward declarations
 * ──────────────────────────────────────────────────────────── */
static ApplicationContext* context_create(void);
static void   context_register_bean(ApplicationContext *ctx,
                                    const char *name,
                                    BeanScope scope,
                                    BeanFactory_fn factory,
                                    InitCallback init_fn,
                                    DestroyCallback destroy_fn);
static void   context_refresh(ApplicationContext *ctx);
static void*  context_get_bean(ApplicationContext *ctx, const char *name);
static void   context_close(ApplicationContext *ctx);
static void   context_print_summary(ApplicationContext *ctx);


/* ═══════════════════════════════════════════════════════════
 *  SAMPLE BEANS — simulating real Spring service classes
 *
 *  Java equivalent:
 *    @Component class DatabaseConfig { String url; int poolSize; }
 *    @Service   class UserRepository  { DatabaseConfig db; }
 *    @Service   class UserService     { UserRepository repo; }
 * ═══════════════════════════════════════════════════════════ */

/* ── DatabaseConfig bean ── */
typedef struct {
    char url[256];
    int  pool_size;
    int  timeout_ms;
} DatabaseConfig;

void* create_database_config(void) {
    DatabaseConfig *cfg = calloc(1, sizeof(DatabaseConfig));
    snprintf(cfg->url, sizeof(cfg->url), "jdbc:mysql://localhost:3306/mydb");
    cfg->pool_size  = 10;
    cfg->timeout_ms = 30000;
    printf("  [factory] DatabaseConfig created at %p\n", (void*)cfg);
    return cfg;
}

void init_database_config(void *bean) {
    DatabaseConfig *cfg = (DatabaseConfig*)bean;
    /* @PostConstruct: validate connection settings */
    printf("  [@PostConstruct] DatabaseConfig.init() — validating url: %s\n", cfg->url);
}

void destroy_database_config(void *bean) {
    DatabaseConfig *cfg = (DatabaseConfig*)bean;
    /* @PreDestroy: close connection pool */
    printf("  [@PreDestroy] DatabaseConfig.destroy() — closing pool (size=%d)\n", cfg->pool_size);
    free(cfg);
}

/* ── UserRepository bean ── */
typedef struct {
    char data_source[128];   /* wired from DatabaseConfig.url */
    int  query_count;        /* tracks how many queries ran   */
} UserRepository;

void* create_user_repository(void) {
    UserRepository *repo = calloc(1, sizeof(UserRepository));
    snprintf(repo->data_source, sizeof(repo->data_source), "mysql://localhost:3306/mydb");
    repo->query_count = 0;
    printf("  [factory] UserRepository created at %p\n", (void*)repo);
    return repo;
}

void init_user_repository(void *bean) {
    UserRepository *repo = (UserRepository*)bean;
    printf("  [@PostConstruct] UserRepository.init() — connected to: %s\n", repo->data_source);
}

void destroy_user_repository(void *bean) {
    UserRepository *repo = (UserRepository*)bean;
    printf("  [@PreDestroy] UserRepository.destroy() — total queries run: %d\n", repo->query_count);
    free(repo);
}

/* ── UserService bean ── */
typedef struct {
    char  service_name[64];
    int   request_count;
    int   error_count;
} UserService;

void* create_user_service(void) {
    UserService *svc = calloc(1, sizeof(UserService));
    snprintf(svc->service_name, sizeof(svc->service_name), "UserService");
    svc->request_count = 0;
    svc->error_count   = 0;
    printf("  [factory] UserService created at %p\n", (void*)svc);
    return svc;
}

void init_user_service(void *bean) {
    UserService *svc = (UserService*)bean;
    printf("  [@PostConstruct] UserService.init() — '%s' ready\n", svc->service_name);
}

void destroy_user_service(void *bean) {
    UserService *svc = (UserService*)bean;
    printf("  [@PreDestroy] UserService.destroy() — served %d requests, %d errors\n",
           svc->request_count, svc->error_count);
    free(svc);
}

/* ── OrderService bean (prototype scope) ── */
typedef struct {
    int    order_id;
    double total_amount;
    char   status[32];
} OrderService;

static int order_counter = 0;  /* simulate Spring's prototype counter */

void* create_order_service(void) {
    OrderService *order = calloc(1, sizeof(OrderService));
    order->order_id    = ++order_counter;
    order->total_amount = 0.0;
    snprintf(order->status, sizeof(order->status), "NEW");
    printf("  [factory] OrderService#%d created at %p (PROTOTYPE — new every time)\n",
           order->order_id, (void*)order);
    return order;
}

void init_order_service(void *bean) {
    OrderService *order = (OrderService*)bean;
    printf("  [@PostConstruct] OrderService#%d.init()\n", order->order_id);
}

void destroy_order_service(void *bean) {
    OrderService *order = (OrderService*)bean;
    printf("  [@PreDestroy] OrderService#%d.destroy() — status: %s\n",
           order->order_id, order->status);
    free(bean);
}


/* ═══════════════════════════════════════════════════════════
 *  CONTEXT IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════ */

/* context_create()
 * ─────────────────────────────────────────────────────────────
 * Java:  new AnnotationConfigApplicationContext()
 *        → creates an empty container, not yet started
 *
 * This is the constructor. No beans are created yet.
 * Beans are created during refresh() (startup).
 */
static ApplicationContext* context_create(void) {
    ApplicationContext *ctx = calloc(1, sizeof(ApplicationContext));
    ctx->bean_count  = 0;
    ctx->initialized = 0;
    printf("[ApplicationContext] Created (empty container)\n");
    return ctx;
}

/* context_register_bean()
 * ─────────────────────────────────────────────────────────────
 * Java:  ctx.register(UserService.class)  or
 *        @Component scanning that finds classes automatically
 *
 * This stores the DEFINITION (metadata) of a bean.
 * The actual instance is NOT created yet — only during refresh().
 *
 * Spring stores this in: Map<String, BeanDefinition> beanDefinitionMap
 */
static void context_register_bean(ApplicationContext *ctx,
                                  const char *name,
                                  BeanScope scope,
                                  BeanFactory_fn factory,
                                  InitCallback init_fn,
                                  DestroyCallback destroy_fn)
{
    if (ctx->bean_count >= MAX_BEANS) {
        fprintf(stderr, "[ERROR] BeanFactory is full (max=%d)\n", MAX_BEANS);
        return;
    }

    BeanEntry *entry = &ctx->beans[ctx->bean_count++];
    strncpy(entry->name, name, BEAN_NAME_LEN - 1);
    entry->scope      = scope;
    entry->lifecycle  = BEAN_REGISTERED;
    entry->instance   = NULL;     /* not created yet */
    entry->factory    = factory;
    entry->init_fn    = init_fn;
    entry->destroy_fn = destroy_fn;

    printf("[register] Bean definition '%s' registered (scope=%s)\n",
           name,
           scope == SCOPE_SINGLETON ? "singleton" : "prototype");
}

/* context_refresh()
 * ─────────────────────────────────────────────────────────────
 * Java:  AbstractApplicationContext.refresh()
 *        This is THE most important method in Spring.
 *        It is called once at startup and:
 *          1. Creates all singleton beans
 *          2. Injects dependencies
 *          3. Calls @PostConstruct methods
 *
 *        Prototype beans are NOT created here — only on demand.
 *
 *        Spring source:
 *          AbstractApplicationContext.java → refresh() {
 *            prepareRefresh();
 *            ConfigurableListableBeanFactory beanFactory = obtainFreshBeanFactory();
 *            finishBeanFactoryInitialization(beanFactory);  ← creates singletons
 *          }
 */
static void context_refresh(ApplicationContext *ctx) {
    printf("\n[ApplicationContext] refresh() — starting container...\n");

    for (int i = 0; i < ctx->bean_count; i++) {
        BeanEntry *entry = &ctx->beans[i];

        if (entry->scope == SCOPE_SINGLETON) {
            /* SINGLETON: create instance NOW, at startup */
            printf("\n  [refresh] Creating singleton bean: '%s'\n", entry->name);

            entry->instance  = entry->factory();            /* call factory */
            entry->lifecycle = BEAN_CREATED;

            if (entry->init_fn) {
                entry->init_fn(entry->instance);             /* @PostConstruct */
            }
            entry->lifecycle = BEAN_INITIALIZED;

        } else {
            /* PROTOTYPE: do NOT create now — will be created on getBean() */
            printf("  [refresh] Prototype bean '%s' deferred (will be created on demand)\n",
                   entry->name);
        }
    }

    ctx->initialized = 1;
    printf("\n[ApplicationContext] refresh() complete — %d bean(s) ready\n", ctx->bean_count);
}

/* context_get_bean()
 * ─────────────────────────────────────────────────────────────
 * Java:  applicationContext.getBean("userService")
 *        applicationContext.getBean(UserService.class)
 *
 * This is the retrieval point.
 * SINGLETON: returns the already-created shared instance
 * PROTOTYPE: creates a BRAND NEW instance every call
 *
 * Spring source:
 *   AbstractBeanFactory.java → getBean(name) → doGetBean()
 *     if (mbd.isSingleton()) return getSingleton(beanName, ...)
 *     if (mbd.isPrototype()) return createBean(beanName, ...)
 */
static void* context_get_bean(ApplicationContext *ctx, const char *name) {
    if (!ctx->initialized) {
        fprintf(stderr, "[ERROR] Context not started. Call context_refresh() first.\n");
        return NULL;
    }

    /* search the registry */
    for (int i = 0; i < ctx->bean_count; i++) {
        BeanEntry *entry = &ctx->beans[i];

        if (strcmp(entry->name, name) == 0) {

            if (entry->scope == SCOPE_SINGLETON) {
                /* SINGLETON: return the same shared instance every time */
                printf("[getBean] '%s' → returning singleton @ %p\n",
                       name, entry->instance);
                return entry->instance;

            } else {
                /* PROTOTYPE: create a NEW instance on every getBean() call */
                /* Spring DOES NOT track prototype instances after handing them out */
                void *new_instance = entry->factory();
                if (entry->init_fn) {
                    entry->init_fn(new_instance);
                }
                printf("[getBean] '%s' → new prototype instance @ %p\n",
                       name, new_instance);
                return new_instance;
            }
        }
    }

    fprintf(stderr, "[ERROR] No bean named '%s' found in context\n", name);
    return NULL;
}

/* context_close()
 * ─────────────────────────────────────────────────────────────
 * Java:  applicationContext.close()
 *        ConfigurableApplicationContext.close()
 *
 * Called on application shutdown (CTRL+C, graceful shutdown).
 * Runs @PreDestroy on all singleton beans in REVERSE order.
 *
 * Spring source:
 *   AbstractApplicationContext.java → close() → doClose() → destroyBeans()
 *     → DisposableBeanAdapter.destroy()  ← calls @PreDestroy
 *
 * NOTE: Prototype beans are NOT destroyed here.
 * Spring does not track prototypes — caller is responsible.
 */
static void context_close(ApplicationContext *ctx) {
    printf("\n[ApplicationContext] close() — shutting down container...\n");

    /* Destroy in REVERSE ORDER (last created = first destroyed)
     * This matters because beans may depend on each other.
     * If A depends on B, B should outlive A.
     * Creating order: B first, A second
     * Destroying order: A first, B second */
    for (int i = ctx->bean_count - 1; i >= 0; i--) {
        BeanEntry *entry = &ctx->beans[i];

        if (entry->scope == SCOPE_SINGLETON && entry->instance != NULL) {
            printf("\n  [close] Destroying singleton bean: '%s'\n", entry->name);

            if (entry->destroy_fn) {
                entry->destroy_fn(entry->instance);   /* @PreDestroy */
            }

            entry->instance  = NULL;
            entry->lifecycle = BEAN_DESTROYED;
        }
    }

    free(ctx);
    printf("\n[ApplicationContext] closed.\n");
}

/* context_print_summary()
 * Pretty-prints all registered beans — like Spring Actuator /beans endpoint */
static void context_print_summary(ApplicationContext *ctx) {
    printf("\n━━━ ApplicationContext Bean Summary (%d beans) ━━━━━━━━━━━━━\n",
           ctx->bean_count);
    printf("  %-24s %-12s %-14s %s\n",
           "Name", "Scope", "Lifecycle", "Instance Address");
    printf("  %-24s %-12s %-14s %s\n",
           "────────────────────────",
           "────────────",
           "──────────────",
           "───────────────────");

    for (int i = 0; i < ctx->bean_count; i++) {
        BeanEntry *e = &ctx->beans[i];

        const char *scope_str = (e->scope == SCOPE_SINGLETON) ? "singleton" : "prototype";
        const char *lc_str;
        switch (e->lifecycle) {
            case BEAN_REGISTERED:  lc_str = "registered";  break;
            case BEAN_CREATED:     lc_str = "created";     break;
            case BEAN_INITIALIZED: lc_str = "initialized"; break;
            case BEAN_DESTROYED:   lc_str = "destroyed";   break;
            default:               lc_str = "unknown";
        }

        printf("  %-24s %-12s %-14s %p\n",
               e->name, scope_str, lc_str, e->instance);
    }
    printf("\n");
}


/* ═══════════════════════════════════════════════════════════
 *  DEMO FUNCTIONS
 * ═══════════════════════════════════════════════════════════ */

void demo_basic_ioc(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Basic IoC Container\n");
    printf("Java equivalent:\n");
    printf("  ApplicationContext ctx = new AnnotationConfigApplicationContext(AppConfig.class);\n");
    printf("  UserService svc = ctx.getBean(UserService.class);\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Step 1: Create empty container */
    ApplicationContext *ctx = context_create();

    /* Step 2: Register bean definitions (like @Component scanning) */
    printf("\n── Registering Bean Definitions (like @Component scan) ──\n");
    context_register_bean(ctx, "databaseConfig",
                          SCOPE_SINGLETON,
                          create_database_config,
                          init_database_config,
                          destroy_database_config);

    context_register_bean(ctx, "userRepository",
                          SCOPE_SINGLETON,
                          create_user_repository,
                          init_user_repository,
                          destroy_user_repository);

    context_register_bean(ctx, "userService",
                          SCOPE_SINGLETON,
                          create_user_service,
                          init_user_service,
                          destroy_user_service);

    /* Step 3: refresh() — this is where Spring creates all beans */
    printf("\n── Starting Container: context_refresh() ──────────────\n");
    context_refresh(ctx);

    /* Step 4: Show the bean registry */
    context_print_summary(ctx);

    /* Step 5: getBean() — retrieve beans */
    printf("── Retrieving Beans: context_get_bean() ───────────────\n");
    UserService    *svc1  = context_get_bean(ctx, "userService");
    UserService    *svc2  = context_get_bean(ctx, "userService");   /* same pointer! */
    UserRepository *repo  = context_get_bean(ctx, "userRepository");

    /* SINGLETON PROOF: same memory address */
    printf("\n[SINGLETON PROOF] svc1 == svc2? %s\n",
           svc1 == svc2 ? "YES — same instance (correct!)" : "NO — different instances (wrong!)");
    printf("  svc1  @ %p\n", (void*)svc1);
    printf("  svc2  @ %p\n", (void*)svc2);
    printf("  repo  @ %p\n", (void*)repo);

    /* Step 6: close() — runs @PreDestroy on all beans */
    printf("\n── Closing Container: context_close() ─────────────────\n");
    context_close(ctx);
}

void demo_prototype_scope(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Prototype Scope\n");
    printf("Java equivalent:\n");
    printf("  @Scope(\"prototype\") @Component class OrderService { }\n");
    printf("  OrderService o1 = ctx.getBean(OrderService.class); // new instance\n");
    printf("  OrderService o2 = ctx.getBean(OrderService.class); // DIFFERENT instance\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    ApplicationContext *ctx = context_create();

    /* Mix singleton and prototype */
    context_register_bean(ctx, "userService",
                          SCOPE_SINGLETON,
                          create_user_service,
                          init_user_service,
                          destroy_user_service);

    context_register_bean(ctx, "orderService",       /* prototype! */
                          SCOPE_PROTOTYPE,
                          create_order_service,
                          init_order_service,
                          destroy_order_service);

    context_refresh(ctx);
    context_print_summary(ctx);

    /* getBean(prototype) creates a NEW object every call */
    printf("── Getting prototype beans (each call = new object) ───\n");
    OrderService *order1 = context_get_bean(ctx, "orderService");
    OrderService *order2 = context_get_bean(ctx, "orderService");
    OrderService *order3 = context_get_bean(ctx, "orderService");

    printf("\n[PROTOTYPE PROOF] All three orders are DIFFERENT objects:\n");
    printf("  order1 #%d @ %p\n", order1->order_id, (void*)order1);
    printf("  order2 #%d @ %p\n", order2->order_id, (void*)order2);
    printf("  order3 #%d @ %p\n", order3->order_id, (void*)order3);

    printf("\n[NOTE] Spring does NOT destroy prototype beans for you.\n");
    printf("       Caller is responsible. We free manually:\n");

    /* Spring does NOT call @PreDestroy for prototypes.
     * The caller must manage lifecycle.
     * destroy_order_service is always defined here, call directly. */
    destroy_order_service(order1);
    destroy_order_service(order2);
    destroy_order_service(order3);

    context_close(ctx);
}

void demo_bean_not_found(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: Bean Not Found\n");
    printf("Java equivalent:\n");
    printf("  ctx.getBean(\"nonExistentService\");\n");
    printf("  → throws NoSuchBeanDefinitionException\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    ApplicationContext *ctx = context_create();
    context_register_bean(ctx, "userService",
                          SCOPE_SINGLETON,
                          create_user_service,
                          init_user_service,
                          destroy_user_service);
    context_refresh(ctx);

    printf("Attempting to get 'paymentService' (not registered)...\n");
    void *result = context_get_bean(ctx, "paymentService");
    printf("Result: %s\n", result == NULL ? "NULL (Spring throws NoSuchBeanDefinitionException)" : "Found");

    context_close(ctx);
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  bean_factory.c — Spring IoC Container in C         ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_basic_ioc();
    demo_prototype_scope();
    demo_bean_not_found();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  IoC Container = HashMap of bean name → object instance\n");
    printf("  refresh()     = create all singletons + run @PostConstruct\n");
    printf("  getBean()     = lookup in map (singleton) or create new (prototype)\n");
    printf("  close()       = run @PreDestroy in REVERSE order\n");
    printf("  Prototype lifecycle = caller's responsibility, Spring won't destroy\n");
    return 0;
}
