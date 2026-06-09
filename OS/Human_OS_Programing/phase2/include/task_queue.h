/*
 * task_queue.h  —  Phase 2: Priority Queue for Tasks
 *
 * OS Analogy:
 *   The Ready Queue — holds all processes waiting to run on CPU.
 *   In real OS: Linux uses a Red-Black Tree per priority level.
 *   Here: we use a min-heap sorted by urgency score (priority + deadline).
 *
 *   Operations:
 *     enqueue()  → process enters ready queue (after fork/unblock)
 *     dequeue()  → scheduler picks next process (dispatch)
 *     peek()     → look at highest-priority without removing
 */

#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <stdint.h>
#include <time.h>

/* Re-use task structures from Phase 1 */
#define MAX_TASK_NAME     64
#define MAX_TASK_DESC     256
#define MAX_TASK_CATEGORY 32
#define MAX_QUEUE_SIZE    100
#define MAX_DEPENDENCIES  10

typedef enum {
    TASK_PENDING = 0, TASK_RUNNING, TASK_BLOCKED,
    TASK_COMPLETED, TASK_FAILED, TASK_PAUSED
} TaskStatus;

typedef enum {
    PRIORITY_CRITICAL = 0, PRIORITY_HIGH, PRIORITY_MEDIUM,
    PRIORITY_LOW, PRIORITY_OPTIONAL
} TaskPriority;

typedef enum {
    ENERGY_MENTAL = 0, ENERGY_PHYSICAL, ENERGY_EMOTIONAL, ENERGY_MIXED
} EnergyType;

/* Task struct (self-contained for phase 2) */
typedef struct {
    uint32_t     id;
    char         name[MAX_TASK_NAME];
    TaskPriority priority;
    TaskStatus   status;
    EnergyType   energy_type;
    float        duration_minutes;
    float        energy_required;
    float        value_score;
    time_t       created_at;
    time_t       deadline;
    float        urgency_score;   /* pre-computed, updated on enqueue */
} QTask;

/* ─────────────────────────────────────────────────────────────
 *  PRIORITY QUEUE — Min-Heap on urgency (highest urgency = lowest key)
 *  OS: This is the CPU ready queue.
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    QTask items[MAX_QUEUE_SIZE];
    int   size;
    int   total_enqueued;   /* lifetime enqueue count */
    int   total_dequeued;   /* lifetime dequeue count */
} TaskQueue;

/* ─────────────────────────────────────────────────────────────
 *  FUNCTIONS
 * ──────────────────────────────────────────────────────────── */
void    tq_init(TaskQueue *q);
int     tq_enqueue(TaskQueue *q, QTask *task);
QTask*  tq_dequeue(TaskQueue *q, QTask *out);   /* removes and returns highest priority */
QTask*  tq_peek(TaskQueue *q);                  /* look without removing */
int     tq_is_empty(const TaskQueue *q);
int     tq_size(const TaskQueue *q);
void    tq_print(const TaskQueue *q);

/* Utility: compute urgency score for a task */
float   tq_urgency(const QTask *t);

/* Populate a queue with sample tasks for demos */
void    tq_populate_sample(TaskQueue *q);

#endif /* TASK_QUEUE_H */
