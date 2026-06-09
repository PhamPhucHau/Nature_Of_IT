# 🔄 Phase 9: Spring Transaction Management

> **Spring Concept**: `@Transactional`, `PlatformTransactionManager`, Propagation
> **C Equivalent**: Nested begin/commit/rollback wrappers with save points

---

## What is @Transactional?

```java
@Transactional
public void transferMoney(Long fromId, Long toId, BigDecimal amount) {
    User from = userRepo.findById(fromId);
    User to   = userRepo.findById(toId);

    from.setBalance(from.getBalance().subtract(amount));  // debit
    to.setBalance(to.getBalance().add(amount));           // credit

    userRepo.save(from);
    userRepo.save(to);
    // If ANYTHING above fails → BOTH changes are ROLLED BACK
    // Either BOTH succeed or NEITHER succeeds (ACID: Atomicity)
}
```

Without `@Transactional`, if debit succeeds but credit fails, you LOSE money.

---

## How @Transactional Works Internally

```text
1. Spring detects @Transactional at startup → creates AOP proxy
2. When method is called → TransactionInterceptor runs:
   a. transactionManager.getTransaction(txDef)  ← BEGIN
   b. Binds connection to current Thread (ThreadLocal)
   c. Calls the REAL method
   d. If no exception → transactionManager.commit()
   e. If RuntimeException → transactionManager.rollback()
3. Connection is returned to pool after commit/rollback
```

The connection is stored in `TransactionSynchronizationManager` — a `ThreadLocal<Map>`.
This ensures all repository calls in the same thread use the SAME connection.

---

## Transaction Propagation

```java
@Transactional(propagation = Propagation.REQUIRED)       // default: join existing or create new
@Transactional(propagation = Propagation.REQUIRES_NEW)   // always create new, suspend existing
@Transactional(propagation = Propagation.NESTED)         // create savepoint in existing tx
@Transactional(propagation = Propagation.MANDATORY)      // must have existing tx, else throw
@Transactional(propagation = Propagation.NEVER)          // must NOT have existing tx, else throw
@Transactional(propagation = Propagation.SUPPORTS)       // join if exists, else run without
@Transactional(propagation = Propagation.NOT_SUPPORTED)  // suspend existing, run without
```

Most important:

```text
REQUIRED (default):
  If txA is running and txA calls txB(REQUIRED):
    txB joins txA — they share ONE transaction
    If txB fails → entire txA also fails

REQUIRES_NEW:
  If txA is running and txA calls txB(REQUIRES_NEW):
    txA is SUSPENDED
    txB runs in a BRAND NEW transaction
    txB commits/rolls back independently
    txA resumes after txB finishes
    (txB success doesn't affect txA's fate)
```

---

## Isolation Levels

```java
@Transactional(isolation = Isolation.READ_COMMITTED)   // default for most DBs
@Transactional(isolation = Isolation.REPEATABLE_READ)  // MySQL InnoDB default
@Transactional(isolation = Isolation.SERIALIZABLE)     // strictest, slowest
@Transactional(isolation = Isolation.READ_UNCOMMITTED) // allow dirty reads (dangerous)
```

Problems each level prevents:

| Isolation Level   | Dirty Read | Non-Repeatable Read | Phantom Read |
|-------------------|-----------|---------------------|--------------|
| READ_UNCOMMITTED  | possible  | possible            | possible     |
| READ_COMMITTED    | prevented | possible            | possible     |
| REPEATABLE_READ   | prevented | prevented           | possible     |
| SERIALIZABLE      | prevented | prevented           | prevented    |

---

## Common Mistakes

```java
// MISTAKE 1: self-invocation bypasses the proxy
@Service class OrderService {
    @Transactional
    public void methodA() {
        methodB();    // ← calls this.methodB(), NOT proxy.methodB()
                      //   @Transactional on methodB is IGNORED!
    }

    @Transactional(propagation = REQUIRES_NEW)
    public void methodB() { ... }
}

// FIX: inject self via @Autowired, or move methodB to different class

// MISTAKE 2: checked exceptions don't trigger rollback by default
@Transactional
public void save(Data d) throws IOException {
    repo.save(d);
    throw new IOException();   // ← DOES NOT rollback by default!
}
// FIX: @Transactional(rollbackFor = IOException.class)

// MISTAKE 3: @Transactional on private methods
// Spring AOP cannot intercept private methods — annotation is ignored
```

---

## C Files in This Phase

| File                    | What It Simulates                         |
|-------------------------|-------------------------------------------|
| `transaction_manager.c` | BEGIN/COMMIT/ROLLBACK with ThreadLocal    |
| `propagation.c`         | REQUIRED vs REQUIRES_NEW behavior         |

---

## Build and Run

```bash
gcc -Wall -Wextra -o transaction_manager transaction_manager.c && ./transaction_manager
```

---

## Key Insight

```text
@Transactional is AOP + Connection binding:

  AOP proxy wraps your method with:
    begin()
    try { your method }
    catch(RuntimeException) { rollback(); throw; }
    commit()

  Connection binding ensures ALL repository calls
  in the same thread share the SAME database connection.

  Without connection binding:
    repo.findById()   ← uses connection A
    repo.save()       ← uses connection B (DIFFERENT transaction!)

  With @Transactional:
    repo.findById()   ← uses bound connection
    repo.save()       ← uses SAME bound connection (same transaction)
```
