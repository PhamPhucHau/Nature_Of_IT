/*
 * startup_events.c  —  Spring Boot ApplicationEvent Timeline in C
 *
 * Build:  gcc -Wall -Wextra -o startup_events startup_events.c
 * Run:    ./startup_events
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   Spring Boot's startup event sequence — WHEN each event fires,
 *   WHAT is available at each point, and WHY events exist.
 *
 *   Key Spring Boot classes:
 *     EventPublishingRunListener     ← publishes events during run()
 *     SpringApplicationRunListener  ← interface for run() lifecycle callbacks
 *     ApplicationListener<E>        ← your beans can listen to any event
 *
 *   Special case: The EARLY events (ApplicationStartingEvent,
 *   ApplicationEnvironmentPreparedEvent) fire BEFORE the ApplicationContext
 *   is created. So @Component beans cannot listen to them.
 *   You must register listeners in META-INF/spring.factories or programmatically.
 *
 *   ONLY from ApplicationPreparedEvent onward can @Component @EventListener
 *   beans receive these events.
 *
 * USE CASES FOR EACH EVENT:
 *   ApplicationStartingEvent        → very early config (log system init)
 *   ApplicationEnvironmentPreparedEvent → inspect/modify environment before context
 *   ApplicationContextInitializedEvent  → before bean definitions loaded
 *   ApplicationPreparedEvent            → beans defined, not yet created
 *   ApplicationStartedEvent             → after refresh, before runners
 *   ApplicationReadyEvent               → fully ready (cache warm, health checks)
 *   ApplicationFailedEvent              → cleanup on startup failure
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  EVENT DESCRIPTOR
 *  Each Spring Boot event has a specific moment in startup
 *  and specific constraints on what's available.
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    const char *event_class;         /* Java class name                    */
    const char *when_fired;          /* exact timing in startup            */
    int         context_available;   /* 1 = ApplicationContext exists      */
    int         beans_available;     /* 1 = singleton beans are ready      */
    int         server_started;      /* 1 = embedded server is running     */
    const char *how_to_listen;       /* how to register for this event     */
    const char *typical_use;         /* what people use this event for     */
    const char *source_method;       /* Spring Boot source method that fires it */
} EventDescriptor;

static const EventDescriptor BOOT_EVENTS[] = {
    {
        .event_class       = "ApplicationStartingEvent",
        .when_fired        = "Before ANYTHING — first line of run()",
        .context_available = 0,
        .beans_available   = 0,
        .server_started    = 0,
        .how_to_listen     = "spring.factories OR app.addListeners() BEFORE run()",
        .typical_use       = "Initialize logging system, early diagnostic setup",
        .source_method     = "SpringApplicationRunListeners.starting()"
    },
    {
        .event_class       = "ApplicationEnvironmentPreparedEvent",
        .when_fired        = "After environment loaded (yml, args, system props)",
        .context_available = 0,   /* context NOT created yet! */
        .beans_available   = 0,
        .server_started    = 0,
        .how_to_listen     = "spring.factories OR app.addListeners() BEFORE run()",
        .typical_use       = "Decrypt encrypted properties, add custom property sources",
        .source_method     = "SpringApplicationRunListeners.environmentPrepared()"
    },
    {
        .event_class       = "ApplicationContextInitializedEvent",
        .when_fired        = "After context created, after initializers ran, BEFORE bean defs loaded",
        .context_available = 1,
        .beans_available   = 0,
        .server_started    = 0,
        .how_to_listen     = "spring.factories OR app.addListeners() BEFORE run()",
        .typical_use       = "Register additional beans programmatically before scanning",
        .source_method     = "SpringApplicationRunListeners.contextPrepared()"
    },
    {
        .event_class       = "ApplicationPreparedEvent",
        .when_fired        = "After bean definitions loaded, BEFORE refresh (beans not created)",
        .context_available = 1,
        .beans_available   = 0,   /* definitions exist, instances DO NOT */
        .server_started    = 0,
        .how_to_listen     = "spring.factories (FIRST time @Component beans CAN listen via @EventListener)",
        .typical_use       = "Modify bean definitions before instantiation",
        .source_method     = "SpringApplicationRunListeners.contextLoaded()"
    },
    {
        .event_class       = "ContextRefreshedEvent",
        .when_fired        = "After ALL beans created, server started — end of refresh()",
        .context_available = 1,
        .beans_available   = 1,
        .server_started    = 1,
        .how_to_listen     = "@EventListener on any @Component bean",
        .typical_use       = "Verify all required beans exist, trigger post-init tasks",
        .source_method     = "AbstractApplicationContext.finishRefresh()"
    },
    {
        .event_class       = "ServletWebServerInitializedEvent",
        .when_fired        = "After embedded server started (sub-event of ContextRefreshedEvent)",
        .context_available = 1,
        .beans_available   = 1,
        .server_started    = 1,
        .how_to_listen     = "@EventListener WebServerInitializedEvent",
        .typical_use       = "Log actual port if server.port=0 (random port)",
        .source_method     = "WebServerStartStopLifecycle.start()"
    },
    {
        .event_class       = "ApplicationStartedEvent",
        .when_fired        = "After refresh, BEFORE runners execute",
        .context_available = 1,
        .beans_available   = 1,
        .server_started    = 1,
        .how_to_listen     = "@EventListener on any @Component bean",
        .typical_use       = "Metrics recording (startup time), pre-runner health checks",
        .source_method     = "SpringApplicationRunListeners.started()"
    },
    {
        .event_class       = "ApplicationReadyEvent",
        .when_fired        = "After ALL runners complete — app fully ready",
        .context_available = 1,
        .beans_available   = 1,
        .server_started    = 1,
        .how_to_listen     = "@EventListener on any @Component bean (most common)",
        .typical_use       = "Cache warmup, notify monitoring, ready health checks",
        .source_method     = "SpringApplicationRunListeners.ready()"
    },
    {
        .event_class       = "ApplicationFailedEvent",
        .when_fired        = "If ANY step above throws an exception",
        .context_available = 0,   /* may or may not exist depending on when failure */
        .beans_available   = 0,
        .server_started    = 0,
        .how_to_listen     = "spring.factories (context may not exist)",
        .typical_use       = "Alert monitoring, cleanup temp files, notify on-call",
        .source_method     = "SpringApplication.handleRunFailure()"
    },
};

static int event_count = (int)(sizeof(BOOT_EVENTS) / sizeof(BOOT_EVENTS[0]));

/* ═══════════════════════════════════════════════════════════
 *  LISTENER REGISTRATION METHODS
 *  Different events require different registration approaches.
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    const char *method;
    const char *when_available;
    const char *example;
    const char *limitation;
} ListenerRegistrationMethod;

static const ListenerRegistrationMethod REGISTRATION_METHODS[] = {
    {
        "spring.factories file",
        "Always — even for the earliest events",
        "org.springframework.context.ApplicationListener=com.example.MyEarlyListener",
        "Must be in a JAR on classpath (or src/main/resources/META-INF/)"
    },
    {
        "app.addListeners(new MyListener())",
        "Before SpringApplication.run() is called",
        "SpringApplication app = new SpringApplication(MyApp.class);\n"
        "  app.addListeners(new MyEarlyListener());\n"
        "  app.run(args);",
        "Programmatic — listener must be instantiated before run()"
    },
    {
        "@EventListener on @Component bean",
        "From ApplicationPreparedEvent onward (context must exist)",
        "@Component class MyListener {\n"
        "  @EventListener\n"
        "  void onReady(ApplicationReadyEvent e) { ... }\n"
        "}",
        "Cannot receive ApplicationStartingEvent or ApplicationEnvironmentPreparedEvent"
    },
    {
        "implements ApplicationListener<E> + @Component",
        "From ApplicationPreparedEvent onward",
        "@Component class MyListener implements ApplicationListener<ApplicationReadyEvent> {\n"
        "  public void onApplicationEvent(ApplicationReadyEvent e) { ... }\n"
        "}",
        "Same as @EventListener — no early events"
    },
};

/* ─────────────────────────────────────────────────────────────
 *  Print the event timeline in a visual format
 * ──────────────────────────────────────────────────────────── */
static void print_event_timeline(void) {
    printf("╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║           Spring Boot Startup Events Timeline                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════════╝\n\n");

    printf("  %-48s  %s  %s  %s\n",
           "Event", "Ctx", "Bns", "Srv");
    printf("  %-48s  %s  %s  %s\n",
           "────────────────────────────────────────────────",
           "───", "───", "───");

    for (int i = 0; i < event_count; i++) {
        const EventDescriptor *e = &BOOT_EVENTS[i];
        printf("  %-48s  %-3s  %-3s  %-3s\n",
               e->event_class,
               e->context_available ? "YES" : "NO ",
               e->beans_available   ? "YES" : "NO ",
               e->server_started    ? "YES" : "NO ");
    }

    printf("\n  Legend: Ctx=context, Bns=beans created, Srv=server started\n\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Print full detail for each event
 * ──────────────────────────────────────────────────────────── */
static void print_event_details(void) {
    printf("━━━ Event Details ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    for (int i = 0; i < event_count; i++) {
        const EventDescriptor *e = &BOOT_EVENTS[i];
        printf("  [%d] %s\n", i + 1, e->event_class);
        printf("       When:     %s\n", e->when_fired);
        printf("       Listen:   %s\n", e->how_to_listen);
        printf("       Use for:  %s\n", e->typical_use);
        printf("       Source:   %s\n\n", e->source_method);
    }
}

/* ─────────────────────────────────────────────────────────────
 *  Simulate event sequence with listener responses
 * ──────────────────────────────────────────────────────────── */
static void simulate_event_sequence(void) {
    printf("━━━ Simulated Startup with Listener Responses ━━━━━━━━━━━━━━━━━━\n\n");

    /* Define what each listener does at each event */
    struct {
        const char *listener_name;
        const char *event;
        const char *action;
    } listener_responses[] = {
        /* LoggingSystem listener — registered via spring.factories */
        {"LoggingSystem",          "ApplicationStartingEvent",
         "Initializing logging system before any log output"},

        /* ConfigFileApplicationListener — reads application.yml */
        {"ConfigFileListener",     "ApplicationEnvironmentPreparedEvent",
         "Loading application.yml → adding 24 property keys"},

        /* AnsiOutputApplicationListener */
        {"AnsiOutputListener",     "ApplicationEnvironmentPreparedEvent",
         "Configuring ANSI color output for console"},

        /* LoggingApplicationListener */
        {"LoggingApplicationListener", "ApplicationPreparedEvent",
         "Configuring log levels from spring.logging.level.*"},

        /* Your own listener registered in spring.factories */
        {"EncryptionPostProcessor", "ApplicationEnvironmentPreparedEvent",
         "Decrypting ENC(...) property values"},

        /* @Component @EventListener bean */
        {"DataSourceHealthChecker", "ContextRefreshedEvent",
         "Verifying database connection is healthy"},

        /* @EventListener ApplicationReadyEvent */
        {"CacheWarmupListener",    "ApplicationReadyEvent",
         "Loading 500 cache entries from database"},

        {"MetricsStartupListener", "ApplicationStartedEvent",
         "Recording startup.time metric: 2.341s"},

        /* Cleanup on failure */
        {"AlertingListener",       "ApplicationFailedEvent",
         "Sending PagerDuty alert: startup failed"},
    };

    int n = (int)(sizeof(listener_responses) / sizeof(listener_responses[0]));

    const char *prev_event = NULL;
    for (int i = 0; i < n; i++) {
        if (prev_event == NULL ||
            strcmp(prev_event, listener_responses[i].event) != 0)
        {
            printf("  ▼ %s\n", listener_responses[i].event);
            prev_event = listener_responses[i].event;
        }
        printf("    [%-28s] %s\n",
               listener_responses[i].listener_name,
               listener_responses[i].action);
    }
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Critical warning: @EventListener vs spring.factories
 * ──────────────────────────────────────────────────────────── */
static void print_registration_warning(void) {
    printf("━━━ CRITICAL: When to Use Which Registration Method ━━━━━━━━━━━━\n\n");

    printf("  PROBLEM:\n");
    printf("  @Component beans are not created until Step 6 (refresh).\n");
    printf("  Events BEFORE refresh fire before @Component beans exist.\n\n");

    printf("  WRONG ✗ — will MISS ApplicationStartingEvent:\n");
    printf("    @Component\n");
    printf("    class MyListener implements ApplicationListener<ApplicationStartingEvent> {\n");
    printf("        // This bean doesn't exist yet when the event fires!\n");
    printf("        // Spring Boot will NOT deliver this event here.\n");
    printf("    }\n\n");

    printf("  CORRECT ✓ — registered BEFORE context exists:\n");
    printf("    // In src/main/resources/META-INF/spring.factories:\n");
    printf("    org.springframework.context.ApplicationListener=\\\n");
    printf("        com.example.MyEarlyListener\n\n");

    printf("  RULE:\n");
    printf("  ┌────────────────────────────────────────────────────────────┐\n");
    printf("  │  ApplicationStartingEvent         → spring.factories ONLY │\n");
    printf("  │  ApplicationEnvironmentPreparedEvent → spring.factories   │\n");
    printf("  │  ApplicationContextInitializedEvent  → spring.factories   │\n");
    printf("  │  ApplicationPreparedEvent            → spring.factories   │\n");
    printf("  │  ContextRefreshedEvent               → @EventListener OK  │\n");
    printf("  │  ApplicationStartedEvent             → @EventListener OK  │\n");
    printf("  │  ApplicationReadyEvent               → @EventListener OK  │\n");
    printf("  │  ApplicationFailedEvent              → spring.factories   │\n");
    printf("  └────────────────────────────────────────────────────────────┘\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  startup_events.c — Spring Boot Event Timeline in C ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    print_event_timeline();
    print_event_details();
    simulate_event_sequence();
    print_registration_warning();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  7 events fire between main() and 'Started App'\n");
    printf("  Early events (before context) need spring.factories registration\n");
    printf("  Late events (after refresh) work with @EventListener @Component\n");
    printf("  ApplicationReadyEvent = safe 'app is fully up' hook\n");
    printf("  ApplicationFailedEvent = cleanup + alerting on startup crash\n");
    return 0;
}
