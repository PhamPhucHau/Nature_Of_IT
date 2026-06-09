# 🗄️ Phase 6: Spring Data — Database Layer

> **Spring Concept**: `JdbcTemplate`, `DataSource`, `HikariCP`, `JpaRepository`
> **C Equivalent**: Socket connection pool + prepared statement simulation

---

## The Data Access Layers

```text
Your Code (@Service)
  ↓ calls
Repository Interface (@Repository)
  ↓ implemented by
Spring Data JPA / JdbcTemplate
  ↓ uses
DataSource (connection pool)
  ↓ provides
JDBC Connection (TCP socket to DB)
  ↓ communicates via
Database Wire Protocol (MySQL protocol, PostgreSQL protocol)
  ↓ hits
Actual Database (MySQL, PostgreSQL, Oracle...)
```

---

## Connection Pool (HikariCP)

Why a connection pool?

```text
WITHOUT pool:
  Every query = new TCP connection to DB
  → 3-way TCP handshake every time = SLOW
  → Database overwhelmed with connections

WITH pool (HikariCP):
  App starts → creates 10 connections → keeps them warm
  Each query:
    1. Borrow connection from pool
    2. Execute SQL
    3. Return connection to pool (not closed!)
  → No TCP handshake overhead
  → DB sees max 10 connections ever
```

HikariCP pool configuration:
```yaml
spring:
  datasource:
    hikari:
      minimum-idle: 5          # always keep 5 connections
      maximum-pool-size: 20    # never exceed 20 connections
      connection-timeout: 30000 # wait max 30s for free connection
      idle-timeout: 600000      # close idle connections after 10min
```

---

## JdbcTemplate

```java
@Repository
public class UserRepository {

    @Autowired
    private JdbcTemplate jdbc;

    // Query — returns list
    public List<User> findAll() {
        return jdbc.query(
            "SELECT * FROM users",
            (rs, rowNum) -> mapRow(rs)  // RowMapper
        );
    }

    // Query — single row
    public User findById(Long id) {
        return jdbc.queryForObject(
            "SELECT * FROM users WHERE id = ?",
            (rs, rowNum) -> mapRow(rs),
            id  // binds to ?
        );
    }

    // Insert/Update/Delete
    public int save(User user) {
        return jdbc.update(
            "INSERT INTO users (name, email) VALUES (?, ?)",
            user.getName(), user.getEmail()
        );
    }
}
```

---

## Spring Data JPA (higher level)

```java
// You only write the interface — Spring generates the implementation
@Repository
public interface UserRepository extends JpaRepository<User, Long> {

    // Spring generates: SELECT * FROM users WHERE email = ?
    Optional<User> findByEmail(String email);

    // Spring generates: SELECT * FROM users WHERE name LIKE ? AND active = ?
    List<User> findByNameContainingAndActive(String name, boolean active);

    // Custom JPQL query
    @Query("SELECT u FROM User u WHERE u.createdAt > :since")
    List<User> findRecentUsers(@Param("since") LocalDate since);
}
```

---

## RowMapper — ResultSet → Object

```java
private User mapRow(ResultSet rs, int rowNum) throws SQLException {
    User u = new User();
    u.setId(rs.getLong("id"));
    u.setName(rs.getString("name"));
    u.setEmail(rs.getString("email"));
    u.setCreatedAt(rs.getTimestamp("created_at").toLocalDateTime());
    return u;
}
```

In C: this is a function pointer that maps each result row to a struct.

---

## C Files in This Phase

| File                  | What It Simulates                            |
|-----------------------|----------------------------------------------|
| `connection_pool.c`   | HikariCP: pool init, borrow, return, timeout |
| `jdbc_template.c`     | JdbcTemplate: query, update, batch           |
| `result_mapper.c`     | RowMapper: ResultSet → struct                |

---

## Build and Run

```bash
gcc -Wall -Wextra -o connection_pool connection_pool.c && ./connection_pool
gcc -Wall -Wextra -o jdbc_template jdbc_template.c && ./jdbc_template
```

---

## Key Insight

```text
Spring Data JPA is 4 layers of abstraction:

  Your @Repository interface
    ↓ Spring generates implementation at startup (using Reflection + CGLIB)
  JPA / Hibernate (ORM)
    ↓ translates Java objects to SQL
  JDBC (Java Database Connectivity)
    ↓ sends SQL over TCP socket
  Database TCP connection
    ↓ database wire protocol
  Database

JdbcTemplate is layers 3-4 only — much simpler, more control.
JPA is all 4 layers — more automation, less control.

Production rule: use JdbcTemplate for complex queries, JPA for simple CRUD.
```
