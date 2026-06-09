/*
 * embedded_tomcat.c  —  Spring Boot Embedded Tomcat in C
 *
 * Build:  gcc -Wall -Wextra -o embedded_tomcat embedded_tomcat.c
 * Run:    ./embedded_tomcat
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.boot.web.embedded.tomcat.TomcatServletWebServerFactory
 *   org.springframework.boot.web.embedded.tomcat.TomcatWebServer
 *
 *   Spring Boot's embedded Tomcat starts the entire HTTP server
 *   from within the application JVM process.
 *   No external Tomcat installation is required.
 *
 *   Real flow inside Spring Boot:
 *     ServletWebServerApplicationContext.onRefresh()
 *       → createWebServer()
 *         → TomcatServletWebServerFactory.getWebServer()
 *           → new Tomcat()
 *           → connector = new Connector("HTTP/1.1")
 *           → connector.setPort(8080)
 *           → addContext(host, contextPath, docBase)
 *           → registerServlet(ctx, "dispatcherServlet", DispatcherServlet)
 *           → tomcat.start()   ← TCP socket bound
 *           → return TomcatWebServer
 *
 *   What this C file models:
 *     - The Tomcat instance lifecycle (CREATED → CONFIGURED → STARTED → STOPPED)
 *     - Connector configuration (port, thread pool, timeouts)
 *     - Context and servlet registration
 *     - Port conflict detection
 *     - SSL configuration structure
 *     - Thread pool management (the "executor" inside Tomcat)
 *     - Graceful stop (drain in-flight requests)
 *
 * KEY INSIGHT:
 *   The fat JAR's embedded Tomcat is NOT magic.
 *   Tomcat has always had a Java API: org.apache.catalina.startup.Tomcat
 *   Spring Boot just calls that API programmatically from application code.
 *   The real TCP socket, thread pool, and HTTP parsing are all inside Tomcat.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════
 *  SERVER STATE MACHINE
 *  Mirrors Tomcat's LifecycleState enum
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    SERVER_NEW,          /* just constructed                      */
    SERVER_INITIALIZED,  /* init() called                         */
    SERVER_STARTED,      /* start() called, socket bound, running */
    SERVER_STOPPING,     /* stop() called, draining requests      */
    SERVER_STOPPED,      /* fully stopped                         */
    SERVER_FAILED,       /* error during start                    */
} ServerState;

static const char* server_state_name(ServerState s) {
    static const char* names[] = {
        "NEW", "INITIALIZED", "STARTED", "STOPPING", "STOPPED", "FAILED"
    };
    return names[(int)s < 6 ? (int)s : 5];
}

/* ═══════════════════════════════════════════════════════════
 *  THREAD POOL (Tomcat Executor)
 *  Tomcat's NioEndpoint uses a ThreadPoolExecutor internally.
 *  Java: org.apache.tomcat.util.threads.ThreadPoolExecutor
 *
 *  Configured via:
 *    server.tomcat.threads.max         (default: 200)
 *    server.tomcat.threads.min-spare   (default: 10)
 *    server.tomcat.accept-count        (default: 100)  ← request queue size
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    int max_threads;         /* max worker threads (server.tomcat.threads.max)       */
    int min_spare_threads;   /* min idle threads   (server.tomcat.threads.min-spare) */
    int accept_count;        /* request queue size (server.tomcat.accept-count)      */
    int active_threads;      /* currently running  (runtime counter)                 */
    int queued_requests;     /* requests waiting   (runtime counter)                 */
    int total_requests;      /* lifetime request count                               */
} TomcatThreadPool;

/* ═══════════════════════════════════════════════════════════
 *  CONNECTOR  (HTTP or HTTPS)
 *  Java: org.apache.catalina.connector.Connector
 *
 *  One connector = one listening port.
 *  A Tomcat instance can have multiple connectors (HTTP + HTTPS).
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    CONNECTOR_HTTP,    /* plain HTTP/1.1 */
    CONNECTOR_HTTPS,   /* TLS/SSL        */
    CONNECTOR_HTTP2,   /* HTTP/2 (AJP removed in Tomcat 10) */
} ConnectorScheme;

typedef struct {
    ConnectorScheme  scheme;
    int              port;                /* server.port         */
    int              connection_timeout_ms; /* server.connection-timeout */
    int              max_connections;     /* server.tomcat.max-connections */
    int              is_secure;
    /* SSL fields (only used if HTTPS) */
    char             key_store_path[256]; /* server.ssl.key-store */
    char             key_store_password[64];
    char             ssl_protocol[16];    /* TLSv1.2, TLSv1.3    */
} TomcatConnector;

/* ═══════════════════════════════════════════════════════════
 *  SERVLET REGISTRATION
 *  Java: javax.servlet.ServletRegistration.Dynamic
 *  Maps a servlet name + class to URL patterns.
 *  Spring Boot registers ONE servlet: DispatcherServlet → "/"
 * ═══════════════════════════════════════════════════════════ */
#define MAX_SERVLETS 8
#define MAX_FILTERS  16

typedef struct {
    char name[64];
    char class_name[128];
    char url_pattern[64];
    int  load_on_startup;   /* 1 = create at context startup, not on first request */
} ServletRegistration;

typedef struct {
    char name[64];
    char class_name[128];
    char url_pattern[64];   /* "/*" = all requests */
    int  order;
} FilterRegistration;

/* ═══════════════════════════════════════════════════════════
 *  TOMCAT CONTEXT
 *  Java: org.apache.catalina.Context
 *  One context = one web application (one context-path).
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    char                context_path[64];   /* server.servlet.context-path */
    char                doc_base[256];      /* document root               */
    ServletRegistration servlets[MAX_SERVLETS];
    int                 servlet_count;
    FilterRegistration  filters[MAX_FILTERS];
    int                 filter_count;
} TomcatContext;

/* ═══════════════════════════════════════════════════════════
 *  EMBEDDED TOMCAT INSTANCE
 *  Java: org.apache.catalina.startup.Tomcat
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    ServerState    state;
    char           base_dir[256];      /* temp dir for Tomcat work files */
    TomcatConnector connector;
    TomcatContext  context;
    TomcatThreadPool thread_pool;

    /* startup timing */
    clock_t        start_time;
    double         startup_ms;

    /* runtime counters */
    int            in_flight_requests;  /* requests currently being processed */
    long           total_bytes_sent;
} EmbeddedTomcat;

/* ─────────────────────────────────────────────────────────────
 *  tomcat_create() — new Tomcat()
 * ──────────────────────────────────────────────────────────── */
static EmbeddedTomcat* tomcat_create(void) {
    EmbeddedTomcat *t = calloc(1, sizeof(EmbeddedTomcat));
    t->state = SERVER_NEW;

    /* Defaults (mirrors Spring Boot's TomcatServletWebServerFactory) */
    t->connector.scheme              = CONNECTOR_HTTP;
    t->connector.port                = 8080;
    t->connector.connection_timeout_ms = 20000;
    t->connector.max_connections     = 8192;
    t->connector.is_secure           = 0;

    t->thread_pool.max_threads        = 200;
    t->thread_pool.min_spare_threads  = 10;
    t->thread_pool.accept_count       = 100;

    snprintf(t->base_dir, sizeof(t->base_dir), "/tmp/tomcat.%d", (int)time(NULL));
    strncpy(t->context.context_path, "/", sizeof(t->context.context_path) - 1);
    strncpy(t->context.doc_base, ".", sizeof(t->context.doc_base) - 1);

    printf("[Tomcat] Instance created (state=%s)\n", server_state_name(t->state));
    return t;
}

/* ─────────────────────────────────────────────────────────────
 *  tomcat_configure() — applies ServerProperties settings
 *  Called by TomcatServletWebServerFactory before start()
 * ──────────────────────────────────────────────────────────── */
static void tomcat_configure(EmbeddedTomcat *t,
                              int port,
                              int max_threads,
                              int min_spare,
                              const char *context_path)
{
    t->connector.port               = port;
    t->thread_pool.max_threads      = max_threads;
    t->thread_pool.min_spare_threads = min_spare;
    strncpy(t->context.context_path, context_path,
            sizeof(t->context.context_path) - 1);

    t->state = SERVER_INITIALIZED;

    printf("[Tomcat] Configured:\n");
    printf("  port               = %d\n",   t->connector.port);
    printf("  threads.max        = %d\n",   t->thread_pool.max_threads);
    printf("  threads.min-spare  = %d\n",   t->thread_pool.min_spare_threads);
    printf("  accept-count       = %d\n",   t->thread_pool.accept_count);
    printf("  connection-timeout = %d ms\n",t->connector.connection_timeout_ms);
    printf("  max-connections    = %d\n",   t->connector.max_connections);
    printf("  context-path       = '%s'\n", t->context.context_path);
    printf("  base-dir           = %s\n",   t->base_dir);
}

/* ─────────────────────────────────────────────────────────────
 *  tomcat_add_servlet() — ctx.addServlet(name, servlet)
 *  Spring Boot calls this to register DispatcherServlet
 * ──────────────────────────────────────────────────────────── */
static void tomcat_add_servlet(EmbeddedTomcat *t,
                                const char *name,
                                const char *class_name,
                                const char *url_pattern,
                                int load_on_startup)
{
    if (t->context.servlet_count >= MAX_SERVLETS) return;
    ServletRegistration *sr = &t->context.servlets[t->context.servlet_count++];
    strncpy(sr->name,        name,        sizeof(sr->name) - 1);
    strncpy(sr->class_name,  class_name,  sizeof(sr->class_name) - 1);
    strncpy(sr->url_pattern, url_pattern, sizeof(sr->url_pattern) - 1);
    sr->load_on_startup = load_on_startup;

    printf("[Tomcat] Registered servlet: %-28s → '%s'\n", name, url_pattern);
}

/* ─────────────────────────────────────────────────────────────
 *  tomcat_add_filter() — ctx.addFilter(name, filter)
 * ──────────────────────────────────────────────────────────── */
static void tomcat_add_filter(EmbeddedTomcat *t,
                               const char *name,
                               const char *class_name,
                               int order)
{
    if (t->context.filter_count >= MAX_FILTERS) return;
    FilterRegistration *fr = &t->context.filters[t->context.filter_count++];
    strncpy(fr->name,       name,       sizeof(fr->name) - 1);
    strncpy(fr->class_name, class_name, sizeof(fr->class_name) - 1);
    strncpy(fr->url_pattern, "/*",      sizeof(fr->url_pattern) - 1);
    fr->order = order;

    printf("[Tomcat] Registered filter:  %-28s @Order(%d)\n", name, order);
}

/* ─────────────────────────────────────────────────────────────
 *  Simulate port-in-use check
 *  Real Tomcat: ServerSocket.bind() throws BindException if port taken
 * ──────────────────────────────────────────────────────────── */
static int port_is_available(int port) {
    /* Simulate port 8888 as "already in use" for the demo */
    return (port != 8888);
}

/* ─────────────────────────────────────────────────────────────
 *  tomcat_start() — tomcat.start()
 *  This is where the TCP socket actually binds.
 *  In real Tomcat: NioEndpoint.bind() → ServerSocketChannel.bind()
 * ──────────────────────────────────────────────────────────── */
static int tomcat_start(EmbeddedTomcat *t) {
    t->start_time = clock();

    printf("\n[Tomcat] Starting...\n");

    /* Step 1: Check port availability */
    printf("  [NioEndpoint] Binding to port %d...\n", t->connector.port);
    if (!port_is_available(t->connector.port)) {
        t->state = SERVER_FAILED;
        fprintf(stderr,
                "  [ERROR] Address already in use: port %d\n"
                "  → Spring Boot throws: PortInUseException\n"
                "  → PortInUseFailureAnalyzer produces friendly message\n",
                t->connector.port);
        return 0;  /* FAIL */
    }
    printf("  [NioEndpoint] Port %d bound successfully\n", t->connector.port);

    /* Step 2: Initialize thread pool */
    printf("  [Executor] Starting thread pool: min=%d max=%d\n",
           t->thread_pool.min_spare_threads, t->thread_pool.max_threads);
    t->thread_pool.active_threads = t->thread_pool.min_spare_threads;
    printf("  [Executor] %d idle threads ready\n",
           t->thread_pool.active_threads);

    /* Step 3: Initialize servlets (load-on-startup) */
    printf("  [Context] Initializing servlets...\n");
    for (int i = 0; i < t->context.servlet_count; i++) {
        ServletRegistration *sr = &t->context.servlets[i];
        if (sr->load_on_startup > 0) {
            printf("    [Servlet] %s.init() [load-on-startup=%d]\n",
                   sr->name, sr->load_on_startup);
        }
    }

    /* Step 4: Initialize filters */
    printf("  [Context] Initializing filters...\n");
    for (int i = 0; i < t->context.filter_count; i++) {
        printf("    [Filter] %s.init()\n", t->context.filters[i].name);
    }

    /* Step 5: Server is UP */
    t->state = SERVER_STARTED;
    t->startup_ms = (double)(clock() - t->start_time) / CLOCKS_PER_SEC * 1000.0;

    printf("\n[Tomcat] Started on port %d (%.2f ms)\n",
           t->connector.port, t->startup_ms);
    return 1;  /* SUCCESS */
}

/* ─────────────────────────────────────────────────────────────
 *  tomcat_stop() — graceful shutdown
 *  Java: TomcatWebServer.stop()
 *        → connector.pause()   (stop accepting new connections)
 *        → wait for in-flight requests to complete
 *        → connector.stop()    (close TCP socket)
 *        → context.destroy()   (destroy servlets)
 * ──────────────────────────────────────────────────────────── */
static void tomcat_stop(EmbeddedTomcat *t, int graceful_timeout_ms) {
    if (t->state != SERVER_STARTED) return;

    t->state = SERVER_STOPPING;
    printf("\n[Tomcat] Stopping (graceful timeout: %d ms)...\n",
           graceful_timeout_ms);

    /* Phase 1: Pause connector — stop accepting new connections */
    printf("  [Connector] Paused: no new connections accepted\n");

    /* Phase 2: Drain in-flight requests */
    if (t->in_flight_requests > 0) {
        printf("  [Drain] %d in-flight request(s) — waiting up to %d ms...\n",
               t->in_flight_requests, graceful_timeout_ms);
        /* Simulate completion */
        t->in_flight_requests = 0;
        printf("  [Drain] All in-flight requests completed\n");
    } else {
        printf("  [Drain] No in-flight requests\n");
    }

    /* Phase 3: Destroy servlets */
    printf("  [Context] Destroying servlets...\n");
    for (int i = t->context.servlet_count - 1; i >= 0; i--) {
        printf("    [Servlet] %s.destroy()\n", t->context.servlets[i].name);
    }

    /* Phase 4: Close TCP socket */
    printf("  [NioEndpoint] Closing socket on port %d\n", t->connector.port);

    t->state = SERVER_STOPPED;
    printf("[Tomcat] Stopped\n");
    printf("[Tomcat] Total requests served: %d\n",
           t->thread_pool.total_requests);
}

static void tomcat_destroy(EmbeddedTomcat *t) {
    free(t);
}

/* ─────────────────────────────────────────────────────────────
 *  Simulate processing an HTTP request
 * ──────────────────────────────────────────────────────────── */
static void tomcat_handle_request(EmbeddedTomcat *t,
                                   const char *method,
                                   const char *path)
{
    if (t->state != SERVER_STARTED) {
        printf("[Tomcat] Cannot handle request — server not running\n");
        return;
    }

    t->in_flight_requests++;
    t->thread_pool.active_threads++;
    t->thread_pool.total_requests++;

    printf("[Tomcat] %-7s %s%s\n",
           method, t->context.context_path, path);
    printf("         Thread pool: %d/%d active | queue: %d\n",
           t->thread_pool.active_threads,
           t->thread_pool.max_threads,
           t->thread_pool.queued_requests);

    t->in_flight_requests--;
    t->thread_pool.active_threads--;
}

/* ─────────────────────────────────────────────────────────────
 *  Print server summary  (like Spring Boot actuator /actuator/info)
 * ──────────────────────────────────────────────────────────── */
static void tomcat_print_status(const EmbeddedTomcat *t) {
    printf("\n━━━ Embedded Tomcat Status ━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  State:             %s\n", server_state_name(t->state));
    printf("  Port:              %d (%s)\n",
           t->connector.port,
           t->connector.is_secure ? "HTTPS" : "HTTP");
    printf("  Context path:      '%s'\n", t->context.context_path);
    printf("  Threads: active=%-4d max=%-4d min-spare=%d\n",
           t->thread_pool.active_threads,
           t->thread_pool.max_threads,
           t->thread_pool.min_spare_threads);
    printf("  Servlets:          %d registered\n", t->context.servlet_count);
    printf("  Filters:           %d registered\n", t->context.filter_count);
    printf("  Startup time:      %.2f ms\n", t->startup_ms);
    printf("  Total requests:    %d\n", t->thread_pool.total_requests);
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_normal_startup(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Normal Web Application Startup\n");
    printf("Simulates: TomcatServletWebServerFactory.getWebServer()\n");
    printf("           called during ServletWebServerApplicationContext.onRefresh()\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    EmbeddedTomcat *tomcat = tomcat_create();

    /* Configure from ServerProperties (application.yml parsed values) */
    tomcat_configure(tomcat, 8080, 200, 10, "/");

    /* Register DispatcherServlet (done by DispatcherServletAutoConfiguration) */
    printf("\n[Context] Registering Servlets and Filters:\n");
    tomcat_add_servlet(tomcat,
                       "dispatcherServlet",
                       "org.springframework.web.servlet.DispatcherServlet",
                       "/",
                       1);

    /* Register Security Filter (done by SpringSecurityFilterChainRegistrationBean) */
    tomcat_add_filter(tomcat,
                      "springSecurityFilterChain",
                      "org.springframework.security.web.FilterChainProxy",
                      1);

    /* Register Logging/Tracing Filter */
    tomcat_add_filter(tomcat,
                      "requestContextFilter",
                      "org.springframework.web.filter.RequestContextFilter",
                      -105);

    /* Register Character Encoding Filter */
    tomcat_add_filter(tomcat,
                      "characterEncodingFilter",
                      "org.springframework.boot.web.servlet.filter.OrderedCharacterEncodingFilter",
                      -2147483648);

    /* Start the server */
    if (tomcat_start(tomcat)) {
        tomcat_print_status(tomcat);

        /* Simulate some requests */
        printf("── Simulating HTTP requests ──────────────────────────\n");
        tomcat_handle_request(tomcat, "GET",    "/api/users");
        tomcat_handle_request(tomcat, "POST",   "/api/users");
        tomcat_handle_request(tomcat, "GET",    "/api/users/42");
        tomcat_handle_request(tomcat, "GET",    "/actuator/health");
        printf("\n");

        /* Graceful shutdown (SIGTERM received) */
        printf("── Graceful Shutdown (SIGTERM) ───────────────────────\n");
        tomcat->in_flight_requests = 2;  /* simulate 2 active requests at shutdown time */
        tomcat_stop(tomcat, 30000);      /* 30-second grace period */
    }

    tomcat_destroy(tomcat);
}

void demo_port_conflict(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Port Already in Use → PortInUseFailureAnalyzer\n");
    printf("Port 8888 is simulated as already in use\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    EmbeddedTomcat *tomcat = tomcat_create();
    tomcat_configure(tomcat, 8888, 200, 10, "/");  /* port 8888 = conflict */

    tomcat_add_servlet(tomcat, "dispatcherServlet",
                       "org.springframework.web.servlet.DispatcherServlet", "/", 1);

    int ok = tomcat_start(tomcat);

    if (!ok) {
        printf("\n  ***************************\n");
        printf("  APPLICATION FAILED TO START\n");
        printf("  ***************************\n\n");
        printf("  Description:\n");
        printf("  Web server failed to start. Port 8888 was already in use.\n\n");
        printf("  Action:\n");
        printf("  Identify and stop the process that is listening on port 8888\n");
        printf("  or configure this application to listen on another port.\n");
        printf("    server.port=<different-port>  in application.yml\n");
        printf("  or: java -jar app.jar --server.port=9090\n\n");
    }

    tomcat_destroy(tomcat);
}

void demo_https_config(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: HTTPS (SSL/TLS) Configuration\n");
    printf("application.yml: server.ssl.enabled=true\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    EmbeddedTomcat *tomcat = tomcat_create();
    tomcat_configure(tomcat, 8443, 200, 10, "/");

    /* Configure SSL */
    tomcat->connector.scheme     = CONNECTOR_HTTPS;
    tomcat->connector.is_secure  = 1;
    strncpy(tomcat->connector.key_store_path,
            "classpath:keystore.p12",
            sizeof(tomcat->connector.key_store_path) - 1);
    strncpy(tomcat->connector.key_store_password, "changeit",
            sizeof(tomcat->connector.key_store_password) - 1);
    strncpy(tomcat->connector.ssl_protocol, "TLSv1.3",
            sizeof(tomcat->connector.ssl_protocol) - 1);

    printf("SSL Connector configured:\n");
    printf("  scheme       = HTTPS\n");
    printf("  port         = %d\n",   tomcat->connector.port);
    printf("  key-store    = %s\n",   tomcat->connector.key_store_path);
    printf("  ssl-protocol = %s\n\n", tomcat->connector.ssl_protocol);

    tomcat_add_servlet(tomcat, "dispatcherServlet",
                       "org.springframework.web.servlet.DispatcherServlet", "/", 1);

    if (tomcat_start(tomcat)) {
        tomcat_print_status(tomcat);
        tomcat_stop(tomcat, 5000);
    }

    tomcat_destroy(tomcat);
}

void demo_thread_pool_exhaustion(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 4: Thread Pool Under Load\n");
    printf("Shows how Tomcat threads handle concurrent requests\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    EmbeddedTomcat *tomcat = tomcat_create();
    /* Small thread pool to show queuing behavior */
    tomcat_configure(tomcat, 8080, 5, 2, "/");
    tomcat->thread_pool.accept_count = 3;

    tomcat_add_servlet(tomcat, "dispatcherServlet",
                       "org.springframework.web.servlet.DispatcherServlet", "/", 1);

    if (!tomcat_start(tomcat)) { tomcat_destroy(tomcat); return; }

    printf("Simulating burst of 8 concurrent requests (max-threads=5, accept-count=3):\n\n");

    for (int i = 1; i <= 8; i++) {
        char path[32];
        snprintf(path, sizeof(path), "/api/order/%d", i);

        if (tomcat->thread_pool.active_threads < tomcat->thread_pool.max_threads) {
            /* Thread available — process immediately */
            tomcat_handle_request(tomcat, "POST", path);
        } else if (tomcat->thread_pool.queued_requests < tomcat->thread_pool.accept_count) {
            /* Queue has space */
            tomcat->thread_pool.queued_requests++;
            printf("[Tomcat] POST %-20s → QUEUED (queue=%d/%d)\n",
                   path,
                   tomcat->thread_pool.queued_requests,
                   tomcat->thread_pool.accept_count);
        } else {
            /* Queue full — reject with 503 */
            printf("[Tomcat] POST %-20s → REJECTED (503 Service Unavailable)\n", path);
            printf("         Threads: %d/%d  Queue: %d/%d FULL\n",
                   tomcat->thread_pool.max_threads,
                   tomcat->thread_pool.max_threads,
                   tomcat->thread_pool.accept_count,
                   tomcat->thread_pool.accept_count);
        }
    }

    printf("\n  Fix: increase server.tomcat.threads.max or server.tomcat.accept-count\n\n");

    tomcat_stop(tomcat, 5000);
    tomcat_destroy(tomcat);
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  embedded_tomcat.c — Spring Boot Embedded Tomcat    ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_normal_startup();
    demo_port_conflict();
    demo_https_config();
    demo_thread_pool_exhaustion();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Embedded Tomcat = Tomcat's API called from inside your JVM\n");
    printf("  No WAR, no external server — JAR contains everything\n");
    printf("  Lifecycle: NEW → INITIALIZED → STARTED → STOPPING → STOPPED\n");
    printf("  DispatcherServlet is registered as the single servlet at '/'\n");
    printf("  Port conflict → PortInUseException → FailureAnalyzer message\n");
    printf("  Thread pool: max-threads=200 by default (tune for your workload)\n");
    printf("  Graceful stop: pause connector → drain in-flight → destroy servlets\n");
    return 0;
}
