/*
 * proxy_chain.c  —  Spring AOP Proxy & Interceptor Chain in C
 *
 * Build:  gcc -Wall -Wextra -o proxy_chain proxy_chain.c
 * Run:    ./proxy_chain
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   Spring AOP — specifically:
 *     org.springframework.aop.framework.JdkDynamicAopProxy.invoke()
 *     org.springframework.aop.framework.ReflectiveMethodInvocation.proceed()
 *
 *   When Spring creates an AOP-proxied bean:
 *     1. Scans @Aspect classes at startup
 *     2. Builds a list of MethodInterceptors for each method
 *     3. Creates a proxy (CGLIB subclass or JDK proxy)
 *     4. Proxy's method calls proceed() through the interceptor chain
 *     5. At the end of the chain: the REAL method is called
 *
 *   The chain is recursive/iterative:
 *     Interceptor1 → Interceptor2 → Interceptor3 → real method
 *     (like nested function calls or Russian dolls)
 *
 *   Java @Around with ProceedingJoinPoint:
 *     Object around(ProceedingJoinPoint pjp) {
 *         doBeforeStuff();
 *         Object result = pjp.proceed();   ← calls next in chain
 *         doAfterStuff();
 *         return result;
 *     }
 *
 * REAL-WORLD SPRING USES:
 *   @Transactional     → TransactionInterceptor (begin/commit/rollback)
 *   @Cacheable         → CacheInterceptor (check/put cache)
 *   @Async             → AsyncExecutionInterceptor (run in thread pool)
 *   @PreAuthorize      → MethodSecurityInterceptor (check permissions)
 *   @Retryable         → RetryOperationsInterceptor (retry on failure)
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════
 *  INVOCATION CONTEXT
 *  Java: MethodInvocation / ProceedingJoinPoint
 *
 *  Holds everything about the current method invocation:
 *  - the target object
 *  - the method arguments
 *  - the return value
 *  - the interceptor chain state (which interceptor is next)
 * ═══════════════════════════════════════════════════════════ */
#define MAX_ARG_LEN   256
#define MAX_CHAIN_LEN 8

struct Invocation;  /* forward declaration */
typedef int (*InterceptorFn)(struct Invocation*);   /* 1 = continue, 0 = abort */

typedef struct Invocation {
    /* Method identity */
    const char *class_name;     /* "UserService"             */
    const char *method_name;    /* "getUserById"             */

    /* Arguments */
    char        arg[MAX_ARG_LEN];  /* simplified: one string arg */

    /* Return value (set by target method or @Around) */
    char        return_value[MAX_ARG_LEN];
    int         has_exception;     /* 1 if method threw         */
    char        exception_msg[128];

    /* Interceptor chain state */
    InterceptorFn chain[MAX_CHAIN_LEN];  /* ordered list of interceptors */
    int           chain_length;
    int           chain_index;           /* current position in chain    */

    /* Target (the real method — last in chain) */
    InterceptorFn target_fn;

    /* Timing (for @Around performance logging) */
    clock_t  start_time;
} Invocation;

/*
 * proceed()
 * ─────────────────────────────────────────────────────────────
 * Java: ProceedingJoinPoint.proceed()
 *
 * This is THE key method in AOP.
 * It calls the NEXT interceptor in the chain.
 * When all interceptors have run, it calls the REAL method.
 *
 * The beauty: each interceptor can run code BEFORE and AFTER
 * calling proceed() — wrapping the entire downstream chain.
 *
 * Recursive model (actual Spring ReflectiveMethodInvocation):
 *   interceptor[0].invoke(mi)
 *     → mi.proceed()
 *       → interceptor[1].invoke(mi)
 *         → mi.proceed()
 *           → interceptor[2].invoke(mi)
 *             → mi.proceed()
 *               → target method (real code)
 *             ← return from target
 *           ← interceptor[2] post logic
 *         ← return
 *       ← interceptor[1] post logic
 *     ← return
 */
static int proceed(Invocation *mi) {
    if (mi->chain_index < mi->chain_length) {
        /* Call next interceptor */
        InterceptorFn next = mi->chain[mi->chain_index++];
        return next(mi);
    } else {
        /* All interceptors exhausted — call the real target method */
        return mi->target_fn(mi);
    }
}

/* ═══════════════════════════════════════════════════════════
 *  INTERCEPTORS (AOP Advice implementations)
 *
 *  Each interceptor simulates a real Spring @Aspect.
 *  The pattern:
 *    1. Before logic
 *    2. call proceed(mi)    ← call next in chain
 *    3. After logic
 * ═══════════════════════════════════════════════════════════ */

/* ── 1. Logging Interceptor ──
 * Java: @Around with method name + duration logging
 * Activated by: @Slf4j + custom @LogExecutionTime or Spring Boot Actuator
 */
static int logging_interceptor(Invocation *mi) {
    mi->start_time = clock();

    printf("  [LoggingAspect.@Around BEFORE] %s.%s(arg='%s')\n",
           mi->class_name, mi->method_name, mi->arg);

    /* ← proceed: runs all remaining interceptors + target */
    int result = proceed(mi);

    double elapsed_ms = (double)(clock() - mi->start_time) / CLOCKS_PER_SEC * 1000.0;

    if (mi->has_exception) {
        printf("  [LoggingAspect.@Around AFTER]  %s.%s() FAILED: %s (%.2f ms)\n",
               mi->class_name, mi->method_name, mi->exception_msg, elapsed_ms);
    } else {
        printf("  [LoggingAspect.@Around AFTER]  %s.%s() returned '%s' in %.2f ms\n",
               mi->class_name, mi->method_name, mi->return_value, elapsed_ms);
    }

    return result;
}

/* ── 2. Security Interceptor ──
 * Java: @PreAuthorize("hasRole('ADMIN')")
 * Activated by: Spring Security @EnableMethodSecurity
 */
static int security_interceptor(Invocation *mi) {
    printf("  [SecurityAspect.@Before] Checking authorization for %s.%s()\n",
           mi->class_name, mi->method_name);

    /* Simulate: allow all for demo (real: check SecurityContext) */
    int authorized = 1;

    if (!authorized) {
        mi->has_exception = 1;
        strncpy(mi->exception_msg, "AccessDeniedException: insufficient privileges",
                sizeof(mi->exception_msg) - 1);
        printf("  [SecurityAspect] DENIED — not authorized\n");
        return 0;  /* do NOT call proceed() — abort chain */
    }

    printf("  [SecurityAspect.@Before] Authorization granted\n");
    int result = proceed(mi);  /* ← continue chain */

    printf("  [SecurityAspect.@After]  Audit log: %s.%s() completed by user\n",
           mi->class_name, mi->method_name);

    return result;
}

/* ── 3. Cache Interceptor ──
 * Java: @Cacheable("users") — check cache first, populate if miss
 * Activated by: @EnableCaching + CacheManager
 */
static int cache_interceptor(Invocation *mi) {
    char cache_key[128];
    snprintf(cache_key, sizeof(cache_key), "users::%s", mi->arg);

    printf("  [CacheAspect.@Around] Checking cache key: '%s'\n", cache_key);

    /* Simulate cache hit for id=1, miss for everything else */
    int cache_hit = (strcmp(mi->arg, "1") == 0);

    if (cache_hit) {
        strncpy(mi->return_value,
                "{\"id\":1,\"name\":\"Alice\",\"source\":\"CACHE\"}",
                MAX_ARG_LEN - 1);
        printf("  [CacheAspect.@Around] CACHE HIT — returning cached value\n");
        return 1;  /* do NOT call proceed() — short-circuit */
    }

    printf("  [CacheAspect.@Around] CACHE MISS — calling target method\n");
    int result = proceed(mi);  /* ← call real method */

    /* Store result in cache */
    if (!mi->has_exception) {
        printf("  [CacheAspect.@Around] Storing result in cache: key='%s'\n", cache_key);
    }

    return result;
}

/* ── 4. Transaction Interceptor ──
 * Java: @Transactional — begin transaction before, commit/rollback after
 * Activated by: @EnableTransactionManagement
 */
static int transaction_interceptor(Invocation *mi) {
    printf("  [TransactionAspect.@Around] BEGIN TRANSACTION\n");

    int result = proceed(mi);  /* ← call next in chain (or target) */

    if (mi->has_exception) {
        printf("  [TransactionAspect.@Around] ROLLBACK TRANSACTION — exception: %s\n",
               mi->exception_msg);
    } else {
        printf("  [TransactionAspect.@Around] COMMIT TRANSACTION\n");
    }

    return result;
}

/* ═══════════════════════════════════════════════════════════
 *  TARGET METHODS — the REAL @Service implementations
 *  These are called LAST in the chain.
 * ═══════════════════════════════════════════════════════════ */

/* Simulates: UserService.getUserById(Long id) */
static int target_get_user_by_id(Invocation *mi) {
    printf("  [UserService.getUserById] REAL METHOD executing with arg='%s'\n",
           mi->arg);

    /* Simulate: user not found for id=999 */
    if (strcmp(mi->arg, "999") == 0) {
        mi->has_exception = 1;
        strncpy(mi->exception_msg,
                "UserNotFoundException: user with id=999 not found",
                sizeof(mi->exception_msg) - 1);
        printf("  [UserService.getUserById] Throwing UserNotFoundException\n");
        return 0;
    }

    snprintf(mi->return_value, MAX_ARG_LEN,
             "{\"id\":%s,\"name\":\"Alice\",\"email\":\"alice@example.com\",\"source\":\"DB\"}",
             mi->arg);
    printf("  [UserService.getUserById] Returning user from database\n");
    return 1;
}

static int target_create_user(Invocation *mi) {
    printf("  [UserService.createUser] REAL METHOD executing with body='%s'\n",
           mi->arg);
    strncpy(mi->return_value,
            "{\"id\":42,\"message\":\"User created successfully\"}",
            MAX_ARG_LEN - 1);
    return 1;
}

/* ═══════════════════════════════════════════════════════════
 *  PROXY FACTORY — builds the interceptor chain
 *  Java: ProxyFactory.getProxy() / DefaultAopProxyFactory
 * ═══════════════════════════════════════════════════════════ */

static Invocation build_invocation(const char *class_name,
                                    const char *method_name,
                                    const char *arg,
                                    InterceptorFn target)
{
    Invocation mi;
    memset(&mi, 0, sizeof(mi));
    mi.class_name  = class_name;
    mi.method_name = method_name;
    strncpy(mi.arg, arg, MAX_ARG_LEN - 1);
    mi.target_fn = target;
    return mi;
}

/* Add interceptor to chain (order matters — first added = outermost wrapper) */
static void add_interceptor(Invocation *mi, InterceptorFn fn) {
    if (mi->chain_length < MAX_CHAIN_LEN) {
        mi->chain[mi->chain_length++] = fn;
    }
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_full_aop_chain(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Full AOP Chain — getUserById(id=2)\n");
    printf("Java annotations on the method:\n");
    printf("  @Transactional @Cacheable(\"users\") @PreAuthorize(\"isAuthenticated()\")\n");
    printf("  public User getUserById(Long id) { ... }\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    Invocation mi = build_invocation("UserService", "getUserById", "2", target_get_user_by_id);

    /* Build the interceptor chain in order:
     * Spring builds this from @Aspect classes sorted by @Order
     * Outermost interceptor is added first (will run first + last) */
    add_interceptor(&mi, logging_interceptor);      /* @Order(1) — outermost  */
    add_interceptor(&mi, security_interceptor);     /* @Order(2)              */
    add_interceptor(&mi, cache_interceptor);        /* @Order(3)              */
    add_interceptor(&mi, transaction_interceptor);  /* @Order(4) — innermost  */

    printf("Interceptor chain: Logging → Security → Cache → Transaction → target\n\n");

    /* This single call drives the entire chain */
    proceed(&mi);

    printf("\n  ✓ Final return value: %s\n\n", mi.return_value);
}

void demo_cache_hit(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: @Cacheable Cache HIT — getUserById(id=1)\n");
    printf("Cache hit: target method is NEVER called\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    Invocation mi = build_invocation("UserService", "getUserById", "1", target_get_user_by_id);
    add_interceptor(&mi, logging_interceptor);
    add_interceptor(&mi, cache_interceptor);
    add_interceptor(&mi, transaction_interceptor);

    proceed(&mi);

    printf("\n  ✓ Final return value: %s\n\n", mi.return_value);
}

void demo_exception_propagation(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: Exception propagation — getUserById(id=999)\n");
    printf("User not found → exception propagates through chain\n");
    printf("→ TransactionInterceptor sees exception → ROLLBACK\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    Invocation mi = build_invocation("UserService", "getUserById", "999", target_get_user_by_id);
    add_interceptor(&mi, logging_interceptor);
    add_interceptor(&mi, cache_interceptor);
    add_interceptor(&mi, transaction_interceptor);

    proceed(&mi);

    if (mi.has_exception) {
        printf("\n  ✗ Exception: %s\n\n", mi.exception_msg);
    }
}

void demo_write_method_no_cache(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 4: Write method (no cache) — createUser()\n");
    printf("Java: @Transactional (no @Cacheable on writes)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    Invocation mi = build_invocation("UserService", "createUser",
                                      "{\"name\":\"Dave\"}", target_create_user);
    add_interceptor(&mi, logging_interceptor);
    add_interceptor(&mi, transaction_interceptor);

    proceed(&mi);

    printf("\n  ✓ Final return value: %s\n\n", mi.return_value);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  proxy_chain.c — Spring AOP Proxy Chain in C        ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_full_aop_chain();
    demo_cache_hit();
    demo_exception_propagation();
    demo_write_method_no_cache();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  AOP Proxy = function pointer chain that wraps target method\n");
    printf("  proceed() = 'call the next interceptor in chain'\n");
    printf("  Each interceptor: do before → proceed() → do after\n");
    printf("  Cache interceptor can SHORT-CIRCUIT (skip proceed)\n");
    printf("  Transaction interceptor: begin before proceed, commit/rollback after\n");
    printf("  Exception propagates up through all interceptors in reverse\n");
    return 0;
}
