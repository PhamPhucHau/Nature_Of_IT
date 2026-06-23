# Detailed Architecture Patterns & Implementation
## For Each Scaling Stage

---

## PATTERN 1: Single Instance (Stage 1)

### When to Use
- Traffic: 0-50 TPS
- Team size: 1-2 engineers
- Budget: < $150K/year
- Timeline: Up to 12 months

### For Smartifi

**Current State:**
```
ATM Network
    ↓
[smartifi process]
    ├─ Main loop (select()-based)
    ├─ NAPAS connection (1)
    └─ Database connection (1)
         ↓
    [Oracle Database]
```

**Configuration:**
```ini
# smartifi.ini
PORT=9001
NAPAS_HOST=napas.bank.com
NAPAS_PORT=4444
DB_HOST=localhost
DB_USER=smartifi
DB_PASSWORD=***
CONNECTION_TIMEOUT=30
TRANSACTION_TIMEOUT=500
MAX_BUFFER=65536
```

**Database Setup:**
```sql
-- Initial schema
CREATE TABLESPACE smartifi_tbs 
  DATAFILE '/data/smartifi.dbf' SIZE 10G;

CREATE TABLE transactions (
  transaction_id VARCHAR2(20),
  amount NUMBER(12,2),
  status VARCHAR2(20),
  timestamp DATE,
  PRIMARY KEY(transaction_id)
);

CREATE INDEX idx_status ON transactions(status);
```

**Startup Script:**
```bash
#!/bin/bash
# startup_smartifi.sh

# Check prerequisites
if [ ! -f "/data/smartifi" ]; then
  echo "Binary not found"
  exit 1
fi

# Clean up old processes
pkill -f smartifi

# Wait for cleanup
sleep 2

# Start process
nohup /data/smartifi > /var/log/smartifi.log 2>&1 &

# Verify startup
sleep 3
if pgrep smartifi > /dev/null; then
  echo "Smartifi started successfully (PID: $!)"
else
  echo "Failed to start Smartifi"
  exit 1
fi
```

**Monitoring (Basic):**
```bash
#!/bin/bash
# monitor_smartifi.sh

while true; do
  echo "=== $(date) ==="
  
  # Process status
  ps aux | grep smartifi | grep -v grep
  
  # Memory usage
  ps -p $(pgrep smartifi) -o %mem=
  
  # CPU usage
  ps -p $(pgrep smartifi) -o %cpu=
  
  # Network connections
  netstat -an | grep smartifi | wc -l
  
  # Database connections
  sqlplus -s <<EOF
  SET PAGESIZE 0 FEEDBACK OFF VERIFY OFF HEADING OFF ECHO OFF
  SELECT COUNT(*) FROM v\$session WHERE username = 'SMARTIFI';
  EXIT;
EOF
  
  sleep 60
done
```

### For ACQ

**Current State:**
```
ATM Clients
    ↓
[ACQ Master Process]
    ├─ Accept connections
    ├─ Fork workers (2-5 initial)
    └─ Shared memory segment
         ↓
    [Worker 1] [Worker 2] ... [Worker N]
    Each handles one transaction
         ↓
    [Oracle Database]
```

**Configuration:**
```ini
# acq.ini
PORT=9002
MAX_WORKERS=5
MIN_WORKERS=2
WORKER_TIMEOUT=30
DB_HOST=localhost
DB_USER=acq
CONNECTION_POOL_SIZE=10
SHM_SIZE=16777216  # 16MB
```

**Shared Memory Setup:**
```c
// atmifp.c - Master process initialization
#define SHM_KEY 0x12345
#define SHM_SIZE (16 * 1024 * 1024) // 16MB

int initSharedMemory() {
    int shmid = shmget(SHM_KEY, SHM_SIZE, 
                      IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return -1;
    }
    
    void *shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat");
        return -1;
    }
    
    // Initialize semaphores for locking
    int semid = semget(SHM_KEY+1, 1, IPC_CREAT | 0666);
    
    return shmid;
}
```

---

## PATTERN 2: Vertical Scaling (Stage 2)

### Optimization Techniques

**1. Kernel Parameter Tuning**

```bash
# /etc/sysctl.conf
# TCP Stack Optimization
net.core.somaxconn = 4096
net.core.netdev_max_backlog = 5000
net.ipv4.tcp_max_syn_backlog = 4096
net.ipv4.tcp_fin_timeout = 30
net.ipv4.tcp_tw_reuse = 1

# Connection tracking
net.netfilter.nf_conntrack_max = 2000000
net.netfilter.nf_conntrack_tcp_timeout_established = 600

# Network buffers
net.core.rmem_max = 134217728
net.core.wmem_max = 134217728
net.ipv4.tcp_rmem = 4096 87380 67108864
net.ipv4.tcp_wmem = 4096 65536 67108864

# File descriptors
fs.file-max = 2097152
```

**2. Application Connection Pooling**

```c
// For Smartifi - Connection Pool Implementation
#define MAX_CONNECTIONS 30
#define MIN_CONNECTIONS 10

typedef struct {
    OCIEnv *envhp;
    OCIError *errhp;
    OCISvcCtx *svchp;
    time_t last_used;
    int in_use;
} DBConnection;

typedef struct {
    DBConnection pool[MAX_CONNECTIONS];
    int available_count;
    pthread_mutex_t lock;
} ConnectionPool;

DBConnection* getConnection(ConnectionPool *pool) {
    pthread_mutex_lock(&pool->lock);
    
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (!pool->pool[i].in_use) {
            pool->pool[i].in_use = 1;
            pool->available_count--;
            pthread_mutex_unlock(&pool->lock);
            return &pool->pool[i];
        }
    }
    
    pthread_mutex_unlock(&pool->lock);
    return NULL; // All connections in use
}

void releaseConnection(ConnectionPool *pool, 
                      DBConnection *conn) {
    pthread_mutex_lock(&pool->lock);
    conn->in_use = 0;
    conn->last_used = time(NULL);
    pool->available_count++;
    pthread_mutex_unlock(&pool->lock);
}
```

**3. Query Optimization**

```sql
-- Before: Sequential table scan
SELECT transaction_id, amount, status
FROM transactions
WHERE status = 'PENDING'
ORDER BY timestamp DESC;

-- After: Indexed access
CREATE INDEX idx_status_timestamp 
ON transactions(status, timestamp DESC);

-- With partition pruning
CREATE TABLE transactions_2024
PARTITION BY RANGE (MONTH(timestamp)) (
    PARTITION p1 VALUES LESS THAN (2),
    PARTITION p2 VALUES LESS THAN (3),
    -- ... etc
);
```

**4. Caching Layer (Redis-like)**

```c
// Cache implementation for Smartifi
#define CACHE_TTL 300 // 5 minutes

typedef struct {
    char key[256];
    char value[4096];
    time_t expiry;
} CacheEntry;

typedef struct {
    CacheEntry entries[10000];
    int count;
    pthread_rwlock_t lock;
} Cache;

int cacheGet(Cache *cache, const char *key, 
             char *value) {
    pthread_rwlock_rdlock(&cache->lock);
    
    for (int i = 0; i < cache->count; i++) {
        if (strcmp(cache->entries[i].key, key) == 0) {
            if (time(NULL) < cache->entries[i].expiry) {
                strcpy(value, cache->entries[i].value);
                pthread_rwlock_unlock(&cache->lock);
                return 1; // Cache hit
            }
        }
    }
    
    pthread_rwlock_unlock(&cache->lock);
    return 0; // Cache miss
}

void cacheSet(Cache *cache, const char *key, 
              const char *value) {
    pthread_rwlock_wrlock(&cache->lock);
    
    if (cache->count < 10000) {
        strcpy(cache->entries[cache->count].key, key);
        strcpy(cache->entries[cache->count].value, value);
        cache->entries[cache->count].expiry = 
            time(NULL) + CACHE_TTL;
        cache->count++;
    }
    
    pthread_rwlock_unlock(&cache->lock);
}
```

**Performance Impact:**
- Query optimization: +30% throughput
- Connection pooling: +25% throughput
- Caching layer: +40% throughput
- Total: ~95-100% increase in throughput (Stage 1 → Stage 2)

---

## PATTERN 3: Horizontal Scaling with Load Balancer (Stage 3)

### HAProxy Configuration

```conf
# /etc/haproxy/haproxy.cfg

global
    maxconn 100000
    log 127.0.0.1 local0
    log 127.0.0.1 local1 notice
    chroot /var/lib/haproxy
    stats socket /run/haproxy/admin.sock mode 660
    stats timeout 30s
    daemon

defaults
    log     global
    mode    tcp
    option  tcplog
    timeout connect 5000
    timeout client 30000
    timeout server 30000
    option  http-server-close
    retry-after 60

frontend smartifi_frontend
    bind *:9001
    mode tcp
    default_backend smartifi_backend
    
    # DDoS protection
    stick-table type ip size 100k expire 30m
    tcp-request connection track-sc0 src
    tcp-request connection reject if { 
        src_conn_cur gt 100 
    }

backend smartifi_backend
    mode tcp
    balance roundrobin
    option tcplog
    
    # Health checks
    option tcp-check
    tcp-check connect port 9001
    
    server smartifi-01 10.0.1.10:9001 
        maxconn 1000 check port 9001 
        inter 5000 rise 2 fall 3
    server smartifi-02 10.0.1.11:9001 
        maxconn 1000 check port 9001 
        inter 5000 rise 2 fall 3
    server smartifi-03 10.0.1.12:9001 
        maxconn 1000 check port 9001 
        inter 5000 rise 2 fall 3
    server smartifi-04 10.0.1.13:9001 
        maxconn 1000 check port 9001 
        inter 5000 rise 2 fall 3

listen stats
    bind *:8404
    stats enable
    stats uri /stats
    stats refresh 30s
```

### Redis Cluster Setup

```bash
#!/bin/bash
# setup_redis_cluster.sh

# Create Redis instances on different ports
for port in 6379 6380 6381; do
    # Create directory
    mkdir -p /data/redis-$port
    
    # Create configuration
    cat > /data/redis-$port/redis.conf << EOF
port $port
dir /data/redis-$port
dbfilename dump-$port.rdb
appendonly yes
cluster-enabled yes
cluster-config-file nodes-$port.conf
cluster-node-timeout 5000
EOF
    
    # Start Redis
    redis-server /data/redis-$port/redis.conf &
done

sleep 2

# Create cluster
redis-cli --cluster create \
  127.0.0.1:6379 \
  127.0.0.1:6380 \
  127.0.0.1:6381 \
  --cluster-replicas 0
```

### ZooKeeper Configuration

```xml
<!-- /etc/zookeeper/zoo.cfg -->
tickTime=2000
dataDir=/var/lib/zookeeper
clientPort=2181
initLimit=5
syncLimit=2

server.1=zk-node-1:2888:3888
server.2=zk-node-2:2888:3888
server.3=zk-node-3:2888:3888
```

### Session State Synchronization

```c
// Smartifi with session state management

// Store session in Redis
void saveSessionState(const char *user_id, 
                      const char *state_json) {
    redis_command(redis_ctx,
        "SET session:%s %s EX 3600",
        user_id, state_json);
}

// Retrieve session from any instance
char* getSessionState(const char *user_id) {
    char result[4096];
    redis_command(redis_ctx,
        "GET session:%s", user_id);
    return redis_read_reply();
}

// Distributed transaction lock
int acquireTransactionLock(const char *txn_id) {
    int result = redis_command(redis_ctx,
        "SET txn:lock:%s 1 EX 30 NX", txn_id);
    return result == 1; // 1 = got lock, 0 = already locked
}

void releaseTransactionLock(const char *txn_id) {
    redis_command(redis_ctx,
        "DEL txn:lock:%s", txn_id);
}
```

**Deployment Topology:**
```
                    HAProxy (10.0.1.1:9001)
                           ↓
    ┌──────────────────────┼──────────────────────┐
    ↓                      ↓                      ↓
Instance-1            Instance-2            Instance-3
(10.0.1.10)          (10.0.1.11)           (10.0.1.12)
  ↓                     ↓                     ↓
  └─────────────────────┴─────────────────────┘
              Redis Cluster
          (10.0.2.10-12:6379)
              ZooKeeper
          (10.0.3.10-12:2181)
              Shared DB
          (10.0.4.10:1521)
```

---

## PATTERN 4: Distributed Multi-Region (Stage 4)

### Multi-Master Replication Setup

```sql
-- Primary DC (Region 1)
CREATE DATABASE LINK region2 CONNECT TO 
  'acq' IDENTIFIED BY 'password'
  USING 'region2.bank.com:1521/acq';

-- Bidirectional replication
BEGIN
  DBMS_REPCAT.CREATE_MASTER_REPGROUP(
    gname => 'acq_repgroup',
    master_comment => 'ACQ Replication Group',
    rep_type => 'SYNCHRONOUS',
    propagation_delay => 0,
    execution => DBMS_REPCAT.IMMEDIATE
  );
END;
/

-- Add table to replication
EXEC DBMS_REPCAT.CREATE_MVIEW_REPOBJECT(
  gname => 'acq_repgroup',
  sname => 'SYS',
  oname => 'TRANSACTIONS',
  type => 'TABLE',
  ogroup => 'acq_table_group'
);
```

### Consensus Algorithm (Raft)

```c
// Simplified Raft implementation for distributed consensus

#define RAFT_STATE_FOLLOWER 0
#define RAFT_STATE_CANDIDATE 1
#define RAFT_STATE_LEADER 2

typedef struct {
    int current_term;
    int voted_for;
    int state; // follower, candidate, or leader
    int commit_index;
    int last_applied;
    
    // Leader info
    int next_index[3]; // for each follower
    int match_index[3];
    
    // Log entries
    LogEntry log[10000];
    int log_length;
} RaftNode;

// Election timeout handler
void onElectionTimeout(RaftNode *node) {
    if (node->state != RAFT_STATE_LEADER) {
        // Become candidate
        node->state = RAFT_STATE_CANDIDATE;
        node->current_term++;
        node->voted_for = NODE_ID;
        
        // Send vote requests to all peers
        for (int i = 0; i < PEER_COUNT; i++) {
            sendVoteRequest(peers[i], node->current_term,
                          NODE_ID, node->log_length,
                          node->log[node->log_length-1].term);
        }
    }
}

// Heartbeat handler (leader only)
void sendHeartbeat(RaftNode *node) {
    if (node->state == RAFT_STATE_LEADER) {
        for (int i = 0; i < PEER_COUNT; i++) {
            sendAppendEntriesRPC(peers[i],
                node->current_term,
                NODE_ID,
                node->next_index[i] - 1,
                node->log[node->next_index[i]-1].term,
                NULL, // no new entries in heartbeat
                node->commit_index);
        }
    }
}
```

### Global Load Balancer (GeoDNS)

```c
// DNS response based on geographic location

typedef struct {
    char region[50];
    char ip_address[20];
    int latitude;
    int longitude;
} RegionEndpoint;

RegionEndpoint endpoints[] = {
    {"us-east", "52.1.1.1", 40, -74},    // New York
    {"eu-west", "54.1.1.1", 51, 0},      // London
    {"ap-south", "1.1.1.1", 19, 72},     // Mumbai
};

const char* getClosestEndpoint(int client_lat, 
                               int client_lon) {
    int min_distance = INT_MAX;
    const char *closest = NULL;
    
    for (int i = 0; i < 3; i++) {
        int dist = abs(endpoints[i].latitude - client_lat) +
                   abs(endpoints[i].longitude - client_lon);
        
        if (dist < min_distance) {
            min_distance = dist;
            closest = endpoints[i].ip_address;
        }
    }
    
    return closest;
}
```

---

## PATTERN 5: Global Multi-Cloud (Stage 5)

### Service Mesh Configuration (Istio)

```yaml
# istio-virtualservice.yaml
apiVersion: networking.istio.io/v1beta1
kind: VirtualService
metadata:
  name: smartifi-global
  namespace: production
spec:
  hosts:
  - smartifi.bank.com
  http:
  - match:
    - sourceLabels:
        region: us-east
    route:
    - destination:
        host: smartifi-us-east
        port:
          number: 9001
        weight: 100
    timeout: 10s
    retries:
      attempts: 3
      perTryTimeout: 2s
  - match:
    - sourceLabels:
        region: eu-west
    route:
    - destination:
        host: smartifi-eu-west
        port:
          number: 9001
        weight: 100
  - match:
    - sourceLabels:
        region: ap-south
    route:
    - destination:
        host: smartifi-ap-south
        port:
          number: 9001
        weight: 100
  - route:
    - destination:
        host: smartifi-us-east
        port:
          number: 9001
        weight: 33
      - destination:
        host: smartifi-eu-west
        port:
          number: 9001
        weight: 33
      - destination:
        host: smartifi-ap-south
        port:
          number: 9001
        weight: 34
```

### Kubernetes Deployment

```yaml
# deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: smartifi-global
spec:
  replicas: 50
  strategy:
    type: RollingUpdate
    rollingUpdate:
      maxSurge: 5
      maxUnavailable: 2
  selector:
    matchLabels:
      app: smartifi
  template:
    metadata:
      labels:
        app: smartifi
        version: v1
    spec:
      affinity:
        podAntiAffinity:
          preferredDuringSchedulingIgnoredDuringExecution:
          - weight: 100
            podAffinityTerm:
              labelSelector:
                matchExpressions:
                - key: app
                  operator: In
                  values:
                  - smartifi
              topologyKey: kubernetes.io/hostname
      containers:
      - name: smartifi
        image: bank/smartifi:v1
        ports:
        - containerPort: 9001
        env:
        - name: REPLICA_ID
          valueFrom:
            fieldRef:
              fieldPath: metadata.name
        - name: DB_URL
          valueFrom:
            secretKeyRef:
              name: db-credentials
              key: url
        resources:
          requests:
            memory: "4Gi"
            cpu: "2000m"
          limits:
            memory: "8Gi"
            cpu: "4000m"
        livenessProbe:
          tcpSocket:
            port: 9001
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          exec:
            command:
            - /healthcheck.sh
          initialDelaySeconds: 5
          periodSeconds: 5
```

---

## Summary: Pattern Selection Guide

| Pattern | Traffic | Complexity | Team Size | Cost/Month |
|---------|---------|-----------|-----------|-----------|
| **Single Instance** | 0-50 TPS | Low | 1-2 | $8K |
| **Vertical** | 50-100 TPS | Low | 2-3 | $15K |
| **Horizontal** | 100-300 TPS | Medium | 4-6 | $37K |
| **Distributed** | 300-1000 TPS | High | 8-15 | $122K |
| **Multi-Cloud** | 1000+ TPS | Very High | 20-30 | $281K |

---

## Next: Implementation Playbooks

See next document for step-by-step implementation playbooks for each stage.
