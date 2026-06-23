# Smartifi Architecture - Quick Reference

## System at a Glance

```
ATM Network → NAPAS DP Gateway
                    ↓
        [smartifi - Message Router]
                    ↓
         Host I/F (Core Banking)
```

**Role:** Real-time ISO 8583 ↔ OASIS message translator  
**Target:** Shinhan Vietnam Bank  
**Protocol:** TCP/IP + Oracle DB + Unix Signals  
**Throughput:** ~100 TPS per instance  
**Latency:** 200-500ms typical  

---

## Core Data Structures

### Message Header Stack (incoming order)
```c
ChnlHead   // Branch, Terminal, Reference
  ↓
MsgHead    // Global ID, Process Sequence  
  ↓
MsgFix     // Fixed-length fields
  ↓
MsgData    // ISO 8583 bitmap (bit_0 to bit_128)
```

### ISO 8583 Bitmap (Key Fields)

| Bit | Name | Length | Purpose | Example |
|-----|------|--------|---------|---------|
| 0 | MTI | 4 | Message Type Indicator | "0200" (request) |
| 2 | PAN | 19 | Card Number | "4111111111111111" |
| 3 | PrCode | 6 | Processing Code | "000000" (withdrawal) |
| 4 | Amount | 12 | Transaction Amount | "000000100000" (100k VND) |
| 7 | DateTime | 10 | Transmission Date/Time | "0622120530" |
| 11 | STAN | 6 | Trace Number | "000001" |
| 39 | RespCode | 2 | Response Code | "00" (OK) / "05" (Decline) |
| 41 | TermID | 8 | Terminal ID | "12345678" |
| 52 | PIN | Variable | Encrypted PIN Block | [binary] |
| 55 | ChipData | 999 | EMV Tag-Length-Value | [binary] |

### Processing States

```
NORM_PROC_YN Values:
  0 = Unprocessed (pending)
  1 = Processed (in-flight)
  2 = Cleared/Purged (archived)
```

---

## Main Control Flow

### Incoming Path (NAPAS → smartifi)

```
┌─ main() event loop
│   ├─ select() waits on sock_fd
│   └─ When readable: runBizProc()
│
├─ runBizProc(sock_fd)
│   ├─ recvFromNAPAS()  // TCP read (4-byte len + payload)
│   │   └─ Validate length, retry on EAGAIN
│   │
│   ├─ Message type dispatch
│   │   ├─ 0800/0810 → recvNetworkMgmt() → sendNetworkRspNAPAS()
│   │   └─ 0200/0420 → [financial processing]
│   │
│   ├─ Iso8583toOasis()  // Parse bitmap → MsgData
│   │   ├─ Parse bitmap bits 0-128
│   │   ├─ Extract each present field
│   │   └─ Decrypt PIN if present (via HSM)
│   │
│   ├─ makeHostSendMsg()  // Format for Host I/F
│   │   └─ Build ChnlHead + MsgHead + trdat
│   │
│   ├─ insertToSWSNAPRCV()  // Log to database
│   │   └─ INSERT GAT_SWSNAPRCV_LOG with 40+ fields
│   │
│   └─ sendToAITHER()  // Queue to Host I/F
│       └─ INSERT GAT_SWSANYSND_LOG
```

### Outgoing Path (Host I/F → NAPAS)

```
┌─ main() event loop, outer
│   └─ recvFromAITHER()  // Periodic check every iteration
│
├─ recvFromAITHER()
│   ├─ OPEN cursor on GAT_SWSNAPSND_LOG (NORM_PROC_YN = 0)
│   │
│   ├─ FOR EACH pending transaction
│   │   ├─ Fetch TRX_DT, ATM_LGGTM
│   │   │
│   │   ├─ SELECT full message from database
│   │   │   └─ Extract trdat (channel header + message)
│   │   │
│   │   ├─ Parse trdat into ChnlHead, MsgHead, MsgData
│   │   │
│   │   ├─ OasistoIso8583()  // Convert to ISO 8583
│   │   │   └─ Rebuild bitmap, format response fields
│   │   │
│   │   ├─ UPDATE GAT_SWSNAPSND_LOG
│   │   │   └─ NORM_PROC_YN = 1, store NAPAS_MSG_DTL_CTT
│   │   │   └─ COMMIT
│   │   │
│   │   └─ sendToNAPAS()  // TCP write
│   │       └─ Prepend 4-byte length, send to sock_fd
│   │
│   └─ CLOSE cursor
```

---

## Function Map (Call Graph)

### Initialization
```
main()
├─ initSignal()          // Register SIGTERM, SIGSEGV handlers
├─ initLogging()         // Create log file (rotate daily)
├─ check_running()       // Acquire exclusive lock
├─ ora_db_connect()      // Connect to Oracle (retry 100×)
├─ getLogLevel()         // Load LOG_ERROR/INFO/DEBUG from DB
├─ loadHostInfo()        // Fetch DP_IBT_I_V2_ADDR0 from DB
└─ loadDesKey()          // Fetch HSM keys from DB
```

### Main Loop
```
for (;;) {
  if (sock_fd == -1)
    hostConnect()        // Establish TCP to NAPAS
  
  clearBeforeData()      // Purge old logs (NORM_PROC_YN = 0→2)
  loadDesKey()           // Reload keys periodically
  
  while (1) {
    checkDate()          // Rotate log if date changed
    
    if (cnt % 12000 == 0)
      getLogLevel()      // Refresh log level every ~2 min
    
    recvFromAITHER()     // Check Host I/F outbound queue
    
    select(sock_fd, ...)  // Wait for NAPAS data
    
    if (readable)
      runBizProc()       // Process incoming NAPAS message
  }
}
```

### Shutdown
```
stopProc()
├─ EXEC SQL ROLLBACK WORK RELEASE  // Abort pending TX
├─ shutdown(sock_fd, SHUT_RDWR)    // Close socket
├─ close(lock_fd)                   // Release lock
├─ fclose(logfp)                    // Flush logs
└─ exit(0)
```

---

## Error Handling Strategy

### By Component

**Network (Socket):**
- EAGAIN/EWOULDBLOCK → retry with select()
- Connection closed → reconnect (with backoff)
- Timeout (select=0) → send ping / increment counter

**Database (Pro*C):**
- SQL_ERROR → check SQLCODE
- SQLCODE = -3114 → connection lost, reconnect
- SQL_NOTFOUND → expected in cursors (break loop)
- Other → log error, rollback, return ABNORMAL

**Message Processing:**
- Truncated message → return ABNORMAL (caller retry)
- Invalid bitmap → log and skip (safety default)
- PIN decrypt fail → return response code "85" (crypto error)

**Process Lifecycle:**
- Signal SIGSEGV/SIGBUS → fatalSignalHandler() → backtrace → shutdown
- Signal SIGTERM → stopProc_sig() → graceful shutdown
- Lock file exists → check_running() returns -1 (already running)

---

## Key Configuration Defaults

```c
#define NH_USLEEP_INTVAL    300000    // 300ms socket timeout
#define NH_BIZ_TIMEOUT      NH_USLEEP_INTVAL * 100  // ~30 sec
#define MAX_DPDATALEN       4000      // Max ISO message size
#define MAXBITMAP           128       // Extended bitmap
#define UNIX_SOCKET_PATH_HSM "/tmp/hsm/"  // Local HSM socket
```

### Database Configuration Keys

| Key | Purpose |
|-----|---------|
| DP_IBT_I_V2_ADDR0 | NAPAS DP IP address |
| DP_IBT_I_V2_PORT0 | NAPAS DP port |
| HSM_HOSTADDR1 | Primary HSM IP |
| HSM_HOSTPORT1 | Primary HSM port |
| HSM_HOSTADDR3 | Secondary HSM IP |
| HSM_HOSTPORT3 | Secondary HSM port |
| DP_V2_ZAKKEY | NAPAS encryption key |
| DP_CLEARKEY | PIN block DES key |
| LOGLVL_{PROC} | Log level for process |

---

## Critical Code Sections

### Socket Read (with retry on partial read)
```c
int readSocket(int sock, char *buf, int len) {
  int n, total = 0;
  while (total < len) {
    n = read(sock, buf + total, len - total);
    if (n <= 0) return -1;  // Connection closed
    total += n;
  }
  return total;
}
```

### ISO 8583 Bitmap Parsing
```c
int bit = 0;
for (int i = 0; i < 8; i++) {  // or 16 for 128-bit
  unsigned char byte = RecvBuffer[4 + i];
  for (int j = 0; j < 8; j++) {
    if (byte & (0x80 >> j)) {
      // Bit is SET → extract field
      int field_num = i * 8 + j + 1;
      char *field_data = extract_field(RecvBuffer, field_num);
    }
  }
}
```

### HSM Command Generation
```c
// Example: PIN decryption command
int hsmCmd = sprintf(buf, "NG%02d", response_code);
// Prepend length (2-byte binary)
unsigned char len[2] = {(total >> 8) & 0xFF, total & 0xFF};
```

---

## Important Patterns

### Pro*C Error Checking
```c
EXEC SQL BEGIN DECLARE SECTION;
  char var[20];
EXEC SQL END DECLARE SECTION;

EXEC SQL SELECT field INTO :var FROM table;

if (SQL_ERROR) {
  if (SQL_NOTFOUND) { /* No data */ }
  else if (SQLCODE == -3114) { /* Reconnect */ }
  else { LOGGER(LOG_ERROR, "SQL Error: %d", SQLCODE); }
}
```

### Signal Handler
```c
void fatalSignalHandler(int signo) {
  LOGGER(LOG_ERROR, "Fatal signal %d", signo);
  void *buffer[64];
  int nptrs = backtrace(buffer, 64);
  backtrace_symbols_fd(buffer, nptrs, STDERR_FILENO);
  stopProc_sig(signo);
}
```

### Graceful Reconnection
```c
if (sock_fd == -1) {
  if (hostConnect() == ABNORMAL) {
    sleep(1);
    loadHostInfo();  // Refresh config
    continue;        // Retry next iteration
  }
}
```

---

## Common Issues & Solutions

| Issue | Cause | Fix |
|-------|-------|-----|
| "Process already running" | Lock file exists | Remove `/home/atmadm/.config/{proc}.lock` |
| DB connection timeout | Network to Oracle down | Check DB connectivity, restart |
| HSM socket not found | HSM process down | Start HSM server on target host |
| Garbled output in logs | Character encoding issue | Check locale settings (UTF-8) |
| Stuck in select loop | sock_fd = -1 but not reconnecting | Check hostConnect logic |
| Message loss | Transaction not logged | Verify DB commit succeeded |
| Memory leak | Long-running buffer not freed | Add malloc/free instrumentation |

---

## Monitoring Checklist

**Real-time:**
- [ ] Socket connection status (connect/disconnect events in log)
- [ ] Message throughput (TPS in last minute)
- [ ] Error rate (failed transactions %)
- [ ] DB connection status (OK / ERROR)

**Daily:**
- [ ] Transaction count (reconcile with Host I/F)
- [ ] Log file rotation (ensure new file created)
- [ ] Lock file cleanup (no stale locks)
- [ ] HSM key availability (test with dummy command)

**Weekly:**
- [ ] Performance trend (latency percentiles)
- [ ] Disk usage (log archival strategy)
- [ ] Database growth (purge old records)
- [ ] Process uptime SLA (99.95% target)

**Monthly:**
- [ ] Backup verification (restore test)
- [ ] Security audit (PCI-DSS log review)
- [ ] Configuration review (HSM keys, DB credentials)
- [ ] Capacity planning (growth rate analysis)

---

## Deployment Checklist

```
[ ] Compile with -g for debugging symbols
[ ] Link against Pro*C runtime & Oracle libraries
[ ] Create log directory with correct permissions
[ ] Pre-create lock file directory
[ ] Verify Oracle connectivity (test SQL query)
[ ] Verify HSM connectivity (test command)
[ ] Set up log rotation (logrotate or cron)
[ ] Create systemd unit or init script
[ ] Test graceful shutdown (SIGTERM → clean exit)
[ ] Load test (measure TPS at 10%, 50%, 100% capacity)
[ ] Production deployment (blue-green or staged)
```

---

**Quick Links:**
- ISO 8583 Spec: [ISO/IEC 61010-1]
- Oracle Pro*C: [docs.oracle.com/en/database/]
- Unix Signals: [man 7 signal]
- Linux Sockets: [man 7 ip]

