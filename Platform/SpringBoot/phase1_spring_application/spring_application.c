/*
 * spring_application.c  —  SpringApplication.run() Lifecycle in C
 *
 * Build:  gcc -Wall -Wextra -o spring_application spring_application.c
 * Run:    ./spring_application
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.boot.SpringApplication.run()
 *
 *   This is the ENTIRE Spring Boot startup sequence in ~400 lines of C.
 *   Real Spring Boot has ~15,000 lines across dozens of classes.
 *   The LOGIC is identical — just more pluggable.
 *
 *   The startup sequence is a LINEAR STATE MACHINE:
 *     STARTING → ENVIRONMENT_READY → CONTEXT_CREATED →
 *     CONTEXT_PREPARED → REFRESHED → STARTED → READY
 *
 *   Each state transition:
 *     1. Does real work
 *     2. Publishes an ApplicationEvent so listeners can react
 *
 * KEY INSIGHT:
 *   Spring Boot startup is deterministic and ordered.
 *   There is NO parallelism in the startup path itself.
 *   Beans are created sequentially (though they may spawn threads later).
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════
 *  APPLICATION TYPE
 *  Java: WebApplicationType enum
 *
 *  Spring Boot deduces this at startup by checking if certain
 *  classes are present on the classpath.
 *  In our simulation, we accept it as a parameter.
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    APP_TYPE_SERVLET,   /* spring-webmvc on classpath → Tomcat embedded  */
    APP_TYPE_REACTIVE,  /* spring-webflux on classpath → Netty embedded  */
    APP_TYPE_NONE,      /* no web libs → no server, exits after runners  */
} ApplicationType;

static const char* app_type_name(ApplicationType t) {
    switch (t) {
        case APP_TYPE_SERVLET:  return "SERVLET";
        case APP_TYPE_REACTIVE: return "REACTIVE";
        case APP_TYPE_NONE:     return "NONE";
        default:                return "UNKNOWN";
    }
}

/* ═══════════════════════════════════════════════════════════
 *  STARTUP PHASE — tracks current state in the state machine
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    PHASE_CREATED,              /* SpringApplication constructed        */
    PHASE_STARTING,             /* run() called, before env            */
    PHASE_ENVIRONMENT_READY,    /* env loaded (yml/properties/args)    */
    PHASE_CONTEXT_CREATED,      /* empty ApplicationContext built      */
    PHASE_CONTEXT_PREPARED,     /* beans defined, not yet created      */
    PHASE_REFRESHED,            /* ALL beans created, server started   */
    PHASE_STARTED,              /* runners executed                    */
    PHASE_READY,                /* ApplicationReadyEvent published     */
    PHASE_FAILED,               /* any step threw an exception         */
} StartupPhase;

static const char* phase_name(StartupPhase p) {
    static const char* names[] = {
        "CREATED", "STARTING", "ENVIRONMENT_READY", "CONTEXT_CREATED",
        "CONTEXT_PREPARED", "REFRESHED", "STARTED", "READY", "FAILED"
    };
    return names[(int)p < 9 ? (int)p : 8];
}

/* ═══════════════════════════════════════════════════════════
 *  APPLICATION EVENT TYPES
 *  Java: each is a class extending ApplicationEvent
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    EVENT_STARTING,                /* first event, before env            */
    EVENT_ENVIRONMENT_PREPARED,    /* env ready, no context yet          */
    EVENT_CONTEXT_INITIALIZED,     /* context created + initializers run */
    EVENT_PREPARED,                /* context loaded, beans defined      */
    EVENT_STARTED,                 /* after refresh, before runners      */
    EVENT_READY,                   /* fully ready to serve traffic       */
    EVENT_FAILED,                  /* startup failed                     */
} EventType;

static const char* event_name(EventType e) {
    static const char* names[] = {
        "ApplicationStartingEvent",
        "ApplicationEnvironmentPreparedEvent",
        "ApplicationContextInitializedEvent",
        "ApplicationPreparedEvent",
        "ApplicationStartedEvent",
        "ApplicationReadyEvent",
        "ApplicationFailedEvent"
    };
    return names[(int)e < 7 ? (int)e : 6];
}

/* ═══════════════════════════════════════════════════════════
 *  ENVIRONMENT — simulates Spring's ConfigurableEnvironment
 *  Holds properties from all sources (yml, args, system, env vars)
 * ═══════════════════════════════════════════════════════════ */
#define MAX_PROPS 64

typedef struct {
    char key[64];
    char value[256];
    int  priority;   /* lower number = higher priority */
} Property;

typedef struct {
    Property properties[MAX_PROPS];
    int      count;
    char     active_profiles[8][32];
    int      profile_count;
} Environment;

static void env_set(Environment *env, const char *key, const char *val, int priority) {
    /* Higher priority overwrites lower priority values for the same key */
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->properties[i].key, key) == 0) {
            if (priority <= env->properties[i].priority) {
                /* incoming priority is higher (lower number) — overwrite */
                strncpy(env->properties[i].value, val, 255);
                env->properties[i].priority = priority;
            }
            return;
        }
    }
    if (env->count >= MAX_PROPS) return;
    strncpy(env->properties[env->count].key,   key, 63);
    strncpy(env->properties[env->count].value, val, 255);
    env->properties[env->count].priority = priority;
    env->count++;
}

static const char* env_get(const Environment *env, const char *key, const char *def) {
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->properties[i].key, key) == 0) {
            return env->properties[i].value;
        }
    }
    return def;
}

/* ═══════════════════════════════════════════════════════════
 *  APPLICATION LISTENER — simulates Spring ApplicationListener<E>
 *  Java: @Component class MyListener implements ApplicationListener<ApplicationReadyEvent>
 * ═══════════════════════════════════════════════════════════ */
typedef void (*ListenerFn)(EventType event, void *ctx_data);

#define MAX_LISTENERS 16

typedef struct {
    const char *name;
    EventType   trigger;    /* which event this listener handles */
    ListenerFn  fn;
} ApplicationListener;

/* ═══════════════════════════════════════════════════════════
 *  RUNNER — simulates ApplicationRunner / CommandLineRunner
 *  Java: @Component class MyRunner implements ApplicationRunner
 * ═══════════════════════════════════════════════════════════ */
typedef void (*RunnerFn)(const char **args, int arg_count);

#define MAX_RUNNERS 8

typedef struct {
    const char *name;
    int         order;
    RunnerFn    fn;
} Runner;

/* ═══════════════════════════════════════════════════════════
 *  FAILURE ANALYZER — produces human-readable startup failure messages
 *  Java: FailureAnalyzer implementations
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    const char *error_pattern;
    const char *description;
    const char *action;
} FailureAnalyzer;

/* ═══════════════════════════════════════════════════════════
 *  SPRING APPLICATION — the main orchestrator
 *  Java: SpringApplication class
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    /* Configuration */
    const char     *application_name;
    ApplicationType app_type;
    int             banner_enabled;
    int             headless;

    /* Command-line args */
    const char    **args;
    int             arg_count;

    /* Startup state */
    StartupPhase    phase;
    int             running;
    clock_t         start_time;

    /* Environment */
    Environment     env;

    /* Listeners */
    ApplicationListener listeners[MAX_LISTENERS];
    int                 listener_count;

    /* Runners */
    Runner  runners[MAX_RUNNERS];
    int     runner_count;

    /* Failure analyzers */
    FailureAnalyzer analyzers[8];
    int             analyzer_count;

    /* Startup timing (for "Started App in 2.3 seconds" log) */
    double elapsed_prepare_ms;
    double elapsed_refresh_ms;
    double elapsed_total_ms;
} SpringApplication;

/* ─────────────────────────────────────────────────────────────
 *  Event publishing — notifies all registered listeners
 *  Java: EventPublishingRunListener.publishEvent()
 * ──────────────────────────────────────────────────────────── */
static void publish_event(SpringApplication *app, EventType event) {
    printf("\n  [Event] ▶ %s\n", event_name(event));
    for (int i = 0; i < app->listener_count; i++) {
        if (app->listeners[i].trigger == event) {
            printf("    [Listener] %s.onApplicationEvent()\n",
                   app->listeners[i].name);
            app->listeners[i].fn(event, app);
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 *  Step-by-step startup phases
 * ──────────────────────────────────────────────────────────── */

/* Step 1: Application starting */
static void step_starting(SpringApplication *app) {
    printf("\n─── Step 1: Starting ─────────────────────────────────\n");
    app->start_time = clock();
    app->phase = PHASE_STARTING;

    printf("  Headless mode: %s\n", app->headless ? "true" : "false");
    printf("  Application type: %s\n", app_type_name(app->app_type));
    printf("  Primary source: %s\n", app->application_name);

    publish_event(app, EVENT_STARTING);
}

/* Step 2: Prepare environment
 * Java: prepareEnvironment()
 * Loads: CLI args > system props > env vars > application.yml > defaults
 */
static void step_prepare_environment(SpringApplication *app) {
    printf("\n─── Step 2: Prepare Environment ─────────────────────\n");
    clock_t t0 = clock();

    /* Priority 1: Command-line args (highest) */
    printf("  [ConfigData] Loading command-line arguments...\n");
    for (int i = 0; i < app->arg_count; i++) {
        if (strncmp(app->args[i], "--", 2) == 0) {
            char key[64], val[256];
            const char *eq = strchr(app->args[i] + 2, '=');
            if (eq) {
                strncpy(key, app->args[i] + 2, (size_t)(eq - app->args[i] - 2));
                key[eq - app->args[i] - 2] = '\0';
                strncpy(val, eq + 1, 255);
                env_set(&app->env, key, val, 1);  /* priority 1 = highest */
                printf("    arg: %s = %s\n", key, val);
            }
        }
    }

    /* Priority 3: System properties (simulated) */
    printf("  [ConfigData] Loading system properties...\n");
    env_set(&app->env, "java.version", "17.0.9", 3);
    env_set(&app->env, "os.name",      "Mac OS X", 3);

    /* Priority 6: application.yml defaults (lowest explicit) */
    printf("  [ConfigData] Loading application.yml...\n");
    env_set(&app->env, "server.port",                   "8080",        6);
    env_set(&app->env, "server.servlet.context-path",   "/",           6);
    env_set(&app->env, "spring.application.name",       "my-app",      6);
    env_set(&app->env, "spring.datasource.url",         "jdbc:h2:mem:testdb", 6);
    env_set(&app->env, "spring.datasource.driver",      "org.h2.Driver", 6);
    env_set(&app->env, "management.endpoints.web.exposure.include", "health,info,metrics", 6);
    env_set(&app->env, "logging.level.root",            "INFO",        6);

    /* Detect active profiles */
    const char *active = env_get(&app->env, "spring.profiles.active", NULL);
    if (active) {
        strncpy(app->env.active_profiles[0], active, 31);
        app->env.profile_count = 1;
        printf("  [ConfigData] Active profiles: %s → loading application-%s.yml\n",
               active, active);
        /* Would load profile-specific properties here */
    } else {
        printf("  [ConfigData] No active profiles (using defaults)\n");
    }

    app->elapsed_prepare_ms =
        (double)(clock() - t0) / CLOCKS_PER_SEC * 1000.0;

    printf("  Environment ready (%d properties loaded)\n", app->env.count);
    app->phase = PHASE_ENVIRONMENT_READY;
    publish_event(app, EVENT_ENVIRONMENT_PREPARED);
}

/* Step 3: Print banner */
static void step_print_banner(SpringApplication *app) {
    if (!app->banner_enabled) return;
    printf("\n─── Step 3: Banner ───────────────────────────────────\n");
    /* Spring Boot default banner */
    printf("\n");
    printf("  .   ____          _            __ _ _\n");
    printf(" /\\\\ / ___'_ __ _ _(_)_ __  __ _ \\ \\ \\ \\\n");
    printf("( ( )___ | '_ | '_| | '_ \\/ _` | \\ \\ \\ \\\n");
    printf(" \\\\/  ___)| |_)| | | | | || (_| |  ) ) ) )\n");
    printf("  '  |____| .__|_| |_|_| |_\\__, | / / / /\n");
    printf(" =========|_|==============|___/=/_/_/_/\n");
    printf(" :: Spring Boot ::                (v3.2.0)\n\n");
    printf("  [Banner] spring.banner.mode = %s\n",
           app->banner_enabled ? "CONSOLE" : "OFF");
}

/* Step 4: Create ApplicationContext */
static void step_create_context(SpringApplication *app) {
    printf("\n─── Step 4: Create ApplicationContext ───────────────\n");

    switch (app->app_type) {
        case APP_TYPE_SERVLET:
            printf("  Creating: AnnotationConfigServletWebServerApplicationContext\n");
            printf("  Reason:   spring-webmvc detected on classpath\n");
            break;
        case APP_TYPE_REACTIVE:
            printf("  Creating: AnnotationConfigReactiveWebServerApplicationContext\n");
            printf("  Reason:   spring-webflux detected (no spring-webmvc)\n");
            break;
        case APP_TYPE_NONE:
            printf("  Creating: AnnotationConfigApplicationContext\n");
            printf("  Reason:   no web libraries on classpath\n");
            break;
    }

    app->phase = PHASE_CONTEXT_CREATED;
    publish_event(app, EVENT_CONTEXT_INITIALIZED);
}

/* Step 5: Prepare context — register bean definitions */
static void step_prepare_context(SpringApplication *app) {
    printf("\n─── Step 5: Prepare Context ─────────────────────────\n");

    printf("  Running ApplicationContextInitializers...\n");
    printf("  Registering primary source: %s\n", app->application_name);
    printf("  BeanDefinition loading: deferred to refresh()\n");

    app->phase = PHASE_CONTEXT_PREPARED;
    publish_event(app, EVENT_PREPARED);
}

/* Step 6: Refresh context — THE BIG STEP
 * This is where Spring Framework does all its work (see Spring/ folder).
 * From Boot's perspective, this is a black box that:
 *   - processes @ComponentScan
 *   - evaluates all @Conditional auto-configurations
 *   - creates all singleton beans
 *   - starts embedded server
 */
static void step_refresh_context(SpringApplication *app) {
    printf("\n─── Step 6: Refresh Context ◄━━━━ THE BIG STEP ━━━━━\n");
    clock_t t0 = clock();

    printf("  [refresh] invokeBeanFactoryPostProcessors...\n");
    printf("    → ConfigurationClassPostProcessor parsing @SpringBootApplication\n");
    printf("    → @ComponentScan: scanning com.example (YOUR beans)\n");
    printf("    → @EnableAutoConfiguration: loading AutoConfiguration.imports\n");
    printf("    → Evaluating %d auto-configuration candidates...\n", 150);

    /* Simulate auto-config selection */
    const char *active_configs[] = {
        "DataSourceAutoConfiguration         ← @ConditionalOnClass(DataSource)",
        "HikariDataSourceConfiguration       ← @ConditionalOnClass(HikariDataSource)",
        "JdbcTemplateAutoConfiguration       ← @ConditionalOnSingleCandidate(DataSource)",
        "WebMvcAutoConfiguration             ← @ConditionalOnWebApplication(SERVLET)",
        "ErrorMvcAutoConfiguration           ← @ConditionalOnWebApplication(SERVLET)",
        "DispatcherServletAutoConfiguration  ← @ConditionalOnClass(DispatcherServlet)",
        "HttpMessageConvertersAutoConfig     ← @ConditionalOnClass(HttpMessageConverter)",
        "JacksonAutoConfiguration            ← @ConditionalOnClass(ObjectMapper)",
        "TransactionAutoConfiguration        ← @ConditionalOnClass(PlatformTransactionManager)",
        "AopAutoConfiguration                ← @ConditionalOnClass(Advice)",
    };
    for (int i = 0; i < 10; i++) {
        printf("    ✓ MATCH: %s\n", active_configs[i]);
    }
    printf("    ✗ SKIP:  %d configs (conditions not met)\n", 140);

    printf("\n  [refresh] registerBeanPostProcessors...\n");
    printf("    → AutowiredAnnotationBeanPostProcessor\n");
    printf("    → CommonAnnotationBeanPostProcessor\n");

    printf("\n  [refresh] onRefresh() → starting embedded server...\n");
    const char *port = env_get(&app->env, "server.port", "8080");
    printf("    → TomcatServletWebServerFactory.getWebServer()\n");
    printf("    → Creating Tomcat instance\n");
    printf("    → Binding connector to port %s\n", port);
    printf("    → Registering DispatcherServlet at '/'\n");
    printf("    → Tomcat.start() — server now accepting connections\n");

    printf("\n  [refresh] finishBeanFactoryInitialization...\n");
    printf("    → Instantiating all non-lazy singletons...\n");

    /* Simulate bean creation order */
    const char *beans[] = {
        "hikariDataSource", "jdbcTemplate", "transactionManager",
        "userRepository", "userService", "userController",
        "dispatcherServlet", "requestMappingHandlerMapping",
        "healthEndpoint", "infoEndpoint"
    };
    for (int i = 0; i < 10; i++) {
        printf("    ✓ %s\n", beans[i]);
    }

    app->elapsed_refresh_ms =
        (double)(clock() - t0) / CLOCKS_PER_SEC * 1000.0;

    printf("\n  [refresh] finishRefresh() — publishing ContextRefreshedEvent\n");
    app->phase = PHASE_REFRESHED;
}

/* Step 7: Post-refresh — run runners */
static void step_start(SpringApplication *app) {
    printf("\n─── Step 7: Post-Refresh ─────────────────────────────\n");
    publish_event(app, EVENT_STARTED);

    printf("\n  Running ApplicationRunners and CommandLineRunners...\n");
    for (int i = 0; i < app->runner_count; i++) {
        printf("  [Runner @Order(%d)] %s\n", app->runners[i].order, app->runners[i].name);
        app->runners[i].fn(app->args, app->arg_count);
    }

    app->phase = PHASE_STARTED;
}

/* Step 8: Ready */
static void step_ready(SpringApplication *app) {
    printf("\n─── Step 8: Application Ready ───────────────────────\n");

    app->elapsed_total_ms =
        (double)(clock() - app->start_time) / CLOCKS_PER_SEC * 1000.0;

    publish_event(app, EVENT_READY);

    const char *port = env_get(&app->env, "server.port", "8080");
    const char *name = env_get(&app->env, "spring.application.name", app->application_name);
    printf("\n");
    printf("  Started '%s' in %.3f seconds (JVM running for %.3f)\n",
           name,
           app->elapsed_total_ms / 1000.0,
           app->elapsed_total_ms / 1000.0 + 0.535);
    printf("  Listening: http://localhost:%s\n", port);

    app->phase = PHASE_READY;
    app->running = 1;
}

/* Step 9: Handle failure */
static void step_failed(SpringApplication *app, const char *reason) {
    app->phase = PHASE_FAILED;
    publish_event(app, EVENT_FAILED);

    printf("\n");
    printf("  ***************************\n");
    printf("  APPLICATION FAILED TO START\n");
    printf("  ***************************\n\n");

    /* Run failure analyzers */
    for (int i = 0; i < app->analyzer_count; i++) {
        if (strstr(reason, app->analyzers[i].error_pattern)) {
            printf("  Description:\n  %s\n\n", app->analyzers[i].description);
            printf("  Action:\n  %s\n", app->analyzers[i].action);
            return;
        }
    }
    printf("  Description:\n  %s\n", reason);
}

/* ─────────────────────────────────────────────────────────────
 *  MAIN RUN METHOD
 *  Java: SpringApplication.run()
 * ──────────────────────────────────────────────────────────── */
static void spring_application_run(SpringApplication *app) {
    step_starting(app);
    step_prepare_environment(app);
    step_print_banner(app);
    step_create_context(app);
    step_prepare_context(app);
    step_refresh_context(app);
    step_start(app);
    step_ready(app);
}

/* ═══════════════════════════════════════════════════════════
 *  SAMPLE LISTENERS AND RUNNERS FOR DEMO
 * ═══════════════════════════════════════════════════════════ */

/* Listener: log environment prepared */
void listener_env_prepared(EventType e, void *ctx) {
    (void)e;
    SpringApplication *app = (SpringApplication*)ctx;
    const char *port = env_get(&app->env, "server.port", "8080");
    printf("    → Application will start on port: %s\n", port);
}

/* Listener: log when ready */
void listener_app_ready(EventType e, void *ctx) {
    (void)e; (void)ctx;
    printf("    → [LoggingApplicationListener] Application is ready!\n");
}

/* Listener: log failure */
void listener_app_failed(EventType e, void *ctx) {
    (void)e; (void)ctx;
    printf("    → [DiagnosticsListener] Gathering startup failure diagnostics...\n");
}

/* Runner: data initializer */
void runner_data_init(const char **args, int n) {
    (void)args; (void)n;
    printf("    Checking database schema...\n");
    printf("    Schema is up-to-date\n");
}

/* Runner: cache warmer */
void runner_cache_warm(const char **args, int n) {
    (void)args; (void)n;
    printf("    Warming up application caches...\n");
    printf("    Loaded 150 cached entries\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_web_application_startup(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Web Application Startup (SERVLET type)\n");
    printf("Java: @SpringBootApplication + spring-boot-starter-web\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    const char *args[] = { "--server.port=9090", "--spring.profiles.active=dev" };

    SpringApplication app = {0};
    app.application_name = "MyWebApp";
    app.app_type         = APP_TYPE_SERVLET;
    app.banner_enabled   = 1;
    app.headless         = 1;
    app.args             = args;
    app.arg_count        = 2;

    /* Register event listeners */
    app.listeners[app.listener_count++] = (ApplicationListener){
        "PortLogListener", EVENT_ENVIRONMENT_PREPARED, listener_env_prepared
    };
    app.listeners[app.listener_count++] = (ApplicationListener){
        "ReadyLogListener", EVENT_READY, listener_app_ready
    };
    app.listeners[app.listener_count++] = (ApplicationListener){
        "FailureListener", EVENT_FAILED, listener_app_failed
    };

    /* Register runners */
    app.runners[app.runner_count++] = (Runner){
        "DataInitializer", 1, runner_data_init
    };
    app.runners[app.runner_count++] = (Runner){
        "CacheWarmer", 2, runner_cache_warm
    };

    /* Register failure analyzers */
    app.analyzers[app.analyzer_count++] = (FailureAnalyzer){
        "Address already in use",
        "Web server failed to start. Port 9090 was already in use.",
        "Identify and stop the process that's listening on port 9090\n"
        "  or configure: server.port=<different port>"
    };

    spring_application_run(&app);
}

void demo_non_web_application(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Non-Web Application (NONE type)\n");
    printf("Java: @SpringBootApplication (no web starter)\n");
    printf("      → runs runners then exits\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    SpringApplication app = {0};
    app.application_name = "BatchProcessor";
    app.app_type         = APP_TYPE_NONE;
    app.banner_enabled   = 0;   /* banner off for batch jobs */
    app.headless         = 1;

    app.runners[app.runner_count++] = (Runner){
        "BatchJobRunner", 1, runner_data_init
    };

    spring_application_run(&app);

    printf("\n  [NONE type] No server started. Application will exit after runners.\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  spring_application.c — SpringApplication.run() in C║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_web_application_startup();
    demo_non_web_application();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  SpringApplication.run() = 8-step state machine\n");
    printf("  Each step publishes an ApplicationEvent for listeners\n");
    printf("  Step 6 (refresh) = entire Spring Framework doing its work\n");
    printf("  CLI args override application.yml (higher priority)\n");
    printf("  SERVLET type → embedded server starts during refresh\n");
    printf("  NONE type → no server, exits after runners complete\n");
    printf("  FailureAnalyzers → human-readable startup error messages\n");
    return 0;
}
