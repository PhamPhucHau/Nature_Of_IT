# Implementation Playbooks - Step-by-Step Guides

## Playbook 1: Stage 1 → Stage 2 Migration (Single to Vertical Scaling)

### Timeline: 4-6 weeks

### Week 1: Planning & Preparation

**Day 1-2: Load Testing**
```bash
# Current system load test
# Target: Measure current bottleneck

# Test script
for i in {1..100}; do
  for j in {1..1000}; do
    curl -X POST http://smartifi:9001/transaction \
      -d "user=$RANDOM&amount=$((RANDOM % 1000))" \
      &
  done
  wait
  sleep 5
done

# Measure metrics:
# - Peak CPU: ____%
# - Peak Memory: ____MB
# - Database connections: ____
# - Transaction latency: ____ms
# - Error rate: ____._%
```

**Day 3: Identify Bottleneck**
```
Is it:
  ☐ CPU-bound? (> 80% CPU usage)
  ☐ Memory-bound? (> 80% memory usage)
  ☐ I/O-bound? (disk operations taking > 50% time)
  ☐ Database-bound? (connections exhausted or query slow)
  ☐ Network-bound? (network saturation)
```

**Day 4: Plan Upgrades**
```
Based on bottleneck:

If CPU-bound:
  - Upgrade CPU (8-core → 16-core)
  - Profile code for hot spots
  - Optimize algorithms

If Memory-bound:
  - Add RAM (32GB → 64GB)
  - Optimize memory usage
  - Implement caching

If Database-bound:
  - Upgrade DB server
  - Add connection pooling
  - Optimize queries
  - Create indexes
```

**Day 5: Cost-Benefit Analysis**
```
Cost of upgrade:    $32,000 (hardware)
Capacity gain:      +50 TPS (from 50 → 100)
Cost per TPS:       $640

Revenue if $5/TPS:  +$250/month
ROI timeline:       ~11 months
Decision:           ☐ Proceed  ☐ Defer  ☐ Alternative
```

### Week 2-3: Procurement & Setup

**Day 8-10: Order Hardware**
- New app server (64GB, 16-core)
- New database server (128GB, 16-core)
- Storage upgrade
- Estimated delivery: 1-2 weeks

**Day 11-14: Prepare New Servers**
```bash
# On new application server
# 1. Install OS
# 2. Install database client libraries
# 3. Install Oracle Instant Client
# 4. Configure environment variables
export ORACLE_HOME=/opt/oracle/instantclient
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ORACLE_HOME

# 3. Compile application
cd /opt/smartifi
make clean
make

# 4. Test compilation
./smartifi --version
```

### Week 4: Configuration & Testing

**Day 22-23: Kernel Tuning**
```bash
# On new servers, apply optimizations

# /etc/sysctl.conf
net.core.somaxconn = 4096
net.core.netdev_max_backlog = 5000
net.ipv4.tcp_max_syn_backlog = 4096
fs.file-max = 2097152

# Apply changes
sysctl -p

# Test
cat /proc/sys/net/core/somaxconn  # Should show 4096
```

**Day 24: Connection Pool Implementation**
```ini
# smartifi.ini - Updated config
[DATABASE]
POOL_MIN=10
POOL_MAX=30
POOL_TIMEOUT=60
POOL_VALIDATE_INTERVAL=300

[CACHING]
CACHE_ENABLED=1
CACHE_SIZE=50000
CACHE_TTL=300
```

**Day 25-26: Load Testing on New Hardware**
```bash
# Same load test as Week 1, but on new hardware
# Expected: 2x capacity

# Compare metrics:
# Old system: 50 TPS @ 85% CPU
# New system: 100+ TPS @ 70% CPU
```

### Week 5: Deployment

**Day 29: Failover Test (Offline)**
```bash
# 1. Verify data consistency
# 2. Test failback procedures
# 3. Validate all connections
# 4. Check monitoring
```

**Day 30: Production Migration**
```bash
# Step 1: Stop traffic (if possible)
service smartifi stop
service haproxy stop  # If using LB

# Step 2: Sync database
expdp schemas=SMARTIFI directory=/backup file=backup.dmp

impdp system/password directory=/backup file=backup.dmp

# Step 3: Start on new system
service smartifi start
service haproxy start

# Step 4: Monitor
# Watch for errors: tail -f /var/log/smartifi.log
# Watch metrics: ./monitor_smartifi.sh
```

### Week 6: Stabilization & Optimization

**Day 36: Performance Tuning**
```sql
-- Analyze query performance
SELECT * FROM v$sqlarea 
WHERE elapsed_time > 1000000 
ORDER BY elapsed_time DESC;

-- Create missing indexes
CREATE INDEX idx_status_timestamp 
ON transactions(status, timestamp DESC);

-- Update table statistics
ANALYZE TABLE transactions COMPUTE STATISTICS;
```

**Day 40: Documentation**
- Update runbooks
- Document optimization changes
- Train team on new configuration
- Capture lessons learned

---

## Playbook 2: Stage 2 → Stage 3 Migration (Vertical to Horizontal)

### Timeline: 8-12 weeks

### Week 1-2: Design & Architecture

**Design Phase:**
```
Decision 1: How many instances?
  Current: 1 instance @ 100 TPS
  Target: 100-300 TPS
  Recommended: 3-5 instances (each 50-60 TPS)
  Decision: 4 instances (provides headroom)

Decision 2: Load balancer?
  Options: HAProxy, Nginx, AWS ALB
  Recommendation: HAProxy (lightweight, proven)
  Decision: HAProxy on separate server

Decision 3: Session storage?
  Options: Database, Redis, Memcached
  Recommendation: Redis (fast, simple)
  Decision: Redis cluster (3 nodes, replication)

Decision 4: Coordination?
  Options: ZooKeeper, Etcd, Consul
  Recommendation: ZooKeeper (mature, battle-tested)
  Decision: ZooKeeper (3 nodes)

Architecture:
[HAProxy 10.0.1.1]
    ├─ [smartifi-01 10.0.1.10]
    ├─ [smartifi-02 10.0.1.11]
    ├─ [smartifi-03 10.0.1.12]
    └─ [smartifi-04 10.0.1.13]
    
[Redis Cluster 10.0.2.x]
[ZooKeeper Cluster 10.0.3.x]
[Shared DB 10.0.4.x]
```

### Week 3-4: Infrastructure Provisioning

**Provisioning Checklist:**
```
☐ 4x Application servers (10.0.1.10-13)
  - 64GB RAM, 16-core CPU
  - 100GB SSD storage
  - Network: 1Gbps

☐ 1x HAProxy server (10.0.1.1)
  - 32GB RAM, 8-core CPU
  - 50GB storage

☐ 3x Redis servers (10.0.2.10-12)
  - 64GB RAM, 8-core CPU
  - 100GB storage
  - Replication enabled

☐ 3x ZooKeeper servers (10.0.3.10-12)
  - 32GB RAM, 8-core CPU
  - 50GB storage

☐ Shared Database (10.0.4.x)
  - Upgrade to 256GB RAM, 32-core CPU
  - Upgrade storage to 500GB
```

### Week 5-6: Component Setup

**HAProxy Installation & Configuration:**
```bash
# Install HAProxy
apt-get install haproxy

# Configuration: /etc/haproxy/haproxy.cfg
# (See ARCHITECTURE_PATTERNS_DETAILED.md for full config)

# Enable and start
systemctl enable haproxy
systemctl start haproxy

# Test
curl http://localhost:8404/stats  # HAProxy stats page
```

**Redis Cluster Setup:**
```bash
# Install Redis on 3 nodes
for node in 10.0.2.10 10.0.2.11 10.0.2.12; do
  ssh $node "apt-get install redis-server"
done

# Create cluster
redis-cli --cluster create \
  10.0.2.10:6379 \
  10.0.2.11:6379 \
  10.0.2.12:6379 \
  --cluster-replicas 0

# Verify
redis-cli -h 10.0.2.10 cluster info
```

**ZooKeeper Cluster Setup:**
```bash
# Install on 3 nodes
for i in 1 2 3; do
  node="10.0.3.$((9+i))"
  ssh $node "apt-get install zookeeper"
  ssh $node "echo 'server.$i=10.0.3.$((9+i)):2888:3888' >> /etc/zookeeper/zoo.cfg"
done

# Start cluster
for node in 10.0.3.10 10.0.3.11 10.0.3.12; do
  ssh $node "systemctl start zookeeper"
done

# Verify
echo stat | nc 10.0.3.10 2181
```

### Week 7: Application Deployment

**Deploy Instance 1:**
```bash
# On smartifi-01 (10.0.1.10)

# 1. Deploy application
scp smartifi user@10.0.1.10:/opt/smartifi/

# 2. Configure instance
ssh 10.0.1.10 "cat > /opt/smartifi/smartifi.ini << EOF
[SERVICE]
INSTANCE_ID=smartifi-01
PORT=9001
REDIS_HOST=10.0.2.10
REDIS_CLUSTER=yes
ZOOKEEPER_HOSTS=10.0.3.10,10.0.3.11,10.0.3.12
DATABASE=production
EOF"

# 3. Register with ZooKeeper
ssh 10.0.1.10 "/opt/smartifi/smartifi &"

# 4. Verify
curl http://10.0.1.10:9001/health
```

**Deploy Instances 2-4:**
```bash
# Repeat for 10.0.1.11-13
# (Automated via Ansible/Puppet in real scenario)

for i in 11 12 13; do
  server="10.0.1.$i"
  ssh $server "/opt/smartifi/smartifi &"
  sleep 5
  curl http://$server:9001/health
done
```

### Week 8: Migration & Testing

**Shadow Mode (Risk Reduction):**
```
Step 1: Start sending 5% traffic to new cluster
  HAProxy weight: old=95, new=5

Step 2: Monitor for 24 hours
  - Error rates (target: < 0.1%)
  - Latency (target: similar ±10%)
  - Database load (should be distributed)

Step 3: Increase to 20%
  HAProxy weight: old=80, new=20
  Monitor for 24 hours

Step 4: Increase to 50%
  HAProxy weight: old=50, new=50
  Monitor for 48 hours

Step 5: Increase to 100%
  HAProxy weight: old=0, new=100
  Complete cutover
```

**Failover Testing:**
```bash
# Kill one instance
ssh 10.0.1.10 "killall smartifi"

# Verify:
# 1. HAProxy detects failure (< 30 sec)
curl http://localhost:8404/stats | grep smartifi-01

# 2. Remaining instances handle traffic
# 3. No customer impact

# 4. Restart instance
ssh 10.0.1.10 "/opt/smartifi/smartifi &"

# 5. Verify rejoin
sleep 10
curl http://10.0.1.10:9001/health
```

### Week 9-12: Optimization & Documentation

**Performance Tuning:**
```sql
-- Analyze query distribution
SELECT * FROM v$sqlarea 
WHERE executions > 1000 
ORDER BY elapsed_time DESC;

-- Optimize hot queries
CREATE INDEX idx_account_balance 
ON accounts(account_id, balance);

-- Monitor connection pool
SELECT COUNT(*) FROM v$session 
GROUP BY username;
```

**Documentation:**
- Update runbooks for multi-instance
- Document failover procedures
- Update monitoring dashboards
- Create incident response guides
- Record lessons learned

---

## Playbook 3: Stage 3 → Stage 4 (Horizontal to Distributed/Multi-Region)

### Timeline: 12-16 weeks

### Phase 1: Design Multi-Region Architecture (Week 1-4)

**Step 1: Identify Regions**
```
Regions based on user distribution:
☐ US-East (NYC) - 40% traffic
☐ EU-West (London) - 35% traffic  
☐ AP-South (Mumbai) - 25% traffic

Each region: 5 app servers + shared infrastructure
```

**Step 2: Data Replication Strategy**
```
Option A: Master-Slave (Simple)
  US-East (Master)
    ↓ Binary Log
    ├─ EU-West (Read-only Slave)
    └─ AP-South (Read-only Slave)
  
  Pros: Simple, low complexity
  Cons: Can only write to US-East
  Replication lag: 100-500ms

Option B: Multi-Master (Complex)
  US-East ↔ EU-West ↔ AP-South
  
  Pros: Can write anywhere
  Cons: Complex conflict resolution
  Replication lag: 50-200ms

Recommendation: Start with Master-Slave, upgrade to Multi-Master later
```

**Step 3: Disaster Recovery Plan**
```
RTO (Recovery Time Objective): < 30 minutes
RPO (Recovery Point Objective): < 5 minutes

Failover triggers:
  - Master unavailable for > 30 seconds
  - Network partition detected
  - Manual failover command

Failover procedure:
  1. Detect failure (5-10 sec)
  2. Global load balancer routes to failover region (1-5 sec)
  3. Promote replica to master (5-10 sec)
  4. Resync other replicas (1-2 min)
  5. Full failover complete (< 15 minutes)
```

### Phase 2: Regional Infrastructure (Week 5-8)

**For Each Region (3x parallel):**

```bash
# Region 1: US-East

# 1. Provision servers (Week 5-6)
- 5x App servers (smartifi-01 to 05)
- 1x HAProxy LB
- 3x Redis cluster
- 3x ZooKeeper
- 1x Database (Master in US-East, Slave in other regions)

# 2. Setup regional HAProxy (similar to Stage 2)

# 3. Setup regional Redis cluster

# 4. Setup regional ZooKeeper

# 5. Deploy applications

# 6. Test region independently (Week 7)

# 7. Integrate with other regions (Week 8)
```

### Phase 3: Cross-Region Replication (Week 9-10)

**Master-Slave Replication Setup:**
```sql
-- On Master (US-East)
CREATE USER repl_user IDENTIFIED BY 'password';
GRANT REPLICATION SLAVE ON *.* TO 'repl_user'@'%';

-- On Slave (EU-West)
CHANGE MASTER TO
  MASTER_HOST='us-east-db.bank.com',
  MASTER_USER='repl_user',
  MASTER_PASSWORD='password',
  MASTER_LOG_FILE='mysql-bin.000001',
  MASTER_LOG_POS=154;

START SLAVE;
SHOW SLAVE STATUS;  -- Verify replication

-- Replication lag monitoring
SELECT TIMESTAMPDIFF(SECOND, MAX(ts), NOW()) 
FROM replication_heartbeat;
```

### Phase 4: Global Load Balancing (Week 11-12)

**GeoDNS Configuration:**
```
smartifi.bank.com
  ├─ User in US → Route to us-east-lb.bank.com (52.1.1.1)
  ├─ User in EU → Route to eu-west-lb.bank.com (54.1.1.1)
  └─ User in Asia → Route to ap-south-lb.bank.com (1.1.1.1)

DNS TTL: 5-30 seconds (balance between failover speed and load)
```

**Failover Testing:**
```
Scenario 1: US-East Region Down
  1. Simulate: DNS stops responding from US-East
  2. Observe: Clients in US get routed to EU-West (2-5 min)
  3. Impact: Latency increases 100-200ms for US clients
  4. Recover: Restart US-East region

Scenario 2: Database Replication Lag
  1. Simulate: Stop replication on EU-West slave
  2. Monitor: Replication lag increases
  3. Alert: Triggers at > 30 second lag
  4. Action: DBA investigates and fixes

Scenario 3: Complete Region Failure
  1. Simulate: Turn off entire US-East region
  2. Failover: Promote EU-West to master (< 30 min)
  3. Verify: All traffic flows to EU-West and AP-South
  4. Monitor: Performance metrics in failover region
```

### Phase 5: Testing & Stability (Week 13-16)

**Load Testing Across Regions:**
```bash
# Generate global traffic pattern
# 40% from US, 35% from EU, 25% from Asia

# Test with peak traffic
# Expected: All regions @ 60-70% CPU
# Should handle 300-1000 TPS across all regions
```

**Compliance & Certification:**
```
☐ ISO 27001 audit
☐ SOC 2 Type II assessment
☐ PCI-DSS compliance review
☐ Data residency verification
☐ Disaster recovery plan approval
```

---

## Quick Reference: Migration Checklist

### Before Any Migration

- ✓ Load tested current system to identify bottleneck
- ✓ Calculated ROI and secured budget
- ✓ Planned new architecture thoroughly
- ✓ Identified risks and mitigation strategies
- ✓ Scheduled maintenance window
- ✓ Notified all stakeholders
- ✓ Prepared rollback procedure

### During Migration

- ✓ Monitor all metrics continuously
- ✓ Have rollback plan ready
- ✓ Have incident commander on call
- ✓ Document any issues
- ✓ Verify data consistency at checkpoints

### After Migration

- ✓ Monitor for 24-72 hours continuously
- ✓ Run failover test within 48 hours
- ✓ Gather performance baselines
- ✓ Document lessons learned
- ✓ Update runbooks and procedures
- ✓ Train operations team
- ✓ Review budget vs. actuals
- ✓ Plan next scaling stage

---

## Cost Tracking Template

| Stage | Item | Planned | Actual | Variance |
|-------|------|---------|--------|----------|
| 2 | Hardware | $32K | $31.5K | +$0.5K |
| 2 | Software | $13K | $13K | $0 |
| 2 | Personnel | $100K | $105K | -$5K |
| 2 | Hidden | $50K | $65K | -$15K |
| **2** | **TOTAL** | **$195K** | **$214.5K** | **-$19.5K** |

---

## Success Metrics by Stage

### Stage 2 Success (After 1 month):
- ✓ Capacity doubled (50 → 100 TPS)
- ✓ CPU usage < 70%
- ✓ No errors introduced
- ✓ Latency maintained (±10%)

### Stage 3 Success (After 1 month):
- ✓ Capacity 200-300 TPS
- ✓ All instances healthy
- ✓ Failover < 30 seconds
- ✓ Session consistency verified

### Stage 4 Success (After 1 month):
- ✓ Cross-region replication lag < 500ms
- ✓ Failover procedures tested
- ✓ Compliance certifications obtained
- ✓ 99.9% availability achieved

### Stage 5 Success (After 1 month):
- ✓ Multi-cloud deployment operational
- ✓ CDN edge caching working
- ✓ Kubernetes autoscaling functional
- ✓ 99.999% availability maintained

---

End of Implementation Playbooks. See ARCHITECTURE_EVOLUTION_GUIDE.md for stage definitions.
