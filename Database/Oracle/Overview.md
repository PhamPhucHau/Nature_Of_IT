# Building a Mini Database System Like Oracle for Learning

If you want to build your own database management system (DBMS) similar to Oracle in order to learn how databases work internally, this is an excellent project because it combines many computer science fields:

- Operating Systems
- Data Structures
- Compiler Design
- Networking
- Storage Engines
- Transactions
- Concurrency

You do not need to build a full Oracle-like system immediately. Most people learn step-by-step by implementing each layer separately.

---

# 1. Recommended Programming Languages

## Best Choices

### C / C++
- Close to hardware
- Most database systems are written in C/C++
- Excellent for learning internals

### Rust
- Memory-safe
- Modern systems programming language
- Increasingly popular for database engines

### Go / Java
- Easier for distributed systems
- Faster development

## Recommended for Learning DB Internals
Choose:
- C++
or
- Rust

---

# 2. Core Knowledge You Need

# A. Data Structures and Algorithms

You will heavily use:

- B+ Trees
- Hash Tables
- LRU Cache
- Bloom Filters
- Skip Lists
- External Sorting

## Most Important: B+ Tree

B+ Trees are the heart of database indexing.

Example:

```sql
SELECT * FROM users WHERE id = 5;
```

The database uses a B+ Tree index instead of scanning the entire table.

---

# B. Operating Systems

Extremely important topics:

- File Systems
- Paging
- Memory Mapping
- Buffer Cache
- Threads / Processes
- Locks / Mutexes
- IO Scheduling

A DBMS is essentially:
> "A mini operating system specialized for data management."

---

# C. Storage Engine

You need to understand:

- Pages
- Page Sizes
- Slotted Pages
- WAL (Write Ahead Logging)
- Checkpoints
- Serialization
- Crash Recovery

Real databases like:
- Oracle
- PostgreSQL
- MySQL

all contain:
- Buffer Pools
- Transaction Logs
- Page Managers

---

# D. Database Theory

Study:

- Relational Algebra
- SQL Parsing
- Query Optimization
- Normalization
- ACID Properties
- Transaction Isolation

## Isolation Levels

- Read Committed
- Repeatable Read
- Serializable

---

# E. Compiler / Parser Design

To parse SQL like:

```sql
SELECT name FROM users;
```

you need:

- Lexer
- Parser
- AST (Abstract Syntax Tree)

## Useful Tools

- ANTLR
- Flex/Bison

---

# F. Concurrency Control

Very important concepts:

- MVCC (Multi-Version Concurrency Control)
- Lock Managers
- Deadlock Detection
- Timestamp Ordering

Example:
What happens when two users update the same row simultaneously?

---

# G. Networking

If you want a real client-server database:

- TCP Server
- Client-Server Architecture
- Connection Pooling
- Binary Protocols

---

# 3. Basic Database Architecture

```text
Client
   |
SQL Parser
   |
Query Planner
   |
Execution Engine
   |
Storage Engine
   |
Disk
```

---

# 4. Suggested Development Roadmap

# Phase 1 — Mini Key-Value Store

Build:

- put(key, value)
- get(key)

Store everything in binary files.

## Learn:
- Serialization
- File IO

---

# Phase 2 — B+ Tree Index

Implement:

- Insert
- Search
- Page Split

This is one of the most important parts.

---

# Phase 3 — SQL Parser

Support commands like:

```sql
CREATE TABLE
INSERT
SELECT
DELETE
```

---

# Phase 4 — Buffer Pool

Implement:

- Page Cache
- Dirty Pages
- Eviction Policies

---

# Phase 5 — Transactions

Add:

- WAL
- Rollback
- Commit

---

# Phase 6 — Concurrency

Add:

- Locks
- MVCC

---

# Phase 7 — Query Optimizer

Implement:

- Execution Plans
- Index Scans
- Join Optimization

---

# 5. Database Source Code You Should Study

# SQLite

Website:
https://www.sqlite.org/index.html

## Why Study SQLite?
- Single-file database
- Famous educational codebase
- Easier to understand than enterprise systems

---

# PostgreSQL

Website:
https://www.postgresql.org/

## Why Study PostgreSQL?
- Enterprise-grade database
- High-quality architecture
- Advanced transaction system

---

# BusTub (CMU Database Project)

Website:
https://15445.courses.cs.cmu.edu/

## Why Study BusTub?
- Educational database system
- Designed specifically for learning database internals

---

# Build Your Own Database

Website:
https://build-your-own.org/database/

---

# 6. Related University Subjects

- Database Systems
- Operating Systems
- Distributed Systems
- Compiler Design
- Computer Architecture
- Storage Systems

---

# 7. What Should You Focus On?

If your goal is:
> "Understanding database internals"

focus heavily on:

- Pages
- B+ Trees
- WAL
- Transactions
- Buffer Pools

Even supporting only:

```text
INSERT
SELECT
DELETE
```

already teaches a huge amount.

---

# 8. Suggested 5-Month Learning Plan

# Month 1
- File IO
- Binary Formats
- Key-Value Store

# Month 2
- B+ Trees
- Page Splitting

# Month 3
- SQL Parser
- AST Construction

# Month 4
- Buffer Pools
- WAL

# Month 5
- Transactions
- Locks
- MVCC

---

# 9. Keywords to Search

- Database Internals
- B+ Tree Implementation
- Write Ahead Logging
- MVCC
- LSM Tree
- Query Optimizer
- Database Storage Engine

---

# 10. Recommended Books

# Database Internals
Author:
- Alex Petrov

---

# Designing Data-Intensive Applications
Author:
- Martin Kleppmann

---

# Architecture of a Database System
Author:
- Joseph M. Hellerstein

---

# 11. Suggested MiniDB Project

# MiniDB v1

## Features
- Create Table
- Insert
- Select
- Delete

## Storage
- Binary Files
- Page Manager

## Indexing
- B+ Tree

## Parser
- Simple SQL Parser

## Logging
- WAL

## Concurrency
- Mutex Locks

---

# 12. Recommended Technologies

## For Deep Learning Experience
- C++
- Rust

## For Faster Development
- Go
- Java

---

# 13. Final Goal

After building your mini database, you will understand:

- How databases store data on disk
- Why indexes speed up queries
- How transactions work internally
- How crash recovery works
- How systems like Oracle, PostgreSQL, and MySQL operate internally

---