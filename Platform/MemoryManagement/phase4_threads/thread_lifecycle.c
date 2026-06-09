/*
 * thread_lifecycle.c  —  Java Thread Lifecycle in C
 *
 * Build:  gcc -Wall -Wextra -pthread -o thread_lifecycle thread_lifecycle.c
 * Run:    ./thread_lifecycle
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   java.lang.Thread lifecycle states:
 *     NEW → RUNNABLE → TIMED_WAITING/WAITING/BLOCKED → RUNNABLE → TERMINATED
 *
 *   And ThreadLocal<T>:
 *     Each thread gets its own isolated copy of a variable.
 *     Spring uses ThreadLocal for:
 *       - SecurityContextHolder (current authenticated user)
 *       - TransactionSynchronizationManager (current DB connection)
 *       - RequestContextHolder (current HTTP request attributes)
 *
 *   PTHREADS → JAVA MAPPING:
 *     pthread_create()      → new Thread(runnable).start()
 *     pthread_join()        → thread.join()
 *     pthread_cancel()      → thread.interrupt() (cooperative)
 *     nanosleep()           → Thread.sleep()
 *     pthread_cond_wait()   → object.wait()
 *     pthread_cond_signal() → object.notify()
 *     pthread_key_create()  → new ThreadLocal<T>()
 *     pthread_setspecific() → threadLocal.set(value)
 *     pthread_getspecific() → threadLocal.get()
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>     /* usleep() */
#include <stdatomic.h>

/* ═══════════════════════════════════════════════════════════
 *  THREAD STATE MACHINE
 *  Java: Thread.State enum
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    T_NEW,            /* Thread object created, .start() not called */
    T_RUNNABLE,       /* .start() called, OS thread running or ready */
    T_TIMED_WAITING,  /* Thread.sleep(n), wait(n), parkNanos()        */
    T_WAITING,        /* wait(), join(), park() with no timeout        */
    T_BLOCKED,        /* Waiting to enter synchronized block           */
    T_TERMINATED,     /* run() returned or threw uncaught exception    */
} ThreadState;

static const char* state_name(ThreadState s) {
    static const char *names[] = {
        "NEW", "RUNNABLE", "TIMED_WAITING", "WAITING", "BLOCKED", "TERMINATED"
    };
    return names[(int)s < 6 ? (int)s : 5];
}

/* ═══════════════════════════════════════════════════════════
 *  JAVA THREAD SIMULATION
 * ═══════════════════════════════════════════════════════════ */
#define MAX_THREADS 8

typedef struct JavaThread {
    int         tid;             /* thread ID                             */
    char        name[64];        /* thread name (Thread.setName())        */
    ThreadState state;           /* current state                         */
    pthread_t   os_thread;       /* underlying OS thread (1:1 mapping)    */
    int         daemon;          /* daemon thread? JVM exits when only    */
                                 /* daemon threads remain                  */
    int         priority;        /* 1-10, maps to OS priority hint        */
    atomic_int  interrupted;     /* interrupt flag (thread.interrupt())   */
    int         join_timeout_ms; /* for timed join                        */
} JavaThread;

/* ─────────────────────────────────────────────────────────────
 *  Thread state transition logging
 * ──────────────────────────────────────────────────────────── */
static void thread_transition(JavaThread *jt, ThreadState from, ThreadState to,
                               const char *reason) {
    (void)from;
    jt->state = to;
    printf("  [%-20s] %-14s → %-14s  (%s)\n",
           jt->name, state_name(from), state_name(to), reason);
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 1: Basic Thread Lifecycle
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    JavaThread *jt;
    int         work_units;
} WorkArgs;

static void* worker_run(void *arg) {
    WorkArgs *wa = (WorkArgs*)arg;
    JavaThread *jt = wa->jt;

    /* NEW → RUNNABLE transition happened in thread_start() */

    printf("  [%-20s] Running on OS thread %lu\n",
           jt->name, (unsigned long)pthread_self());

    for (int i = 0; i < wa->work_units; i++) {
        /* Check interrupt flag (cooperative cancellation) */
        if (atomic_load(&jt->interrupted)) {
            printf("  [%-20s] InterruptedException — stopping early\n", jt->name);
            thread_transition(jt, T_RUNNABLE, T_TERMINATED, "InterruptedException");
            return NULL;
        }

        printf("  [%-20s] Processing unit %d/%d\n", jt->name, i+1, wa->work_units);

        /* Simulate Thread.sleep(100ms) */
        thread_transition(jt, T_RUNNABLE, T_TIMED_WAITING, "Thread.sleep(100)");
        usleep(50000);  /* 50ms */
        thread_transition(jt, T_TIMED_WAITING, T_RUNNABLE, "sleep elapsed");
    }

    thread_transition(jt, T_RUNNABLE, T_TERMINATED, "run() completed normally");
    return NULL;
}

static void thread_start(JavaThread *jt, WorkArgs *args) {
    thread_transition(jt, T_NEW, T_RUNNABLE, "start() called → OS thread created");
    pthread_create(&jt->os_thread, NULL, worker_run, args);
}

static void thread_join(JavaThread *jt) {
    printf("  [main] Waiting for '%s' (thread.join())...\n", jt->name);
    thread_transition(jt, T_RUNNABLE, T_WAITING, "another thread called join()");
    pthread_join(jt->os_thread, NULL);
    /* Main thread returns from WAITING when target terminates */
    printf("  [main] '%s' joined\n\n", jt->name);
}

void demo_thread_lifecycle(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Thread Lifecycle — NEW→RUNNABLE→WAITING→TERMINATED\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    JavaThread t1 = {
        .tid = 1, .name = "WorkerThread-1",
        .state = T_NEW, .daemon = 0, .priority = 5
    };
    atomic_init(&t1.interrupted, 0);

    WorkArgs args1 = { .jt = &t1, .work_units = 3 };

    printf("  Creating thread in NEW state\n");
    printf("  Java: Thread t = new Thread(runnable);  // state=NEW\n\n");

    thread_start(&t1, &args1);
    thread_join(&t1);

    printf("  Thread state: %s\n\n", state_name(t1.state));
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 2: Thread Interruption
 *  Java: thread.interrupt() → sets interrupt flag
 *        Thread.sleep() throws InterruptedException when interrupted
 * ═══════════════════════════════════════════════════════════ */

void demo_thread_interrupt(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Thread.interrupt() — Cooperative Cancellation\n");
    printf("Java: thread.interrupt(); // sets interrupt flag\n");
    printf("      Thread.sleep() throws InterruptedException\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    JavaThread t2 = {
        .tid = 2, .name = "LongWorker",
        .state = T_NEW, .daemon = 0, .priority = 5
    };
    atomic_init(&t2.interrupted, 0);

    WorkArgs args2 = { .jt = &t2, .work_units = 100 };  /* would run forever */

    thread_start(&t2, &args2);

    /* Let it run briefly, then interrupt */
    usleep(120000);  /* 120ms */
    printf("  [main] Calling thread.interrupt() on '%s'\n", t2.name);
    atomic_store(&t2.interrupted, 1);   /* Java: t2.interrupt() */

    thread_join(&t2);

    printf("  LESSON: Java threads should check isInterrupted() in loops.\n");
    printf("          Thread.sleep() auto-checks and throws InterruptedException.\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 3: ThreadLocal<T>
 *  Java: ThreadLocal stores separate value per thread.
 *        Spring Security: SecurityContextHolder.getContext()
 *        Spring TX:       TransactionSynchronizationManager
 * ═══════════════════════════════════════════════════════════ */

/* ThreadLocal key — created once, used by ALL threads */
static pthread_key_t current_user_key;  /* Java: ThreadLocal<User> */
static pthread_key_t request_id_key;    /* Java: ThreadLocal<String> requestId */

typedef struct {
    int  user_id;
    char username[32];
    char role[16];
} User;

/* Destructor: called when thread exits — equivalent to ThreadLocal.remove() */
static void user_destructor(void *data) {
    if (data) {
        User *user = (User*)data;
        printf("  [ThreadLocal] Cleaning up User{id=%d, %s} for thread %lu\n",
               user->user_id, user->username,
               (unsigned long)pthread_self());
        free(data);
    }
}

static void request_id_destructor(void *data) {
    free(data);  /* free the request ID string */
}

typedef struct {
    int  user_id;
    char username[32];
    char role[16];
    char request_id[64];
    int  iterations;
} RequestArgs;

static void* handle_request(void *arg) {
    RequestArgs *ra = (RequestArgs*)arg;

    /* Simulate: SecurityContextHolder.setAuthentication(auth)
     * Each thread stores its OWN user in the thread-local */
    User *user = calloc(1, sizeof(User));
    user->user_id = ra->user_id;
    strncpy(user->username, ra->username, 31);
    strncpy(user->role,     ra->role,     15);

    pthread_setspecific(current_user_key, user);  /* Java: threadLocal.set(user) */

    char *req_id = strdup(ra->request_id);
    pthread_setspecific(request_id_key, req_id);

    /* Simulate multiple method calls in the request pipeline */
    /* Each can access the current user via ThreadLocal */
    for (int i = 0; i < ra->iterations; i++) {
        usleep(10000);  /* 10ms per "layer" */

        /* Java: User u = SecurityContextHolder.getContext().getAuthentication(); */
        User *current = (User*)pthread_getspecific(current_user_key);
        char *rid     = (char*)pthread_getspecific(request_id_key);

        if (i == 0) {
            printf("  [Thread %lu] req=%s user=%s/%s accessing layer %d\n",
                   (unsigned long)pthread_self(),
                   rid, current->username, current->role, i+1);
        }
    }

    User *verify = (User*)pthread_getspecific(current_user_key);
    printf("  [Thread %lu] Request complete. Final user: %s (role=%s)\n",
           (unsigned long)pthread_self(),
           verify->username, verify->role);

    /* ThreadLocal.remove() — happens automatically via destructor when thread exits */
    /* But for pooled threads, we MUST call it manually: */
    /* pthread_setspecific(current_user_key, NULL); */
    /* In Spring: SecurityContextHolder.clearContext() at end of request */

    return NULL;
}

void demo_thread_local(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: ThreadLocal<T> — Per-Thread State Isolation\n");
    printf("Java: ThreadLocal<User> currentUser = new ThreadLocal<>();\n");
    printf("Spring: SecurityContextHolder, TransactionSynchronizationManager\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Create ThreadLocal keys (equivalent to new ThreadLocal<>()) */
    pthread_key_create(&current_user_key, user_destructor);
    pthread_key_create(&request_id_key,   request_id_destructor);

    /* Simulate 3 concurrent HTTP requests, each with different users */
    RequestArgs req_args[] = {
        { 1, "alice",   "ADMIN",  "req-001", 3 },
        { 2, "bob",     "USER",   "req-002", 3 },
        { 3, "charlie", "VIEWER", "req-003", 3 },
    };

    pthread_t threads[3];
    printf("  3 concurrent HTTP requests, each with different authenticated users:\n\n");

    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, handle_request, &req_args[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_key_delete(current_user_key);
    pthread_key_delete(request_id_key);

    printf("\n  LESSON: Each thread has its OWN user — no sharing, no synchronization needed.\n");
    printf("  Spring: SecurityContextHolder.getContext() returns THIS thread's security context.\n");
    printf("  DANGER: ThreadLocal leaks in thread pools → always call remove() after use.\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 4: Daemon Thread
 *  Java: t.setDaemon(true) → JVM exits when only daemon threads remain
 *        Background tasks (GC, finalizer thread) are daemons
 * ═══════════════════════════════════════════════════════════ */

void demo_daemon_thread(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 4: Daemon Threads\n");
    printf("Java: thread.setDaemon(true)\n");
    printf("      JVM exits when no non-daemon threads remain\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    printf("  Thread types:\n");
    printf("  Non-daemon (user thread):\n");
    printf("    - main thread\n");
    printf("    - application threads (Tomcat request threads)\n");
    printf("    - @Async task threads\n");
    printf("    JVM WAITS for these to complete before exiting.\n\n");

    printf("  Daemon threads:\n");
    printf("    - GC threads\n");
    printf("    - JIT compiler threads\n");
    printf("    - Background monitoring threads\n");
    printf("    JVM does NOT wait — they are killed when main exits.\n\n");

    printf("  Spring Boot: sets shutdown hooks to drain non-daemon threads.\n");
    printf("  This is how graceful shutdown works:\n");
    printf("    1. SIGTERM received\n");
    printf("    2. Stop accepting new requests\n");
    printf("    3. Wait for non-daemon request threads to finish\n");
    printf("    4. JVM exits normally\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  thread_lifecycle.c — Java Threads in C             ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_thread_lifecycle();
    demo_thread_interrupt();
    demo_thread_local();
    demo_daemon_thread();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Java Thread state machine: NEW→RUNNABLE→{WAITING,BLOCKED}→TERMINATED\n");
    printf("  pthread_create() = thread.start()   pthread_join() = thread.join()\n");
    printf("  interrupt() = cooperative: sets flag, sleeping threads throw InterruptedException\n");
    printf("  ThreadLocal = per-thread isolated storage (no synchronization needed)\n");
    printf("  Spring: SecurityContext, TX connection bind via ThreadLocal\n");
    printf("  Always call ThreadLocal.remove() in pooled threads → prevent leaks\n");
    return 0;
}
