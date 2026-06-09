/*
 * scheduler.h  —  Phase 2: Task Scheduling Algorithms
 *
 * OS Analogy:
 *   This is the CPU Scheduler — the most critical kernel component.
 *   It decides which process runs next and for how long.
 *
 *   Real OS algorithms implemented here:
 *     FIFO (First-In-First-Out)   → like batch OS scheduling
 *     Priority Scheduling         → like UNIX nice / real-time priority
 *     Round Robin                 → like Linux CFS (Completely Fair Scheduler)
 *     Shortest Job First (SJF)    → minimize average waiting time
 *     EDF (Earliest Deadline First) → real-time scheduling
 *
 *   Human OS equivalent:
 *     FIFO      → do tasks in the order they were created
 *     Priority  → do critical tasks first (Eisenhower Matrix)
 *     RR        → spend equal time on each task (balanced progress)
 *     SJF       → do quick wins first (getting things done)
 *     EDF       → meet deadlines first (reactive mode)
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task_queue.h"
#include <stdint.h>
#include <time.h>

/* ─────────────────────────────────────────────────────────────
 *  SCHEDULING ALGORITHM SELECTION
 * ──────────────────────────────────────────────────────────── */
typedef enum {
    SCHED_FIFO     = 0,   /* First-In-First-Out: simple queue            */
    SCHED_PRIORITY = 1,   /* Priority: highest priority runs first       */
    SCHED_RR       = 2,   /* Round Robin: each task gets a time slice    */
    SCHED_SJF      = 3,   /* Shortest Job First: minimize wait time      */
    SCHED_EDF      = 4,   /* Earliest Deadline First: deadline-driven    */
} SchedAlgorithm;

/* ─────────────────────────────────────────────────────────────
 *  TIME SLOT — represents a scheduled block
 *  OS: like a CPU time quantum in round-robin scheduling
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    uint32_t task_id;
    time_t   start_time;       /* when this slot begins         */
    float    duration_minutes; /* length of this slot           */
    int      is_break;         /* 1 = recovery slot (no task)   */
    char     label[32];        /* display label                 */
} TimeSlot;

/* ─────────────────────────────────────────────────────────────
 *  SCHEDULE — the output of scheduling: ordered list of slots
 *  OS: this is the "dispatch queue" ready for the CPU
 * ──────────────────────────────────────────────────────────── */
#define MAX_TIME_SLOTS 50

typedef struct {
    TimeSlot     slots[MAX_TIME_SLOTS];
    int          slot_count;
    float        total_duration_minutes;  /* total scheduled time    */
    float        total_break_minutes;     /* total recovery time     */
    SchedAlgorithm algorithm;             /* which algorithm was used*/
    float        context_switch_cost;    /* OS: context switch time */
} Schedule;

/* ─────────────────────────────────────────────────────────────
 *  SCHEDULER METRICS — performance analysis
 *  OS: scheduling metrics (turnaround, waiting, response time)
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    float avg_waiting_time;      /* average time tasks wait before running  */
    float avg_turnaround_time;   /* average time from create to complete     */
    float cpu_utilization;       /* % of time doing real work (not breaks)  */
    float throughput;            /* tasks completed per hour                 */
    int   context_switches;     /* how many task switches occurred          */
} SchedMetrics;

/* ─────────────────────────────────────────────────────────────
 *  FUNCTION DECLARATIONS
 * ──────────────────────────────────────────────────────────── */
void   schedule_init(Schedule *s, SchedAlgorithm algo);
int    schedule_build(Schedule *s, TaskQueue *q, float available_hours);

/* Individual algorithm implementations */
int    sched_fifo    (Schedule *s, TaskQueue *q, float hours);
int    sched_priority(Schedule *s, TaskQueue *q, float hours);
int    sched_rr      (Schedule *s, TaskQueue *q, float hours, float quantum);
int    sched_sjf     (Schedule *s, TaskQueue *q, float hours);
int    sched_edf     (Schedule *s, TaskQueue *q, float hours);

/* Metrics and display */
SchedMetrics schedule_compute_metrics(const Schedule *s);
void   schedule_print(const Schedule *s);
void   schedule_compare(TaskQueue *q, float hours);
const char* sched_algorithm_name(SchedAlgorithm a);

#endif /* SCHEDULER_H */
