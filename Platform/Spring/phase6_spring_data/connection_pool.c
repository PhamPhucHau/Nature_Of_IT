/*
 * connection_pool.c  —  HikariCP Connection Pool in C
 *
 * Build:  gcc -Wall -Wextra -o connection_pool connection_pool.c
 * Run:    ./connection_pool
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   com.zaxxer.hikari.HikariDataSource (HikariCP)
 *   — the default connection pool in Spring Boot since version 2.0
 *
 *   A connection pool:
 *     1. At startup: opens N TCP connections to the database
 *     2. Keeps them open (warm connections, no handshake overhead)
 *     3. Each query: borrow a connection from pool
 *     4. After query: return connection to pool (DO NOT CLOSE)
 *     5. If pool is empty: wait up to connection-timeout, then throw
 *
 *   HikariCP is famous for:
 *     - Fastest pool in Java ecosystem
 *     - Minimal overhead per borrow/return
 *     - Byte-level connection validation
 *
 * SPRING CONFIGURATION:
 *   spring.datasource.hikari.maximum-pool-size=10
 *   spring.datasource.hikari.minimum-idle=5
 *   spring.datasource.hikari.connection-timeout=30000
 *   spring.datasource.hikari.idle-timeout=600000
 *   spring.datasource.hikari.max-lifetime=1800000
 *
 * SIMPLIFIED MODEL:
 *   We simulate connections as simple structs (not real TCP sockets).
 *   The pool logic (borrow/return/timeout) is fully functional.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════
 *  CONNECTION — simulates a JDBC Connection (= TCP socket to DB)
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    CONN_IDLE,        /* available in pool — ready to borrow       */
    CONN_IN_USE,      /* borrowed by a thread — executing a query  */
    CONN_CLOSED,      /* connection closed / invalidated           */
    CONN_VALIDATING,  /* pool is testing if connection is alive    */
} ConnectionState;

typedef struct {
    int             id;              /* connection number (1-based)      */
    ConnectionState state;           /* current state                    */
    char            db_url[256];     /* e.g. "jdbc:mysql://host:3306/db" */
    int             query_count;     /* how many queries ran on this conn */
    time_t          created_at;      /* when this connection was opened  */
    time_t          last_used_at;    /* for idle timeout tracking        */
    int             is_valid;        /* 1 = DB server still alive        */
} DbConnection;

/* ═══════════════════════════════════════════════════════════
 *  HIKARI POOL CONFIGURATION
 *  Java: HikariConfig / spring.datasource.hikari.*
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    char   jdbc_url[256];           /* spring.datasource.url              */
    char   username[64];            /* spring.datasource.username         */
    char   password[64];            /* spring.datasource.password (hidden)*/
    int    minimum_idle;            /* minimum-idle (default: same as max)*/
    int    maximum_pool_size;       /* maximum-pool-size (default: 10)    */
    int    connection_timeout_ms;   /* connection-timeout (default: 30000)*/
    int    idle_timeout_ms;         /* idle-timeout (default: 600000)     */
    int    max_lifetime_ms;         /* max-lifetime (default: 1800000)    */
    char   pool_name[64];           /* pool-name (for logging)            */
} HikariConfig;

/* ═══════════════════════════════════════════════════════════
 *  HIKARI CONNECTION POOL
 *  Java: HikariPool (internal class of HikariDataSource)
 * ═══════════════════════════════════════════════════════════ */
#define MAX_POOL_SIZE 32

typedef struct {
    HikariConfig  config;
    DbConnection  connections[MAX_POOL_SIZE];
    int           pool_size;        /* current total connections (idle + in-use) */
    int           idle_count;       /* currently available connections           */
    int           active_count;     /* currently borrowed connections            */
    int           total_borrows;    /* lifetime borrow count (for stats)         */
    int           total_timeouts;   /* times caller waited too long              */
} HikariPool;

/* ─────────────────────────────────────────────────────────────
 *  Simulate opening a TCP connection to the database
 *  Java: DriverManager.getConnection(url, user, pass)
 * ──────────────────────────────────────────────────────────── */
static DbConnection open_physical_connection(const HikariConfig *cfg, int conn_id) {
    DbConnection conn;
    memset(&conn, 0, sizeof(conn));

    conn.id         = conn_id;
    conn.state      = CONN_IDLE;
    conn.query_count = 0;
    conn.is_valid   = 1;
    conn.created_at = time(NULL);
    conn.last_used_at = time(NULL);
    strncpy(conn.db_url, cfg->jdbc_url, sizeof(conn.db_url) - 1);

    printf("  [HikariPool] Opening physical connection #%d → %s\n",
           conn_id, cfg->jdbc_url);

    return conn;
}

/* ─────────────────────────────────────────────────────────────
 *  hikari_pool_create()
 *  Java: new HikariDataSource(config) → creates pool, opens minimum-idle connections
 *
 *  HikariCP opens minimum-idle connections at startup.
 *  If minimum-idle < maximum-pool-size, more are opened on demand.
 * ──────────────────────────────────────────────────────────── */
static HikariPool* hikari_pool_create(const HikariConfig *cfg) {
    HikariPool *pool = calloc(1, sizeof(HikariPool));
    pool->config = *cfg;

    printf("[HikariPool] Initializing pool '%s'\n", cfg->pool_name);
    printf("  URL:              %s\n", cfg->jdbc_url);
    printf("  minimum-idle:     %d\n", cfg->minimum_idle);
    printf("  maximum-pool-size:%d\n", cfg->maximum_pool_size);
    printf("  connection-timeout: %d ms\n", cfg->connection_timeout_ms);
    printf("\n[HikariPool] Opening %d initial connections...\n", cfg->minimum_idle);

    for (int i = 0; i < cfg->minimum_idle; i++) {
        pool->connections[pool->pool_size] = open_physical_connection(cfg, i + 1);
        pool->pool_size++;
        pool->idle_count++;
    }

    printf("[HikariPool] Pool ready: %d idle connections\n\n", pool->idle_count);
    return pool;
}

/* ─────────────────────────────────────────────────────────────
 *  hikari_borrow()
 *  Java: DataSource.getConnection() — borrow a connection
 *
 *  HikariCP algorithm:
 *    1. Scan pool for IDLE connection → return immediately
 *    2. If none and pool_size < max → create new connection → return
 *    3. If none and pool_size == max → WAIT up to connection-timeout
 *    4. Still none → throw SQLTimeoutException
 *
 *  HikariCP uses a highly optimized lock-free ConcurrentBag internally.
 *  We use simple linear scan for clarity.
 * ──────────────────────────────────────────────────────────── */
static DbConnection* hikari_borrow(HikariPool *pool) {
    printf("[HikariPool] getConnection() — borrowing connection...\n");

    /* Step 1: Find an idle connection */
    for (int i = 0; i < pool->pool_size; i++) {
        if (pool->connections[i].state == CONN_IDLE &&
            pool->connections[i].is_valid)
        {
            pool->connections[i].state      = CONN_IN_USE;
            pool->connections[i].last_used_at = time(NULL);
            pool->idle_count--;
            pool->active_count++;
            pool->total_borrows++;

            printf("  [HikariPool] Borrowing connection #%d (idle→in-use)\n",
                   pool->connections[i].id);
            printf("  [HikariPool] Pool state: %d idle, %d active / %d max\n\n",
                   pool->idle_count, pool->active_count, pool->config.maximum_pool_size);
            return &pool->connections[i];
        }
    }

    /* Step 2: No idle connection — can we grow the pool? */
    if (pool->pool_size < pool->config.maximum_pool_size) {
        printf("  [HikariPool] No idle connections — growing pool (%d → %d)\n",
               pool->pool_size, pool->pool_size + 1);

        pool->connections[pool->pool_size] = open_physical_connection(
            &pool->config, pool->pool_size + 1);
        pool->connections[pool->pool_size].state = CONN_IN_USE;
        pool->pool_size++;
        pool->active_count++;
        pool->total_borrows++;

        printf("  [HikariPool] Pool state: %d idle, %d active / %d max\n\n",
               pool->idle_count, pool->active_count, pool->config.maximum_pool_size);
        return &pool->connections[pool->pool_size - 1];
    }

    /* Step 3: Pool full and no idle connections → TIMEOUT */
    pool->total_timeouts++;
    fprintf(stderr,
            "  [HikariPool] ERROR: Pool exhausted! All %d connections in use.\n"
            "               Waited %d ms → throwing SQLTimeoutException\n"
            "               → Spring throws: CannotGetJdbcConnectionException\n\n",
            pool->config.maximum_pool_size,
            pool->config.connection_timeout_ms);
    return NULL;
}

/* ─────────────────────────────────────────────────────────────
 *  hikari_return()
 *  Java: Connection.close()  ← despite the name, does NOT close!
 *        It returns the connection to the pool.
 *
 *  HikariCP wraps real JDBC Connection in a ProxyConnection.
 *  When you call connection.close(), it actually calls pool.recycle().
 *
 *  IMPORTANT LESSON:
 *    In Spring, NEVER close a Connection directly.
 *    Use JdbcTemplate or @Transactional — they manage return automatically.
 * ──────────────────────────────────────────────────────────── */
static void hikari_return(HikariPool *pool, DbConnection *conn) {
    printf("[HikariPool] connection.close() → returning #%d to pool\n", conn->id);

    /* Validate connection is still alive before returning
     * HikariCP uses: connectionTestQuery (SELECT 1) */
    if (!conn->is_valid) {
        printf("  [HikariPool] Connection #%d is invalid → discarding, opening new one\n",
               conn->id);
        *conn = open_physical_connection(&pool->config, conn->id);
    }

    conn->state = CONN_IDLE;
    pool->idle_count++;
    pool->active_count--;

    printf("  [HikariPool] Pool state: %d idle, %d active / %d max\n\n",
           pool->idle_count, pool->active_count, pool->config.maximum_pool_size);
}

/* ─────────────────────────────────────────────────────────────
 *  hikari_pool_stats()
 *  Java: HikariPoolMXBean — exposed via Spring Boot Actuator
 *        GET /actuator/metrics/hikaricp.connections
 * ──────────────────────────────────────────────────────────── */
static void hikari_pool_stats(const HikariPool *pool) {
    printf("━━━ HikariPool Stats ('%s') ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n",
           pool->config.pool_name);
    printf("  Total connections:     %d / %d max\n",
           pool->pool_size, pool->config.maximum_pool_size);
    printf("  Idle connections:      %d\n", pool->idle_count);
    printf("  Active connections:    %d\n", pool->active_count);
    printf("  Total borrows:         %d\n", pool->total_borrows);
    printf("  Total timeouts:        %d\n", pool->total_timeouts);

    printf("\n  Connection details:\n");
    for (int i = 0; i < pool->pool_size; i++) {
        const DbConnection *c = &pool->connections[i];
        const char *state_str[] = {"IDLE", "IN_USE", "CLOSED", "VALIDATING"};
        printf("    conn#%d: %-12s queries=%d valid=%s\n",
               c->id, state_str[c->state], c->query_count, c->is_valid ? "yes" : "no");
    }
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
 *  hikari_pool_destroy()
 *  Java: HikariDataSource.close() — called on application shutdown
 *  Closes all physical connections.
 * ──────────────────────────────────────────────────────────── */
static void hikari_pool_destroy(HikariPool *pool) {
    printf("[HikariPool] Shutting down pool '%s'...\n", pool->config.pool_name);
    for (int i = 0; i < pool->pool_size; i++) {
        if (pool->connections[i].state != CONN_CLOSED) {
            printf("  [HikariPool] Closing physical connection #%d\n",
                   pool->connections[i].id);
            pool->connections[i].state = CONN_CLOSED;
        }
    }
    printf("[HikariPool] All connections closed.\n\n");
    free(pool);
}

/* ═══════════════════════════════════════════════════════════
 *  SIMULATE JDBC QUERY EXECUTION
 *  Shows how JdbcTemplate uses the pool internally
 * ═══════════════════════════════════════════════════════════ */
static void simulate_query(HikariPool *pool, const char *sql, const char *desc) {
    printf("── [JdbcTemplate] Executing: %s ──\n", desc);
    printf("   SQL: %s\n", sql);

    DbConnection *conn = hikari_borrow(pool);
    if (!conn) {
        printf("   FAILED: no connection available\n\n");
        return;
    }

    /* Execute the "query" (simulated) */
    conn->query_count++;
    printf("   Executed on connection #%d (query #%d on this conn)\n\n",
           conn->id, conn->query_count);

    /* Return connection to pool */
    hikari_return(pool, conn);
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_basic_pool(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Basic Connection Pool Lifecycle\n");
    printf("Spring Boot: spring.datasource.hikari.maximum-pool-size=3\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    HikariConfig cfg = {
        .jdbc_url             = "jdbc:mysql://localhost:3306/mydb",
        .username             = "app_user",
        .password             = "***hidden***",
        .minimum_idle         = 2,
        .maximum_pool_size    = 3,
        .connection_timeout_ms = 30000,
        .idle_timeout_ms      = 600000,
        .max_lifetime_ms      = 1800000,
        .pool_name            = "HikariPool-1"
    };

    HikariPool *pool = hikari_pool_create(&cfg);

    /* Execute several queries — borrowing and returning connections */
    simulate_query(pool, "SELECT * FROM users WHERE id = 1", "findById(1)");
    simulate_query(pool, "SELECT COUNT(*) FROM users",        "count()");
    simulate_query(pool, "INSERT INTO orders VALUES (...)",   "createOrder()");
    simulate_query(pool, "UPDATE users SET name = ? WHERE id = ?", "updateUser()");

    hikari_pool_stats(pool);
    hikari_pool_destroy(pool);
}

void demo_pool_exhaustion(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Pool Exhaustion (SQLTimeoutException)\n");
    printf("Borrow all connections, then try one more → timeout\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    HikariConfig cfg = {
        .jdbc_url              = "jdbc:postgresql://localhost:5432/orders",
        .minimum_idle          = 1,
        .maximum_pool_size     = 2,   /* tiny pool for demo */
        .connection_timeout_ms = 5000,
        .pool_name             = "HikariPool-small"
    };

    HikariPool *pool = hikari_pool_create(&cfg);

    /* Borrow all connections and hold them (don't return) */
    printf("── Borrowing all %d connections and holding them ────────\n",
           cfg.maximum_pool_size);
    DbConnection *c1 = hikari_borrow(pool);
    DbConnection *c2 = hikari_borrow(pool);

    printf("── Now trying to borrow one more (pool is full) ─────────\n");
    DbConnection *c3 = hikari_borrow(pool);   /* This should TIMEOUT */

    /* Return held connections */
    if (c1) hikari_return(pool, c1);
    if (c2) hikari_return(pool, c2);
    if (c3) hikari_return(pool, c3);

    hikari_pool_stats(pool);
    hikari_pool_destroy(pool);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  connection_pool.c — HikariCP Connection Pool in C  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_basic_pool();
    demo_pool_exhaustion();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Connection pool = pre-opened DB sockets kept warm\n");
    printf("  getConnection() = borrow from pool (NOT new TCP handshake)\n");
    printf("  connection.close() = RETURN to pool (NOT actual close)\n");
    printf("  Pool exhaustion → SQLTimeoutException after connection-timeout\n");
    printf("  JdbcTemplate manages borrow/return automatically per query\n");
    printf("  @Transactional holds one connection for entire method duration\n");
    return 0;
}
