# Smartifi - Visual Architecture Guide

## System Components Diagram

```
┌──────────────────────────────────────────────────────────────────┐
│                      EXTERNAL SYSTEMS                             │
├──────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ATM Terminal          NAPAS DP              Oracle DB   HSM     │
│  (Card Input)         (Switching)          (Persistence) (Crypto)│
│     │                    │                      │          │     │
│     └────────┬───────────┤                      │          │     │
│              │           │                      │          │     │
└──────────────┼───────────┼──────────────────────┼──────────┼──────┘
               │           │                      │          │
               │    TCP    │                      │          │
               │   9999    │  Pro*C SQL           │ Socket   │
               │           │  (:atmdba)           │          │
               │           │                      │          │
┌──────────────┼───────────┼──────────────────────┼──────────┼──────┐
│              ↓           ↓                      ↓          ↓      │
│         ┌──────────────────────────────────────────────────────┐ │
│         │                  smartifi                            │ │
│         │         (DP Interface Daemon)                        │ │
│         │                                                      │ │
│         │  ┌────────────────────────────────────────────────┐ │ │
│         │  │ Process Management Layer                       │ │ │
│         │  │ • initSignal() - Signal handlers               │ │ │
│         │  │ • check_running() - Process lock               │ │ │
│         │  │ • stopProc() - Graceful shutdown               │ │ │
│         │  └────────────────────────────────────────────────┘ │ │
│         │                       ↑                              │ │
│         │  ┌────────────────────────────────────────────────┐ │ │
│         │  │ Main Event Loop (select-based)                 │ │ │
│         │  │ • recvFromNAPAS() - Read incoming              │ │ │
│         │  │ • runBizProc() - Process transaction           │ │ │
│         │  │ • recvFromAITHER() - Check Host I/F queue      │ │ │
│         │  │ • sendToNAPAS() - Write response               │ │ │
│         │  └────────────────────────────────────────────────┘ │ │
│         │                       ↑                              │ │
│         │  ┌────────────────────────────────────────────────┐ │ │
│         │  │ Message Transformation Layer                   │ │ │
│         │  │ • Iso8583toOasis() - Parse incoming (bitmap)   │ │ │
│         │  │ • OasistoIso8583() - Generate outgoing (bitmap)│ │ │
│         │  │ • makeHostSendMsg() - Format for Host I/F      │ │ │
│         │  └────────────────────────────────────────────────┘ │ │
│         │                       ↑                              │ │
│         │  ┌────────────────────────────────────────────────┐ │ │
│         │  │ Cryptography & Security Layer                  │ │ │
│         │  │ • hsmManagement() - HSM communication          │ │ │
│         │  │ • PIN decryption/encryption via HSM            │ │ │
│         │  │ • Key management (ZPK, LMK, ZAK)               │ │ │
│         │  └────────────────────────────────────────────────┘ │ │
│         │                       ↑                              │ │
│         │  ┌────────────────────────────────────────────────┐ │ │
│         │  │ Database Persistence Layer                     │ │ │
│         │  │ • insertToSWSNAPRCV() - Log incoming TX        │ │ │
│         │  │ • insertToSWSANYSND() - Queue for Host I/F    │ │ │
│         │  │ • loadHostInfo() - Get NAPAS config            │ │ │
│         │  │ • clearBeforeData() - Purge old records        │ │ │
│         │  └────────────────────────────────────────────────┘ │ │
│         │                       ↑                              │ │
│         │  ┌────────────────────────────────────────────────┐ │ │
│         │  │ Network Communication Layer                    │ │ │
│         │  │ • hostConnect() - TCP to NAPAS                 │ │ │
│         │  │ • readSocket()/writeSocket() - Non-blocking    │ │ │
│         │  │ • recvFromNAPAS() - Length-prefixed messages   │ │ │
│         │  │ • sendToNAPAS() - TCP write response           │ │ │
│         │  └────────────────────────────────────────────────┘ │ │
│         │                                                      │ │
│         └──────────────────────────────────────────────────────┘ │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

---

## Transaction Processing Flow (Incoming)

```
START
  │
  ├─→ main() - Initialize
  │   ├─ initSignal()      [Register SIGTERM, SIGSEGV handlers]
  │   ├─ initLogging()     [Create log file]
  │   ├─ check_running()   [Acquire lock]
  │   ├─ ora_db_connect()  [Connect to Oracle with 100-retry]
  │   ├─ loadHostInfo()    [Fetch NAPAS IP:port from DB]
  │   └─ hostConnect()     [Establish TCP to NAPAS]
  │
  ├─→ Main Event Loop (infinite)
  │   │
  │   ├─→ select() - Wait for data on sock_fd
  │   │   │ (timeout: NH_USLEEP_INTVAL = 300ms)
  │   │   │
  │   │   ├─→ If no data: Increment ping counter
  │   │   │   │ If timeout > NH_BIZ_TIMEOUT: Send ping
  │   │   │
  │   │   └─→ If readable: Continue to runBizProc()
  │   │
  │   └─→ runBizProc(sock_fd)
  │       │
  │       ├─→ recvFromNAPAS()
  │       │   │ Read 4-byte length header
  │       │   ├─ readSocket(sock, buf, 4)
  │       │   │ Convert: hex bytes → decimal length
  │       │   │
  │       │   │ Read message body
  │       │   └─ readSocket(sock, buf+4, length)
  │       │
  │       ├─→ Check Message Type (bit 0)
  │       │   │
  │       │   ├─ If "0800" or "0810"
  │       │   │  └─ recvNetworkMgmt() → sendNetworkRspNAPAS()
  │       │   │     [Sign-on/key-exchange/echo]
  │       │   │     RETURN
  │       │   │
  │       │   └─ If "0200" or "0420" [Financial]
  │       │      └─ Continue...
  │       │
  │       ├─→ Iso8583toOasis() - Parse ISO 8583
  │       │   │ Extract bitmap (8 or 16 bytes)
  │       │   │ For each set bit: Extract field
  │       │   │
  │       │   ├─ Bit 0: MTI (Message Type)
  │       │   ├─ Bit 2: PAN (Card Number)
  │       │   ├─ Bit 3: Processing Code
  │       │   ├─ Bit 4: Amount
  │       │   ├─ Bit 7: DateTime
  │       │   ├─ Bit 11: Trace Number
  │       │   ├─ Bit 39: Response Code
  │       │   ├─ Bit 52: PIN Block [Encrypted]
  │       │   └─ ... (more fields)
  │       │
  │       │   Store in MsgData structure
  │       │
  │       ├─→ HSM PIN Decryption (if Bit 52 present)
  │       │   │ hsmManagement()
  │       │   ├─ Connect to HSM (Unix socket or TCP)
  │       │   ├─ Send command (NG/JE/SMART decrypt)
  │       │   ├─ Receive response
  │       │   └─ Store decrypted PIN
  │       │
  │       ├─→ makeHostSendMsg() - Format for Host I/F
  │       │   │ Build ChnlHead (channel header)
  │       │   ├─ trxbrno (branch number)
  │       │   ├─ trx_trmno (terminal number)
  │       │   ├─ reference number
  │       │   │
  │       │   │ Build MsgHead
  │       │   ├─ glob_id (global transaction ID)
  │       │   ├─ step (sequence number)
  │       │   │
  │       │   │ Prepare trdat (transaction data)
  │       │   └─ ChnlHead + MsgHead + MsgFix + MsgData
  │       │
  │       ├─→ insertToSWSNAPRCV() - Log to DB
  │       │   │ Extract 40+ fields from MsgData
  │       │   ├─ trx_brno, atm_trx_tmn_no, atm_trx_trace_no
  │       │   ├─ glb_id (glob_id + step)
  │       │   ├─ atm_msg_t_id (message type)
  │       │   ├─ atm_trx_c (processing code)
  │       │   ├─ atm_pan_ctt (PAN)
  │       │   ├─ atm_trx_amt (amount)
  │       │   ├─ napas_msg_dtl_ctt (original ISO message)
  │       │   ├─ atm_msg_dtl_ctt (converted OASIS data)
  │       │   │
  │       │   │ EXEC SQL INSERT INTO GAT_SWSNAPRCV_LOG
  │       │   ├─ WITH 40+ columns + sequence
  │       │   ├─ NORM_PROC_YN = 0 (unprocessed)
  │       │   │
  │       │   ├─ IF SQL_ERROR: ROLLBACK, return ABNORMAL
  │       │   └─ ELSE: COMMIT
  │       │
  │       ├─→ sendToAITHER() - Queue to Host I/F
  │       │   │ insertToSWSANYSND() with same data
  │       │   │
  │       │   └─ INSERT INTO GAT_SWSANYSND_LOG
  │       │      ├─ NORM_PROC_YN = 0 (unprocessed)
  │       │      ├─ ATM_PROC_TMN_D = 'g' (Host I/F)
  │       │      └─ COMMIT
  │       │
  │       └─→ RETURN to event loop
  │
  │
  ├─→ (Parallel) recvFromAITHER()
  │   │ Check Host I/F outbound queue periodically
  │   │
  │   ├─ OPEN CURSOR on GAT_SWSNAPSND_LOG
  │   │   WHERE NAPAS_PROC_ID = 'IBT'
  │   │   AND NORM_PROC_YN = 0     [Unprocessed]
  │   │
  │   ├─ FOR EACH pending transaction
  │   │  │
  │   │  ├─ Fetch TRX_DT, ATM_LGGTM
  │   │  │
  │   │  ├─ SELECT full message from DB
  │   │  │
  │   │  ├─ Parse into ChnlHead, MsgHead, MsgData
  │   │  │
  │   │  ├─ OasistoIso8583() - Convert to ISO 8583
  │   │  │   └─ Rebuild bitmap for response fields
  │   │  │
  │   │  ├─ UPDATE GAT_SWSNAPSND_LOG
  │   │  │   └─ NORM_PROC_YN = 1 (processed)
  │   │  │
  │   │  └─ sendToNAPAS()
  │   │      ├─ Prepend 4-byte length header
  │   │      ├─ write(sock_fd, message)
  │   │      └─ Validate written length
  │   │
  │   └─ CLOSE CURSOR
  │
  │
  └─→ Signal Handler (SIGTERM)
      │
      └─→ stopProc()
          ├─ EXEC SQL ROLLBACK WORK RELEASE  [DB cleanup]
          ├─ shutdown(sock_fd, SHUT_RDWR)   [Socket close]
          ├─ close(lock_fd)                  [Release lock]
          ├─ fclose(logfp)                   [Flush logs]
          └─ exit(0)

END
```

---

## ISO 8583 Message Structure

```
Incoming Message (Binary)
┌──────────────┬──────────────┬─────────────────────────────────┐
│  4 bytes     │  8-16 bytes  │  Variable length data           │
│  Length      │  Bitmap      │  Field data (bits 1-128)        │
└──────────────┴──────────────┴─────────────────────────────────┘
     │              │                    │
     │              │                    └─→ Bits 1-64 present
     │              │                        (8-byte bitmap)
     │              │
     │              └─→ Bitmap bits set (1=present, 0=absent)
     │                  Bit layout: [Bit0] [Bit1-8]
     │                               [Byte0] [Byte1]
     │                               Byte0 bit 7 = Bit 1
     │                               Byte0 bit 0 = Bit 8
     │
     └─→ Includes 4-byte header + field data
         Example: 00 50 = 80 bytes total

Bitmap Bit Assignments (Key Fields)
Bit 0:   MTI (Message Type Indicator) = "0200" (4 bytes)
Bit 2:   PAN (Card Number) = "4111111111111111" (var)
Bit 3:   Processing Code = "000000" (6 bytes)
Bit 4:   Amount = "000000100000" (12 bytes)
Bit 7:   DateTime = "0622120530" (10 bytes)
Bit 11:  Trace Number = "000001" (6 bytes)
Bit 12:  Expiry = "0622" (4 bytes)
Bit 39:  Response = "00" (2 bytes)
Bit 41:  Terminal ID = "12345678" (8 bytes)
Bit 52:  PIN Block = [encrypted binary] (var)
...more fields...

Bitmap Byte Representation
     Bit  1    2    3    4    5    6    7    8
      │    │    │    │    │    │    │    │
Byte0 [PAN][PCod][Amou][Date][STAN][Expy][Resp][bit9]
Byte1 [Bit9][Bit10][Bit11]...[Bit16]
...
```

---

## Data Structure Hierarchy (Memory Layout)

```
Message Buffer (incoming from NAPAS):
┌─────────────────────────────────────────┐
│ [0-3]  Length (4 bytes)                 │
│ [4-7]  MTI = "0200" (4 bytes)           │
│ [8-15] Bitmap (8 bytes)                 │
│ [16-] Field data (variable)             │
│   └─ PAN (19 bytes max)                 │
│   └─ Processing Code (6 bytes)          │
│   └─ Amount (12 bytes)                  │
│   └─ ... more fields ...                │
└─────────────────────────────────────────┘
          │
          ├─→ Stored in: RecvBuffer[]
          │
          ├─→ Copy to: napas_data[]
          │
          └─→ Parse into:
              ├─ ChnlHead struct
              ├─ MsgHead struct
              ├─ MsgFix struct
              └─ MsgData struct
                  ├─ msgtype (bit 0)
                  ├─ bit_2 (PAN)
                  ├─ bit_3 (Processing)
                  ├─ bit_4 (Amount)
                  ├─ ... 128 fields ...
                  └─ atm_ord_ctt (order data)

Database Record (Insert):
┌────────────────────────────────┐
│ GAT_SWSNAPRCV_LOG              │
├────────────────────────────────┤
│ SH_LCL_CORP_C = 130            │ (Company ID)
│ TRX_DT = '20260622'            │ (Date)
│ ATM_LGGTM = '1205301234567'    │ (Timestamp)
│ NAPAS_PROC_ID = 'IBT'          │ (Process)
│ NORM_PROC_YN = 0               │ (Status: unprocessed)
│ ATM_TRX_TRACE_NO = '000001'    │ (Trace)
│ GLB_ID = 'ABC123...'           │ (Global ID + step)
│ ATM_MSG_T_ID = '0200'          │ (Message type)
│ ATM_TRX_C = '000000'           │ (Processing code)
│ ATM_TRX_AMT = 100000           │ (Amount)
│ ATM_PAN_CTT = '4111...1111'    │ (PAN)
│ ... 30+ more fields ...        │
│ NAPAS_MSG_DTL_CTT = [binary]   │ (Original ISO message)
│ ATM_MSG_DTL_CTT = [binary]     │ (Converted OASIS data)
│ INF_REGIS_DT = SYSDATE         │ (Creation date)
│ INF_REGIS_TIME = '120530'      │ (Creation time)
└────────────────────────────────┘
```

---

## Error Handling & Signal Flow

```
┌─────────────────────────────┐
│   Process Execution         │
└────────────┬────────────────┘
             │
        ┌────┴────┬─────────┬─────────┐
        │          │         │         │
        ↓          ↓         ↓         ↓
    Normal      Timeout   Network   Data
   Processing   Expired   Error    Corrupt
        │          │        │         │
        └──────┬───┴───┬────┴────┬────┘
               │       │         │
               ↓       ↓         ↓
          Recoverable  Retryable Fatal
               │       │         │
               ↓       │         ↓
        Continue?   Reconnect?  Stack Trace?
        │           │           │
        YES         YES        PRINT
        │           │           │
        ├─→ Return  ├─→ LoadInfo ├─→ Backtrace
        │   NORMAL  │   hostConnect() │   fatalSignalHandler()
        │           │   → Return to   │   → stopProc_sig()
        │           │     event loop  │
        │           │                 │
        └───────────┴────────┬────────┘
                             │
                             ↓
                    ┌────────────────┐
                    │ stopProc()     │
                    │ • ROLLBACK TX  │
                    │ • Close socket │
                    │ • Release lock │
                    │ • Flush logs   │
                    │ • exit(0)      │
                    └────────────────┘

Signal Handlers Registered:
┌──────────────┬──────────────┬──────────────┐
│  Fatal       │  Graceful    │  Ignored     │
├──────────────┼──────────────┼──────────────┤
│ SIGSEGV      │ SIGTERM      │ SIGHUP       │
│ SIGBUS       │ SIGKILL      │ SIGINT       │
│ SIGABRT      │ SIGSTOP      │ SIGPIPE      │
│ SIGILL       │              │ SIGCHLD      │
│ SIGFPE       │              │ SIGIO        │
│              │              │ SIGTSTP      │
│              │              │ SIGCONT      │
└──────────────┴──────────────┴──────────────┘
     ↓                ↓               ↓
  Print stack    Graceful        No action
  + shutdown    shutdown
```

---

## Performance & Timing Analysis

```
Transaction Latency Breakdown
┌──────────────────────────────────────────────────────────┐
│ Socket I/O (recvFromNAPAS)        ~50-200 ms            │
│ │  ├─ DNS resolution (if needed):     0-50 ms          │
│ │  ├─ TCP connect (if needed):        10-100 ms        │
│ │  └─ Read 4-byte header + payload:   10-50 ms         │
│ │                                                        │
│ ISO 8583 Parsing (Iso8583toOasis)  ~10-30 ms           │
│ │  ├─ Bitmap extraction:              1-5 ms           │
│ │  ├─ Field parsing loop:             5-10 ms          │
│ │  └─ PIN decryption (HSM):           3-15 ms          │
│ │                                                        │
│ Database Insert (insertToSWSNAPRCV) ~50-200 ms         │
│ │  ├─ SQL prepare:                    1-5 ms           │
│ │  ├─ Field binding:                  5-10 ms          │
│ │  ├─ Execute INSERT:                 20-100 ms        │
│ │  └─ COMMIT:                         20-80 ms         │
│ │                                                        │
│ Socket Write (sendToAITHER)         ~10-50 ms          │
│ │  └─ Message queue insertion         10-50 ms         │
│ │                                                        │
│ ─────────────────────────────────────────────────────   │
│ TOTAL (Typical)                     ~150-500 ms        │
│ TOTAL (Worst case)                  ~500ms-1s          │
└──────────────────────────────────────────────────────────┘

Throughput vs. Latency:
┌─────────────────────────────────────────┐
│ 500 ms latency → 2 TPS (serial)        │
│ 250 ms latency → 4 TPS (serial)        │
│ 200 ms latency → 5 TPS (serial)        │
│ 100 ms latency → 10 TPS (serial)       │
│                                        │
│ With N workers (parallel):             │
│ N = 10: 10 × TPS_per_worker possible  │
│         but DB becomes bottleneck      │
└─────────────────────────────────────────┘

Resource Utilization:
┌─────────────────────────────────────────┐
│ CPU:       ~20-30% (I/O bound)         │
│ Memory:    ~50-100 MB (buffers)        │
│ Network:   ~10-50 Mbps (typical)       │
│ DB Conn:   1 (connection pooling)      │
│ File Desc: 3 (socket, log, lock)       │
└─────────────────────────────────────────┘
```

---

## Configuration & Startup Sequence

```
Configuration Sources (Priority):
1. Command Line Arguments
   └─ ./smartifi [server_gb]
      where server_gb = 'Z' (default) or other code

2. Environment Variables
   └─ ORACLE_HOME, LD_LIBRARY_PATH, etc.

3. Database Configuration Table (GAT_SWSCONF_INF)
   ├─ DP_IBT_I_V2_ADDR0:    NAPAS IP address
   ├─ DP_IBT_I_V2_PORT0:    NAPAS port
   ├─ HSM_HOSTADDR1/3:      HSM IP addresses
   ├─ HSM_HOSTPORT1/3:      HSM ports
   ├─ DP_V2_ZAKKEY:         NAPAS encryption key
   ├─ DP_CLEARKEY:          PIN block DES key
   └─ LOGLVL_smartifi:      Log level

Startup Sequence:
┌─────────────────────────────────────┐
│ 1. Check process lock               │
│    └─→ F_WRLCK on /.../.lock       │
│        Return if already running    │
│                                     │
│ 2. Setup signals                    │
│    └─→ Register handlers            │
│        SIGTERM, SIGSEGV, etc.      │
│                                     │
│ 3. Init logging                     │
│    └─→ Create log file              │
│        with daily rotation          │
│                                     │
│ 4. DB connection                    │
│    └─→ Connect to Oracle            │
│        with 100 retries             │
│        (1 sec between attempts)     │
│                                     │
│ 5. Load configuration               │
│    ├─ NAPAS address:port           │
│    ├─ HSM address:port             │
│    ├─ Encryption keys              │
│    └─ Log level                    │
│                                     │
│ 6. Establish NAPAS connection      │
│    └─→ TCP connect with 10s timeout│
│        and SIGALRM                 │
│                                     │
│ 7. Enter event loop                 │
│    └─→ select() wait for messages  │
└─────────────────────────────────────┘
```

---

These diagrams complement the detailed documentation. Refer to them while reading the learning path for visual reinforcement of concepts.

