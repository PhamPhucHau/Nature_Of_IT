/*
 * dispatcher_servlet.c  —  Spring MVC DispatcherServlet in C
 *
 * Build:  gcc -Wall -Wextra -o dispatcher_servlet dispatcher_servlet.c
 * Run:    ./dispatcher_servlet
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.web.servlet.DispatcherServlet
 *
 *   The DispatcherServlet is Spring MVC's Front Controller.
 *   EVERY HTTP request flows through it.
 *
 *   Spring Java call chain:
 *     DispatcherServlet.service()
 *       → doService()
 *         → doDispatch()
 *           → getHandler()         ← HandlerMapping lookup
 *           → getHandlerAdapter()  ← find right adapter
 *           → applyPreHandle()     ← run interceptors (before)
 *           → ha.handle()          ← invoke @Controller method
 *           → applyPostHandle()    ← run interceptors (after)
 *           → processDispatchResult()
 *             → render()           ← serialize response
 *
 * ARCHITECTURE:
 *   This file implements the full pipeline in ~300 lines of C.
 *   Real Spring has ~10,000 lines across many files, but the
 *   LOGIC is identical — just more pluggable and generalized.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  HTTP METHOD ENUM
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_PATCH,
    HTTP_UNKNOWN
} HttpMethod;

static HttpMethod parse_method(const char *s) {
    if (strcmp(s, "GET")    == 0) return HTTP_GET;
    if (strcmp(s, "POST")   == 0) return HTTP_POST;
    if (strcmp(s, "PUT")    == 0) return HTTP_PUT;
    if (strcmp(s, "DELETE") == 0) return HTTP_DELETE;
    if (strcmp(s, "PATCH")  == 0) return HTTP_PATCH;
    return HTTP_UNKNOWN;
}

static const char* method_name(HttpMethod m) {
    static const char* names[] = {"GET","POST","PUT","DELETE","PATCH","UNKNOWN"};
    return names[(int)m < 6 ? (int)m : 5];
}

/* ═══════════════════════════════════════════════════════════
 *  HTTP REQUEST
 *  Simulates javax.servlet.http.HttpServletRequest
 * ═══════════════════════════════════════════════════════════ */
#define MAX_PARAMS 16

typedef struct {
    char key[64];
    char value[256];
} QueryParam;

typedef struct {
    HttpMethod  method;
    char        path[256];         /* e.g. "/api/users/42"   */
    char        body[1024];        /* raw request body       */
    char        content_type[64];  /* Content-Type header    */
    char        auth_header[256];  /* Authorization header   */
    QueryParam  params[MAX_PARAMS];/* query string params    */
    int         param_count;
} HttpRequest;

/* ═══════════════════════════════════════════════════════════
 *  HTTP RESPONSE
 *  Simulates javax.servlet.http.HttpServletResponse
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    int  status;           /* HTTP status code (200, 404, 500...) */
    char body[4096];       /* response body                       */
    char content_type[64]; /* response Content-Type               */
    int  committed;        /* 1 = already sent (can't modify)     */
} HttpResponse;

static void response_write(HttpResponse *res, int status, const char *body, const char *ct) {
    res->status = status;
    strncpy(res->body, body, sizeof(res->body) - 1);
    strncpy(res->content_type, ct, sizeof(res->content_type) - 1);
}

static void response_print(const HttpResponse *res) {
    printf("  ┌─ HTTP Response ───────────────────────────────────\n");
    printf("  │  Status: %d\n", res->status);
    printf("  │  Content-Type: %s\n", res->content_type);
    printf("  │  Body: %s\n", res->body);
    printf("  └───────────────────────────────────────────────────\n");
}

/* ═══════════════════════════════════════════════════════════
 *  HANDLER (controller method descriptor)
 *
 *  Java equivalent:
 *    HandlerMethod — wraps a Method + controller bean
 *    Holds: method ref, controller instance, parameter info
 * ═══════════════════════════════════════════════════════════ */

/* Handler function signature: takes request, fills response */
typedef void (*HandlerFn)(const HttpRequest*, HttpResponse*, const char* path_var);

typedef struct {
    HttpMethod  method;                  /* GET, POST, etc.              */
    char        path_pattern[256];       /* e.g. "/api/users/{id}"       */
    HandlerFn   handler;                 /* the @Controller method       */
    char        controller_name[64];     /* "UserController" for logging */
    char        method_name[64];         /* "getUserById" for logging    */
} HandlerMapping;

/* ═══════════════════════════════════════════════════════════
 *  INTERCEPTOR (HandlerInterceptor)
 *  Java: org.springframework.web.servlet.HandlerInterceptor
 * ═══════════════════════════════════════════════════════════ */
typedef int  (*PreHandleFn) (const HttpRequest*, HttpResponse*);   /* return 0 to abort */
typedef void (*PostHandleFn)(const HttpRequest*, HttpResponse*);

typedef struct {
    const char   *name;
    PreHandleFn   pre_handle;
    PostHandleFn  post_handle;
} Interceptor;


/* ═══════════════════════════════════════════════════════════
 *  SAMPLE CONTROLLERS — simulating @RestController classes
 * ═══════════════════════════════════════════════════════════ */

/* ── UserController ──
 * Java:
 *   @RestController @RequestMapping("/api")
 *   class UserController {
 *     @GetMapping("/users")
 *     List<User> getUsers() { ... }
 *
 *     @GetMapping("/users/{id}")
 *     User getUserById(@PathVariable Long id) { ... }
 *
 *     @PostMapping("/users")
 *     User createUser(@RequestBody CreateUserRequest req) { ... }
 *
 *     @DeleteMapping("/users/{id}")
 *     void deleteUser(@PathVariable Long id) { ... }
 *   }
 */

void handler_get_users(const HttpRequest *req, HttpResponse *res, const char *pv) {
    (void)pv;
    /* Simulate @RequestParam parsing */
    const char *page = "1";
    for (int i = 0; i < req->param_count; i++) {
        if (strcmp(req->params[i].key, "page") == 0) {
            page = req->params[i].value;
        }
    }
    char body[512];
    snprintf(body, sizeof(body),
             "{\"users\":[{\"id\":1,\"name\":\"Alice\"},{\"id\":2,\"name\":\"Bob\"}]"
             ",\"page\":%s,\"total\":2}", page);
    response_write(res, 200, body, "application/json");
}

void handler_get_user_by_id(const HttpRequest *req, HttpResponse *res, const char *pv) {
    (void)req;
    if (!pv || pv[0] == '\0') {
        response_write(res, 400, "{\"error\":\"Missing path variable 'id'\"}", "application/json");
        return;
    }
    char body[256];
    snprintf(body, sizeof(body),
             "{\"id\":%s,\"name\":\"Alice\",\"email\":\"alice@example.com\"}", pv);
    response_write(res, 200, body, "application/json");
}

void handler_create_user(const HttpRequest *req, HttpResponse *res, const char *pv) {
    (void)pv;
    if (req->body[0] == '\0') {
        response_write(res, 400, "{\"error\":\"Request body is required\"}", "application/json");
        return;
    }
    response_write(res, 201,
                   "{\"id\":3,\"name\":\"Charlie\",\"message\":\"User created\"}",
                   "application/json");
}

void handler_delete_user(const HttpRequest *req, HttpResponse *res, const char *pv) {
    (void)req;
    if (!pv || pv[0] == '\0') {
        response_write(res, 400, "{\"error\":\"Missing user id\"}", "application/json");
        return;
    }
    char body[128];
    snprintf(body, sizeof(body), "{\"message\":\"User %s deleted\"}", pv);
    response_write(res, 200, body, "application/json");
}

void handler_health_check(const HttpRequest *req, HttpResponse *res, const char *pv) {
    (void)req; (void)pv;
    response_write(res, 200,
                   "{\"status\":\"UP\",\"service\":\"user-service\"}",
                   "application/json");
}

void handler_not_found(const HttpRequest *req, HttpResponse *res, const char *pv) {
    (void)pv;
    char body[256];
    snprintf(body, sizeof(body),
             "{\"error\":\"No handler found for %s %s\"}",
             method_name(req->method), req->path);
    response_write(res, 404, body, "application/json");
}

/* ═══════════════════════════════════════════════════════════
 *  SAMPLE INTERCEPTORS
 * ═══════════════════════════════════════════════════════════ */

int logging_pre_handle(const HttpRequest *req, HttpResponse *res) {
    (void)res;
    printf("  [LoggingInterceptor.preHandle]  → %s %s\n",
           method_name(req->method), req->path);
    return 1;  /* continue processing */
}

void logging_post_handle(const HttpRequest *req, HttpResponse *res) {
    printf("  [LoggingInterceptor.postHandle] ← %d\n", res->status);
    (void)req;
}

int auth_pre_handle(const HttpRequest *req, HttpResponse *res) {
    /* Simple auth: require "Authorization: Bearer token" header */
    if (req->auth_header[0] == '\0') {
        printf("  [AuthInterceptor.preHandle] No auth header → 401\n");
        response_write(res, 401,
                       "{\"error\":\"Unauthorized — missing Authorization header\"}",
                       "application/json");
        return 0;  /* ABORT — stop processing */
    }
    if (strncmp(req->auth_header, "Bearer ", 7) != 0) {
        printf("  [AuthInterceptor.preHandle] Invalid token format → 401\n");
        response_write(res, 401,
                       "{\"error\":\"Unauthorized — invalid token format\"}",
                       "application/json");
        return 0;  /* ABORT */
    }
    printf("  [AuthInterceptor.preHandle] Auth OK (token: %s)\n",
           req->auth_header + 7);
    return 1;  /* continue */
}

void auth_post_handle(const HttpRequest *req, HttpResponse *res) {
    (void)req; (void)res;
    /* nothing needed after auth */
}

/* ═══════════════════════════════════════════════════════════
 *  HANDLER MAPPING TABLE
 *  Built at startup from @RequestMapping annotations.
 *  Java: RequestMappingHandlerMapping.detectHandlerMethods()
 * ═══════════════════════════════════════════════════════════ */
static HandlerMapping handler_table[] = {
    {HTTP_GET,    "/api/users",       handler_get_users,      "UserController", "getUsers"},
    {HTTP_GET,    "/api/users/{id}",  handler_get_user_by_id, "UserController", "getUserById"},
    {HTTP_POST,   "/api/users",       handler_create_user,    "UserController", "createUser"},
    {HTTP_DELETE, "/api/users/{id}",  handler_delete_user,    "UserController", "deleteUser"},
    {HTTP_GET,    "/actuator/health", handler_health_check,   "ActuatorController", "health"},
};
static int handler_count = (int)(sizeof(handler_table) / sizeof(handler_table[0]));

/* ═══════════════════════════════════════════════════════════
 *  INTERCEPTOR CHAIN (order matters!)
 *  Java: MappedInterceptor list in DispatcherServlet
 * ═══════════════════════════════════════════════════════════ */
static Interceptor interceptors[] = {
    {"LoggingInterceptor", logging_pre_handle, logging_post_handle},
    {"AuthInterceptor",    auth_pre_handle,    auth_post_handle},
};
static int interceptor_count = (int)(sizeof(interceptors) / sizeof(interceptors[0]));

/* ═══════════════════════════════════════════════════════════
 *  PATH MATCHING — simple pattern matcher
 *  Supports: exact paths and {variable} patterns
 *
 *  Java: AntPathMatcher.match(pattern, path)
 *  e.g. "/api/users/{id}" matches "/api/users/42"
 *       and extracts pathVariable = "42"
 * ═══════════════════════════════════════════════════════════ */
static int path_matches(const char *pattern, const char *path, char *path_var, int pv_size) {
    if (path_var) path_var[0] = '\0';

    /* Find '{' in pattern — indicates path variable */
    const char *brace = strchr(pattern, '{');
    if (!brace) {
        /* exact match */
        return strcmp(pattern, path) == 0;
    }

    /* Match the prefix before '{' */
    int prefix_len = (int)(brace - pattern);
    if (strncmp(pattern, path, prefix_len) != 0) {
        return 0;
    }

    /* Extract the path variable from the remaining path */
    const char *var_start = path + prefix_len;
    const char *slash     = strchr(var_start, '/');
    int var_len;
    if (slash) {
        var_len = (int)(slash - var_start);
    } else {
        var_len = (int)strlen(var_start);
    }

    if (path_var && var_len > 0 && var_len < pv_size) {
        strncpy(path_var, var_start, var_len);
        path_var[var_len] = '\0';
    }
    return 1;
}

/* ═══════════════════════════════════════════════════════════
 *  DISPATCHER SERVLET — the core routing engine
 *
 *  Java: DispatcherServlet.doDispatch()
 *  This is THE method that processes every HTTP request.
 * ═══════════════════════════════════════════════════════════ */
static void do_dispatch(const HttpRequest *req, HttpResponse *res) {
    char path_var[128] = {0};

    printf("\n┌── DispatcherServlet.doDispatch() ──────────────────────\n");
    printf("│  %s %s\n", method_name(req->method), req->path);
    printf("├────────────────────────────────────────────────────────\n");

    /* ── Step 1: Run interceptor PRE-HANDLE chain ──
     * Java: applyPreHandle() — iterates HandlerInterceptor.preHandle()
     * If any interceptor returns false: ABORT, send response immediately */
    printf("│  [1] Running interceptor preHandle chain (%d interceptors)\n",
           interceptor_count);

    for (int i = 0; i < interceptor_count; i++) {
        int cont = interceptors[i].pre_handle(req, res);
        if (!cont) {
            printf("│  [1] Interceptor '%s' aborted request → returning early\n",
                   interceptors[i].name);
            printf("└─────────────────────────────────────────────────────────\n");
            return;   /* response already set by interceptor */
        }
    }

    /* ── Step 2: HandlerMapping — find the handler ──
     * Java: getHandler() → tries each HandlerMapping in order
     *       Returns HandlerExecutionChain (handler + interceptors)
     *
     * We scan our route table for a match. */
    printf("│  [2] HandlerMapping: finding handler for '%s %s'\n",
           method_name(req->method), req->path);

    HandlerMapping *matched = NULL;
    for (int i = 0; i < handler_count; i++) {
        if (handler_table[i].method != req->method) continue;
        if (path_matches(handler_table[i].path_pattern, req->path, path_var, sizeof(path_var))) {
            matched = &handler_table[i];
            break;
        }
    }

    if (!matched) {
        printf("│  [2] No handler found → 404\n");
        handler_not_found(req, res, NULL);
        printf("└─────────────────────────────────────────────────────────\n");
        return;
    }

    printf("│  [2] Handler found: %s.%s()\n",
           matched->controller_name, matched->method_name);
    if (path_var[0]) {
        printf("│      PathVariable extracted: '%s'\n", path_var);
    }

    /* ── Step 3: HandlerAdapter — invoke the handler ──
     * Java: HandlerAdapter.handle()
     *       RequestMappingHandlerAdapter handles @Controller methods
     *       It reads @RequestParam, @PathVariable, @RequestBody
     *       then invokes method via Reflection */
    printf("│  [3] HandlerAdapter: invoking %s.%s()\n",
           matched->controller_name, matched->method_name);

    matched->handler(req, res, path_var);

    printf("│  [3] Handler returned: status=%d\n", res->status);

    /* ── Step 4: Run interceptor POST-HANDLE chain ──
     * Java: applyPostHandle() — reverse order of preHandle */
    printf("│  [4] Running interceptor postHandle chain\n");
    for (int i = interceptor_count - 1; i >= 0; i--) {
        interceptors[i].post_handle(req, res);
    }

    /* ── Step 5: Message conversion / view resolution ──
     * Java: processDispatchResult() → render() → HttpMessageConverter.write()
     *       Converts return value to JSON/XML bytes
     *       Already done in our handler functions (for simplicity) */
    printf("│  [5] HttpMessageConverter: serialized to %s\n", res->content_type);

    printf("└─────────────────────────────────────────────────────────\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Helper: build a simple request
 * ──────────────────────────────────────────────────────────── */
static HttpRequest make_request(const char *method_str, const char *path,
                                 const char *body, const char *auth) {
    HttpRequest req;
    memset(&req, 0, sizeof(req));
    req.method = parse_method(method_str);
    strncpy(req.path, path, sizeof(req.path) - 1);
    if (body) strncpy(req.body, body, sizeof(req.body) - 1);
    if (auth) strncpy(req.auth_header, auth, sizeof(req.auth_header) - 1);
    strncpy(req.content_type, "application/json", sizeof(req.content_type) - 1);
    return req;
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS — simulating real HTTP requests
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  dispatcher_servlet.c — Spring MVC Pipeline in C    ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    HttpResponse res;

    /* ── Request 1: GET /api/users — requires auth ── */
    memset(&res, 0, sizeof(res));
    HttpRequest r1 = make_request("GET", "/api/users", NULL, "Bearer eyJhbGci.eyJ1c2Vy.sig");
    do_dispatch(&r1, &res);
    response_print(&res);

    /* ── Request 2: GET /api/users/42 — path variable ── */
    memset(&res, 0, sizeof(res));
    HttpRequest r2 = make_request("GET", "/api/users/42", NULL, "Bearer valid-token-xyz");
    do_dispatch(&r2, &res);
    response_print(&res);

    /* ── Request 3: POST /api/users — request body ── */
    memset(&res, 0, sizeof(res));
    HttpRequest r3 = make_request("POST", "/api/users",
                                   "{\"name\":\"Charlie\",\"email\":\"charlie@example.com\"}",
                                   "Bearer admin-token");
    do_dispatch(&r3, &res);
    response_print(&res);

    /* ── Request 4: No auth header → interceptor blocks ── */
    memset(&res, 0, sizeof(res));
    HttpRequest r4 = make_request("GET", "/api/users", NULL, NULL);
    do_dispatch(&r4, &res);
    response_print(&res);

    /* ── Request 5: 404 — no handler matched ── */
    memset(&res, 0, sizeof(res));
    HttpRequest r5 = make_request("GET", "/api/products", NULL, "Bearer token");
    do_dispatch(&r5, &res);
    response_print(&res);

    /* ── Request 6: DELETE /api/users/99 ── */
    memset(&res, 0, sizeof(res));
    HttpRequest r6 = make_request("DELETE", "/api/users/99", NULL, "Bearer admin-token");
    do_dispatch(&r6, &res);
    response_print(&res);

    /* ── Request 7: Actuator health (public endpoint) ── */
    memset(&res, 0, sizeof(res));
    HttpRequest r7 = make_request("GET", "/actuator/health", NULL, "Bearer any-token");
    do_dispatch(&r7, &res);
    response_print(&res);

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  DispatcherServlet = Front Controller: routes ALL requests\n");
    printf("  HandlerMapping    = route table: (method+path) → handler fn\n");
    printf("  Interceptors      = preHandle chain (auth, logging, CORS)\n");
    printf("  HandlerAdapter    = extracts params + invokes handler method\n");
    printf("  MessageConverter  = serializes return value to JSON/XML\n");
    printf("  Path variables    = extracted from URL pattern matching\n");
    return 0;
}
