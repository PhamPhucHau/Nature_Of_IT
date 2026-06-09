/*
 * task.h  —  Phase 1: Task (Process) Data Structures
 *
 * In an OS, a PROCESS is the fundamental unit of execution.
 * In Human OS, a TASK is the fundamental unit of life management.
 *
 * OS Analogy:
 *   OS Process   → Human Task
 *   PID          → task_id
 *   CPU burst    → duration_minutes
 *   Priority     → priority (0=highest, like UNIX nice)
 *   State        → TaskStatus (PENDING, RUNNING, BLOCKED, COMPLETED)
 *   Deadline     → deadline_timestamp
 */

#ifndef TASK_H
#define TASK_H

#include <time.h>
#include <stdint.h>

/* ─────────────────────────────────────────────────────────────
 *  CONSTANTS
 * ──────────────────────────────────────────────────────────── */
#define MAX_TASK_NAME     64
#define MAX_TASK_DESC     256
#define MAX_TASK_CATEGORY 32
#define MAX_TASKS         100
#define MAX_DEPENDENCIES  10

/* ─────────────────────────────────────────────────────────────
 *  TASK STATUS — mirrors OS process states
 *
 *  OS:          NEW → READY → RUNNING → BLOCKED → TERMINATED
 *  Human OS:  PENDING → RUNNING → BLOCKED → COMPLETED / FAILED
 * ──────────────────────────────────────────────────────────── */
typedef enum {
    TASK_PENDING   = 0,   /* created, not started yet (OS: NEW/READY)    */
    TASK_RUNNING   = 1,   /* currently being worked on (OS: RUNNING)     */
    TASK_BLOCKED   = 2,   /* waiting for something (OS: BLOCKED/WAITING) */
    TASK_COMPLETED = 3,   /* done successfully (OS: TERMINATED normal)   */
    TASK_FAILED    = 4,   /* did not complete (OS: TERMINATED abnormal)  */
    TASK_PAUSED    = 5,   /* intentionally paused (OS: SUSPENDED)        */
} TaskStatus;

/* ─────────────────────────────────────────────────────────────
 *  TASK PRIORITY — mirrors OS scheduling priority
 *
 *  Lower number = higher priority (like UNIX nice values).
 *  CRITICAL tasks are scheduled first (like OS real-time processes).
 * ──────────────────────────────────────────────────────────── */
typedef enum {
    PRIORITY_CRITICAL = 0,   /* must do now — like OS interrupt handler  */
    PRIORITY_HIGH     = 1,   /* important, do today                      */
    PRIORITY_MEDIUM   = 2,   /* normal work                              */
    PRIORITY_LOW      = 3,   /* nice to have                             */
    PRIORITY_OPTIONAL = 4,   /* do if time allows                        */
} TaskPriority;

/* ─────────────────────────────────────────────────────────────
 *  ENERGY TYPE — what kind of "CPU" the task uses
 *
 *  OS: tasks consume CPU cycles.
 *  Human OS: tasks consume mental, physical, or emotional energy.
 * ──────────────────────────────────────────────────────────── */
typedef enum {
    ENERGY_MENTAL    = 0,   /* thinking, coding, writing, learning   */
    ENERGY_PHYSICAL  = 1,   /* exercise, movement, manual work       */
    ENERGY_EMOTIONAL = 2,   /* relationships, communication, support */
    ENERGY_MIXED     = 3,   /* combination of above                  */
} EnergyType;

/* ─────────────────────────────────────────────────────────────
 *  TASK STRUCT — the core data structure
 *
 *  OS Process Control Block (PCB) analogy:
 *    PCB field          → Task field
 *    pid                → id
 *    process_name       → name
 *    priority           → priority
 *    state              → status
 *    cpu_time_used      → actual_duration_minutes
 *    time_limit         → duration_minutes
 *    deadline           → deadline
 * ──────────────────────────────────────────────────────────── */
typedef struct Task {
    uint32_t   id;                                  /* unique task ID (like PID)           */
    char       name[MAX_TASK_NAME];                 /* human-readable name                 */
    char       description[MAX_TASK_DESC];          /* what the task involves              */
    char       category[MAX_TASK_CATEGORY];         /* e.g. "work", "health", "learning"   */

    TaskPriority  priority;                         /* scheduling priority                 */
    TaskStatus    status;                           /* current lifecycle state             */
    EnergyType    energy_type;                      /* what kind of energy it uses         */

    float      duration_minutes;                    /* estimated duration (CPU burst)      */
    float      actual_duration_minutes;             /* how long it actually took           */
    float      energy_required;                     /* energy units needed (0.0–100.0)     */
    float      progress_percent;                    /* completion percentage 0–100         */

    time_t     created_at;                          /* when task was created               */
    time_t     started_at;                          /* when execution began                */
    time_t     completed_at;                        /* when execution finished             */
    time_t     deadline;                            /* must finish by this time            */

    uint32_t   dependencies[MAX_DEPENDENCIES];      /* task IDs that must complete first   */
    int        dependency_count;                    /* how many dependencies               */

    int        is_recurring;                        /* 1 = repeats (like a cron job)       */
    float      value_score;                         /* estimated value/ROI (0.0–10.0)      */
} Task;

/* ─────────────────────────────────────────────────────────────
 *  TASK MANAGER — holds all tasks (like OS process table)
 *
 *  OS Process Table: fixed array indexed by PID
 *  Human OS Task Table: dynamic array with metadata
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    Task     tasks[MAX_TASKS];   /* the process table                        */
    int      count;              /* number of tasks currently in the table   */
    uint32_t next_id;            /* PID counter (auto-increment)             */
} TaskManager;

/* ─────────────────────────────────────────────────────────────
 *  FUNCTION DECLARATIONS
 * ──────────────────────────────────────────────────────────── */

/* Lifecycle */
void     task_manager_init(TaskManager *tm);
Task*    task_create(TaskManager *tm, const char *name, const char *desc,
                     TaskPriority priority, float duration_min, float energy);
int      task_start(TaskManager *tm, uint32_t task_id);
int      task_complete(TaskManager *tm, uint32_t task_id);
int      task_fail(TaskManager *tm, uint32_t task_id);
int      task_pause(TaskManager *tm, uint32_t task_id);
int      task_delete(TaskManager *tm, uint32_t task_id);

/* Query */
Task*    task_find_by_id(TaskManager *tm, uint32_t task_id);
int      task_is_overdue(const Task *t);
int      task_can_start(const TaskManager *tm, const Task *t);
float    task_urgency_score(const Task *t);

/* Display */
void     task_print(const Task *t);
void     task_manager_print_all(const TaskManager *tm);
void     task_manager_print_by_priority(const TaskManager *tm);
const char* task_status_name(TaskStatus s);
const char* task_priority_name(TaskPriority p);
const char* task_energy_type_name(EnergyType e);

#endif /* TASK_H */
