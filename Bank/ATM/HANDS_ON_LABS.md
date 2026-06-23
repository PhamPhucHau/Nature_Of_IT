# Smartifi Architecture - Hands-On Labs

## Lab Environment Setup

### Prerequisites
- C compiler (gcc, clang)
- Pro*C precompiler (Oracle InstantClient)
- Oracle client libraries
- gdb debugger
- Make utility
- Network tools (nc, tcpdump)

### Quick Setup Script
```bash
# Install dependencies (macOS with Homebrew)
brew install gcc oracle-instantclient

# Create lab workspace
mkdir -p ~/smartifi-labs
cd ~/smartifi-labs
mkdir -p {phase1, phase2, phase3, phase4, phase5}

# Set environment
export ORACLE_HOME=/usr/local/opt/oracle-instantclient
export LD_LIBRARY_PATH=$ORACLE_HOME/lib:$LD_LIBRARY_PATH
export PATH=$ORACLE_HOME/bin:$PATH
```

---

## Phase 1 Labs: Foundations

### Lab 1.1: ISO 8583 Parser (Manual Binary)

**Objective:** Parse a real ISO 8583 message without external libraries

**Sample Binary Message (Hex):**
```
// Complete 0200 withdrawal request
00 50                                    // Length (80 bytes)
32 30 30 30                              // MTI: "2000" → bit 0 = "0200"
80 00 00 00 00 00 00 00                  // Bitmap: bits 2,3,4,7,11,12,39,41 present
34 31 31 31 31 31 31 31 31 31 31 31 31 31 31 31  // PAN: "4111111111111111" (bit 2)
00 00 00                                 // Processing Code: "000000" (bit 3)
00 00 00 10 00 00                        // Amount: "100000" (bit 4)
06 22 12 05 30                           // DateTime: "0622120530" (bit 7)
00 00 01                                 // STAN: "000001" (bit 11)
06 22                                    // Expiry: "0622" (bit 12)
00                                       // Response Code: "00" (bit 39)
49 54 45 53 54                           // Terminal ID: "ITEST" (bit 41)
```

**Task 1.1.1: Write Parser**
```c
#include <stdio.h>
#include <string.h>

typedef struct {
  char mti[4];
  unsigned char bitmap[16];
  char pan[20];
  char proc_code[7];
  char amount[13];
  char datetime[11];
  char stan[7];
  char response[3];
  char term_id[9];
} ISO8583_MSG;

int parse_iso8583(unsigned char *buf, int len, ISO8583_MSG *msg) {
  int pos = 2;  // Skip length
  
  // Extract MTI
  memcpy(msg->mti, &buf[pos], 4);
  pos += 4;
  
  // Extract bitmap (8 bytes = 64 bits)
  memcpy(msg->bitmap, &buf[pos], 8);
  pos += 8;
  
  // Check each bit and extract field
  if (msg->bitmap[0] & 0x80)  // Bit 2 (PAN)
    // Extract PAN...
    pos += ?;
  
  if (msg->bitmap[0] & 0x40)  // Bit 3 (Processing Code)
    // Extract...
  
  // Continue for all bits...
  
  return pos;
}

int main() {
  unsigned char hex_data[] = { /* ... your hex above ... */ };
  ISO8583_MSG msg;
  
  parse_iso8583(hex_data, sizeof(hex_data), &msg);
  
  printf("MTI: %s\n", msg.mti);
  printf("PAN: %s\n", msg.pan);
  printf("Amount: %s\n", msg.amount);
  
  return 0;
}
```

**Task 1.1.2: Verify Results**
- Parse the sample message
- Print extracted fields
- Validate field lengths against schema
- Handle variable-length fields (LLVAR)

**Task 1.1.3: Generate Response**
- Create ISO 8583 0210 response
- Include response code "00" in bit 39
- Rebuild bitmap with response bits set
- Output hex dump

---

### Lab 1.2: Pro*C Database Connection

**Objective:** Connect to Oracle, insert/query data with error handling

**Setup:**
```bash
# Create test table
sqlplus atmdba@ORACLE_SID

SQL> CREATE TABLE TEST_TRANSACTION (
  TRX_ID NUMBER,
  TRX_TYPE VARCHAR2(4),
  AMOUNT NUMBER,
  STATUS VARCHAR2(1),
  CREATED_DATE DATE
);

SQL> CREATE SEQUENCE test_trx_seq START WITH 1 INCREMENT BY 1;
```

**Task 1.2.1: Write Pro*C Insert**
```c
/*
 * insert_transaction.pc
 * Compile: proc iname=insert_transaction.pc include=$ORACLE_HOME/precomp/public
 *          gcc -o insert_transaction insert_transaction.c \
 *              -I$ORACLE_HOME/precomp/public \
 *              -I$ORACLE_HOME/sdk/include \
 *              -L$ORACLE_HOME/lib \
 *              -lclntsh
 */
#include <stdio.h>
#include <string.h>

EXEC SQL INCLUDE SQLCA;

EXEC SQL BEGIN DECLARE SECTION;
  char username[20] = "atmdba";
  char password[20] = "password";
  int trx_id;
  char trx_type[5];
  int amount;
EXEC SQL END DECLARE SECTION;

int main() {
  EXEC SQL WHENEVER SQLERROR DO sql_error();
  
  EXEC SQL CONNECT :username IDENTIFIED BY :password;
  
  strcpy(trx_type, "0200");
  amount = 100000;
  
  EXEC SQL INSERT INTO TEST_TRANSACTION 
    (TRX_ID, TRX_TYPE, AMOUNT, STATUS, CREATED_DATE)
    VALUES (test_trx_seq.NEXTVAL, :trx_type, :amount, '0', SYSDATE);
  
  EXEC SQL COMMIT;
  printf("Inserted transaction, ID: %d\n", SQLCA.sqlerrd[2]);
  
  EXEC SQL DISCONNECT;
  return 0;
}

void sql_error() {
  printf("SQL Error %d: %.70s\n", SQLCA.sqlcode, SQLCA.sqlerrm.sqlerrmc);
  exit(1);
}
```

**Task 1.2.2: Query & Cursor**
```c
// Add to above:
EXEC SQL DECLARE c_trans CURSOR FOR
  SELECT TRX_ID, TRX_TYPE, AMOUNT FROM TEST_TRANSACTION
  WHERE STATUS = '0'
  ORDER BY CREATED_DATE DESC;

EXEC SQL OPEN c_trans;

while (1) {
  EXEC SQL FETCH c_trans INTO :trx_id, :trx_type, :amount;
  
  if (SQL_NOTFOUND) break;
  
  printf("TRX %d: %s - %d\n", trx_id, trx_type, amount);
}

EXEC SQL CLOSE c_trans;
```

**Task 1.2.3: Error Handling**
- Test with wrong password → catch SQL error
- Test with invalid table → SQL_NOTFOUND
- Test with connection loss (kill -9 sqlplus) → reconnect logic
- Verify COMMIT behavior (transaction isolation)

---

### Lab 1.3: Unix Signals & Process Control

**Objective:** Implement signal handlers and process lifecycle

**Task 1.3.1: Signal Handler**
```c
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <execinfo.h>

volatile int shutdown_requested = 0;

void signal_handler(int sig) {
  printf("\n[SIGNAL] Caught signal %d\n", sig);
  
  if (sig == SIGSEGV || sig == SIGBUS) {
    // Print stack trace
    void *addrlist[10];
    int addrlen = backtrace(addrlist, 10);
    char **symbollist = backtrace_symbols(addrlist, addrlen);
    
    for (int i = 0; i < addrlen; i++) {
      printf("  [%d] %s\n", i, symbollist[i]);
    }
    free(symbollist);
  }
  
  if (sig == SIGTERM) {
    shutdown_requested = 1;
  }
}

int main() {
  signal(SIGTERM, signal_handler);
  signal(SIGSEGV, signal_handler);
  signal(SIGBUS, signal_handler);
  signal(SIGINT, SIG_IGN);
  
  printf("PID: %d\n", getpid());
  printf("Send SIGTERM to terminate gracefully\n");
  
  while (!shutdown_requested) {
    printf("Working...\n");
    sleep(1);
  }
  
  printf("Shutting down cleanly\n");
  return 0;
}
```

**Task 1.3.2: Process Lock**
```c
#include <fcntl.h>
#include <unistd.h>

int acquire_lock(const char *lock_file) {
  int fd = open(lock_file, O_RDWR | O_CREAT, 0666);
  if (fd < 0) return -1;
  
  struct flock fl;
  memset(&fl, 0, sizeof(fl));
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  
  if (fcntl(fd, F_SETLK, &fl) < 0) {
    if (errno == EACCES || errno == EAGAIN) {
      printf("Lock already held by another process\n");
      close(fd);
      return -2;
    }
    return -1;
  }
  
  // Write PID to lock file
  char buff[32];
  snprintf(buff, sizeof(buff), "%d\n", getpid());
  ftruncate(fd, 0);
  write(fd, buff, strlen(buff));
  
  return fd;
}

int main() {
  int lock_fd = acquire_lock("/tmp/myapp.lock");
  if (lock_fd < 0) exit(1);
  
  printf("Lock acquired, running...\n");
  sleep(5);
  
  close(lock_fd);
  return 0;
}
```

**Task 1.3.3: Test**
```bash
# Terminal 1:
gcc -o signal_demo signal_demo.c -rdynamic
./signal_demo
# Shows: PID: 12345

# Terminal 2:
kill -TERM 12345    # Graceful shutdown
# Terminal 1 should exit cleanly

# Test lock file:
./signal_demo &     # Background
./signal_demo       # Should fail with "Lock already held"
```

---

## Phase 2 Labs: Architecture Layers

### Lab 2.1: TCP Socket Client

**Objective:** Build a non-blocking socket client with reconnection

**Task 2.1.1: Connect & Communicate**
```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <select.h>

int create_nonblock_socket(const char *host, int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) return -1;
  
  // Set non-blocking
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);
  
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_aton(host, &addr.sin_addr);
  
  connect(sock, (struct sockaddr *)&addr, sizeof(addr));
  
  return sock;
}

int read_with_timeout(int sock, char *buf, int len, int timeout_ms) {
  fd_set readset;
  struct timeval tv;
  
  FD_ZERO(&readset);
  FD_SET(sock, &readset);
  
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;
  
  int ret = select(sock + 1, &readset, NULL, NULL, &tv);
  
  if (ret <= 0) return ret;  // Timeout or error
  if (!FD_ISSET(sock, &readset)) return 0;
  
  return read(sock, buf, len);
}

int main() {
  int sock = create_nonblock_socket("localhost", 8080);
  if (sock < 0) {
    printf("Connection failed\n");
    return 1;
  }
  
  // Send length-prefixed message
  char msg[] = "Hello NAPAS!";
  unsigned char len_buf[4];
  len_buf[0] = 0x00;
  len_buf[1] = 0x00;
  len_buf[2] = 0x00;
  len_buf[3] = strlen(msg) + 4;  // Include length header
  
  write(sock, len_buf, 4);
  write(sock, msg, strlen(msg));
  
  // Read response
  char response[1024];
  int n = read_with_timeout(sock, response, sizeof(response), 5000);
  printf("Received %d bytes\n", n);
  
  close(sock);
  return 0;
}
```

**Task 2.1.2: Test Server**
```bash
# Terminal 1: Start echo server
nc -l -p 8080

# Terminal 2: Run client
gcc -o socket_client socket_client.c
./socket_client
# Should send/receive message
```

---

### Lab 2.2: ISO 8583 Message Generator

**Objective:** Generate valid ISO 8583 messages programmatically

**Task 2.2.1: Build Message Structure**
```c
typedef struct {
  unsigned char bitmap[8];
  struct {
    char *data;
    int len;
  } fields[64];
} ISO8583;

int set_field(ISO8583 *msg, int bit_num, const char *data, int len) {
  if (bit_num > 64) return -1;
  
  // Set bit in bitmap
  int byte_idx = (bit_num - 1) / 8;
  int bit_idx = 7 - ((bit_num - 1) % 8);
  msg->bitmap[byte_idx] |= (1 << bit_idx);
  
  // Store field data
  msg->fields[bit_num].data = malloc(len);
  memcpy(msg->fields[bit_num].data, data, len);
  msg->fields[bit_num].len = len;
  
  return 0;
}

int serialize_iso8583(ISO8583 *msg, unsigned char *buf, int max_len) {
  int pos = 0;
  
  // Bitmap
  memcpy(&buf[pos], msg->bitmap, 8);
  pos += 8;
  
  // Fields in order
  for (int bit = 1; bit <= 64; bit++) {
    if (msg->bitmap[(bit-1)/8] & (1 << (7 - ((bit-1)%8)))) {
      if (msg->fields[bit].len > 0) {
        memcpy(&buf[pos], msg->fields[bit].data, msg->fields[bit].len);
        pos += msg->fields[bit].len;
      }
    }
  }
  
  return pos;
}

int main() {
  ISO8583 msg = {0};
  
  // Message Type
  set_field(&msg, 0, "0200", 4);
  
  // PAN
  set_field(&msg, 2, "4111111111111111", 16);
  
  // Processing Code
  set_field(&msg, 3, "000000", 6);
  
  // Amount
  set_field(&msg, 4, "000000100000", 12);
  
  unsigned char buf[1024];
  int len = serialize_iso8583(&msg, buf, sizeof(buf));
  
  printf("Serialized message: %d bytes\n", len);
  for (int i = 0; i < len; i++) {
    printf("%02x ", buf[i]);
  }
  printf("\n");
  
  return 0;
}
```

---

## Phase 3 Labs: Transaction Flow

### Lab 3.1: End-to-End Message Flow

**Objective:** Simulate complete transaction from NAPAS through smartifi to Host I/F

**Task 3.1.1: Create Mock NAPAS Server**
```bash
#!/bin/bash
# mock_napas.sh - Simple echo server returning responses

port=9999

# Create FIFO for communication
mkfifo /tmp/napas_request /tmp/napas_response

# Start nc listener
nc -l -p $port < /tmp/napas_response > /tmp/napas_request &

while true; do
  # Read incoming length
  read -n 4 len
  
  # Convert length bytes to decimal
  decimal=$((16#$(echo $len | od -An -tx1 | tr -d ' ')))
  
  # Read message body
  read -n $decimal msg
  
  echo "Received: $msg" >> /tmp/napas.log
  
  # Send response (0210 for 0200)
  response=$(echo "$msg" | sed 's/0200/0210/')
  
  # Prepend length
  resp_len=$(echo -n "$response" | wc -c)
  printf "\\x00\\x00\\x00\\x$(printf '%02x' $resp_len)" > /tmp/napas_response
  echo "$response" >> /tmp/napas_response
done
```

**Task 3.1.2: Trace with Debugging**
```c
// Add to smartifi.c:
#define TRACE(fmt, ...) \
  fprintf(stderr, "[%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)

// In runBizProc:
TRACE("Received %d bytes from NAPAS", trlen);
TRACE("Message type: %s", msgtype);
TRACE("Processing code: %s", pcode);

// Add instrumentation for latency:
struct timespec start, end;
clock_gettime(CLOCK_MONOTONIC, &start);

// ... processing ...

clock_gettime(CLOCK_MONOTONIC, &end);
long elapsed_ms = (end.tv_sec - start.tv_sec) * 1000 +
                  (end.tv_nsec - start.tv_nsec) / 1000000;
TRACE("Total latency: %ld ms", elapsed_ms);
```

---

### Lab 3.2: Database Transaction Logging

**Objective:** Verify transaction data is correctly logged through all phases

**Task 3.2.1: Create Test Schema**
```sql
CREATE TABLE GAT_SWSNAPRCV_LOG_TEST (
  TRX_ID NUMBER,
  TRX_DT VARCHAR2(8),
  ATM_MSG_T_ID VARCHAR2(4),
  ATM_TRX_AMT NUMBER,
  ATM_PAN_CTT VARCHAR2(20),
  NAPAS_MSG_DTL_CTT CLOB,
  ATM_MSG_DTL_CTT CLOB,
  NORM_PROC_YN NUMBER(1),
  INF_REGIS_DT DATE
);
```

**Task 3.2.2: Verify Transaction Audit Trail**
```bash
#!/bin/bash
# audit_transactions.sh

sqlplus atmdba/password@ORACLE_SID << EOF
SET PAGESIZE 50
SET LINESIZE 200
COLUMN TRX_ID FORMAT 9999999
COLUMN ATM_MSG_T_ID FORMAT A8
COLUMN ATM_TRX_AMT FORMAT 999999999
COLUMN ATM_PAN_CTT FORMAT A20
COLUMN NORM_PROC_YN FORMAT 9

SELECT TRX_ID, TRX_DT, ATM_MSG_T_ID, ATM_TRX_AMT, 
       ATM_PAN_CTT, NORM_PROC_YN, INF_REGIS_DT
FROM GAT_SWSNAPRCV_LOG_TEST
WHERE TRX_DT = TO_CHAR(SYSDATE, 'YYYYMMDD')
ORDER BY INF_REGIS_DT DESC;
EOF
```

---

## Phase 4 Labs: Process Management

### Lab 4.1: Graceful Shutdown Testing

**Task 4.1.1: Test Signal Handling**
```bash
#!/bin/bash
# test_shutdown.sh

# Start smartifi in background
./smartifi &
PID=$!
echo "Started smartifi with PID $PID"

# Let it run
sleep 5

# Send SIGTERM
echo "Sending SIGTERM..."
kill -TERM $PID

# Wait for graceful shutdown
wait $PID
EXIT_CODE=$?

echo "Process exited with code: $EXIT_CODE"
echo "Check logs for 'PROCESS SHUTDOWN COMPLETE'"
tail -20 smartifi.log | grep -i shutdown
```

**Task 4.1.2: Verify Lock File Cleanup**
```bash
# Before start:
ls -la /home/atmadm/.config/smartifi.lock 2>/dev/null || echo "No lock"

# After exit:
# Should show lock removed or new lock on restart
```

---

### Lab 4.2: Database Connection Resilience

**Task 4.2.1: Simulate Connection Loss**
```bash
#!/bin/bash
# test_db_reconnection.sh

# Terminal 1: Start smartifi
./smartifi

# Terminal 2: Monitor Oracle connection
watch -n 1 'sqlplus -s atmdba/password@ORACLE_SID <<< "SELECT COUNT(*) FROM GAT_SWSNAPRCV_LOG WHERE TRX_DT = TO_CHAR(SYSDATE, YYYYMMDD);"'

# Terminal 3: Kill database connection
sqlplus atmdba/password@ORACLE_SID
SQL> SELECT * FROM V$SESSION WHERE USERNAME = 'ATMADM';
SQL> ALTER SYSTEM KILL SESSION 'SID,SERIAL#' IMMEDIATE;

# Watch Terminal 1: Should see "DB Reconnect ERROR" and retry logic
# Watch Terminal 2: Should recover after ~10 seconds
```

---

## Phase 5 Labs: Performance

### Lab 5.1: Load Testing

**Task 5.1.1: Build Message Generator**
```c
// message_generator.c
#include <stdio.h>
#include <time.h>
#include <sys/socket.h>

int generate_iso8583_0200(char *buf, int seq) {
  // Build withdrawal request
  char msg[500];
  sprintf(msg,
    "0200"              // MTI
    "8000000000000000"  // Bitmap (bits 1,2,3,4,7,11,12,39)
    "4111111111111111"  // PAN
    "000000"            // Processing
    "000000100000"      // Amount
    "0622120530"        // DateTime
    "%06d"              // Trace
    "0622"              // Expiry
    "00"                // Response
    , seq);
  
  // Prepend length
  int len = strlen(msg) + 4;
  char *frame = malloc(len + 4);
  frame[0] = 0x00;
  frame[1] = 0x00;
  frame[2] = (len >> 8) & 0xFF;
  frame[3] = len & 0xFF;
  memcpy(&frame[4], msg, len - 4);
  
  memcpy(buf, frame, len + 4);
  free(frame);
  return len + 4;
}

void load_test(int duration_sec, int tps_target) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  // ... connect to smartifi ...
  
  time_t start = time(NULL);
  int seq = 0;
  
  while (time(NULL) - start < duration_sec) {
    char buf[1024];
    int len = generate_iso8583_0200(buf, ++seq);
    
    write(sock, buf, len);
    
    // Rate limiting
    usleep(1000000 / tps_target);
  }
  
  close(sock);
}
```

**Task 5.1.2: Measure Throughput**
```bash
#!/bin/bash
# load_test.sh

TPS_RATES=(10 50 100 200)

for tps in "${TPS_RATES[@]}"; do
  echo "Testing at $tps TPS..."
  
  # Start test
  gcc -o msg_gen message_generator.c
  ./msg_gen &
  GEN_PID=$!
  
  # Measure transactions
  BEFORE=$(sqlplus -s atmdba/password <<< "SELECT COUNT(*) FROM GAT_SWSNAPRCV_LOG WHERE TRX_DT = TO_CHAR(SYSDATE, 'YYYYMMDD');")
  
  sleep 60
  
  AFTER=$(sqlplus -s atmdba/password <<< "SELECT COUNT(*) FROM GAT_SWSNAPRCV_LOG WHERE TRX_DT = TO_CHAR(SYSDATE, 'YYYYMMDD');")
  
  ACTUAL_TPS=$((($AFTER - $BEFORE) / 60))
  
  echo "Target: $tps TPS, Actual: $ACTUAL_TPS TPS"
  
  kill $GEN_PID
done
```

---

### Lab 5.2: Latency Profiling

**Task 5.2.1: Add Timestamp Instrumentation**
```c
// In runBizProc() - add around each major phase:

struct timespec phases[10];
int phase_idx = 0;

clock_gettime(CLOCK_MONOTONIC, &phases[phase_idx++]);

recvFromNAPAS(...)
clock_gettime(CLOCK_MONOTONIC, &phases[phase_idx++]);

Iso8583toOasis(...)
clock_gettime(CLOCK_MONOTONIC, &phases[phase_idx++]);

insertToSWSNAPRCV(...)
clock_gettime(CLOCK_MONOTONIC, &phases[phase_idx++]);

sendToAITHER(...)
clock_gettime(CLOCK_MONOTONIC, &phases[phase_idx++]);

// Print latencies
printf("Socket I/O: %.1f ms\n", 
  (phases[1].tv_sec - phases[0].tv_sec) * 1000 +
  (phases[1].tv_nsec - phases[0].tv_nsec) / 1000000);
printf("ISO Parsing: %.1f ms\n", ...);
printf("DB Insert: %.1f ms\n", ...);
printf("Total: %.1f ms\n", ...);
```

**Task 5.2.2: Profile with perf**
```bash
gcc -O2 -g smartifi.c -o smartifi

# Run with perf
perf record -g ./smartifi

# Analyze
perf report   # Shows which functions use most CPU

# Flamegraph
perf script > out.perf
# Use flamegraph tool for visualization
```

---

## Testing Checklist

```
Phase 1: Foundations
[ ] ISO 8583 parser handles all 64 bits
[ ] Pro*C INSERT/SELECT/CURSOR works
[ ] Signal handlers capture and print stack trace
[ ] Process lock prevents duplicate instances

Phase 2: Architecture Layers
[ ] Socket connects with timeout
[ ] Socket recovers from disconnection
[ ] ISO 8583 message generation is valid
[ ] Database transactions COMMIT/ROLLBACK properly

Phase 3: Transaction Flow
[ ] Incoming 0200 → database log → outbound  
[ ] Outgoing transaction updates NORM_PROC_YN
[ ] Network management (0800/0810) handled
[ ] PIN encryption/decryption via HSM

Phase 4: Process Management
[ ] SIGTERM causes graceful shutdown
[ ] SIGSEGV prints backtrace + shutdown
[ ] DB reconnection works after connection loss
[ ] Log rotation happens daily

Phase 5: Performance
[ ] Latency < 500ms per transaction
[ ] Throughput > 100 TPS sustainable
[ ] Memory stable (no leaks)
[ ] CPU usage reasonable (< 80%)
```

---

## Debugging Tips

### Use strace for System Call Tracing
```bash
strace -e socket,connect,read,write,select -o trace.log ./smartifi
# Shows all network/file operations
```

### Use gdb for Interactive Debugging
```bash
gdb ./smartifi
(gdb) break runBizProc
(gdb) run
(gdb) print msgtype   # Inspect variable
(gdb) next            # Step to next line
(gdb) backtrace       # Print call stack
```

### Monitor Database Activity
```bash
sqlplus atmdba/password@ORACLE_SID
SQL> SELECT PID, OSUSER, PROGRAM FROM V$SESSION WHERE USERNAME = 'ATMDBA';
SQL> SELECT * FROM V$SQL WHERE SQL_TEXT LIKE 'INSERT%NAPRCV%';
```

### Check Network with tcpdump
```bash
tcpdump -i eth0 -w napas.pcap port 9999  # Capture NAPAS traffic
wireshark napas.pcap                      # Analyze in UI
```

---

**Lab Completion Checklist:** Have you completed all labs in each phase? Document your results and create a final integration test report.

