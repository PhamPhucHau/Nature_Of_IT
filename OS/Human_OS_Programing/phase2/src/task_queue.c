/*
 * task_queue.c  —  Phase 2: Priority Queue Implementation
 *
 * Implements a MAX-HEAP based priority queue.
 * Highest urgency score = dequeued first.
 *
 * OS Analogy: Linux CFS (Completely Fair Scheduler) uses a
 * Red-Black Tree keyed on virtual runtime. Here we use a simpler
 * binary heap — same concept, different data structure.
 *
 * Heap invariant: parent.urgency >= child.urgency
 * Insertion: O(log n)  — bubble up
 * Removal:   O(log n)  — bubble down
 * Peek:      O(1)      — heap[0] is always max
 */

#include "../include/task_queue.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ─────────────────────────────────────────────────────────────
 *  tq_urgency() — compute urgency score for sorting
 *  Higher urgency = scheduled sooner.
 *
 *  OS: This is like the priority key used by the scheduler.
 *  In Linux CFS: vruntime (lower = sooner).
 *  Here: higher number = sooner (max-heap).
 * ──────────────────────────────────────────────────────────── */
float tq_urgency(const QTask *t) {
    if (!t) return 0.0f;

    /* Base score: CRITICAL=100, HIGH=75, MEDIUM=50, LOW=25, OPTIONAL=10 */
    float base_scores[] = {100.0f, 75.0f, 50.0f, 25.0f, 10.0f};
    float score = base_scores[(int)t->priority < 5 ? (int)t->priority : 4];

    /* Deadline pressure bonus */
    if (t->deadline > 0) {
        double secs_left = difftime(t->deadline, time(NULL));
        if      (secs_left <= 0)      score += 50.0f;  /* overdue!       */
        else if (secs_left < 3600)    score += 40.0f;  /* < 1 hour       */
        else if (secs_left < 86400)   score += 20.0f;  /* < 1 day        */
        else if (secs_left < 604800)  score += 10.0f;  /* < 1 week       */
    }

    /* Value bonus: high-value tasks get slight priority boost */
    score += t->value_score * 0.5f;

    return score;
}

/* ─────────────────────────────────────────────────────────────
 *  Heap helpers: swap, bubble-up, bubble-down
 * ──────────────────────────────────────────────────────────── */
static void swap_tasks(QTask *a, QTask *b) {
    QTask tmp = *a;
    *a = *b;
    *b = tmp;
}

/* Bubble up: after inserting at end, restore heap property upward */
static void heap_bubble_up(TaskQueue *q, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (q->items[parent].urgency_score < q->items[idx].urgency_score) {
            swap_tasks(&q->items[parent], &q->items[idx]);
            idx = parent;
        } else {
            break;
        }
    }
}

/* Bubble down: after removing root, restore heap property downward */
static void heap_bubble_down(TaskQueue *q, int idx) {
    int size = q->size;
    while (1) {
        int left  = 2 * idx + 1;
        int right = 2 * idx + 2;
        int largest = idx;

        if (left  < size && q->items[left].urgency_score  > q->items[largest].urgency_score)
            largest = left;
        if (right < size && q->items[right].urgency_score > q->items[largest].urgency_score)
            largest = right;

        if (largest != idx) {
            swap_tasks(&q->items[largest], &q->items[idx]);
            idx = largest;
        } else {
            break;
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 *  Public API
 * ──────────────────────────────────────────────────────────── */
void tq_init(TaskQueue *q) {
    memset(q, 0, sizeof(TaskQueue));
    printf("[TaskQueue] Initialized (capacity=%d)\n", MAX_QUEUE_SIZE);
}

/* tq_enqueue() — add task to ready queue
 * OS: process moves from BLOCKED/NEW → READY, enters ready queue */
int tq_enqueue(TaskQueue *q, QTask *task) {
    if (!q || !task || q->size >= MAX_QUEUE_SIZE) return -1;

    /* Compute urgency before inserting */
    task->urgency_score = tq_urgency(task);

    /* Insert at end of heap */
    q->items[q->size] = *task;
    q->size++;
    q->total_enqueued++;

    /* Restore heap property: bubble up */
    heap_bubble_up(q, q->size - 1);
    return 0;
}

/* tq_dequeue() — remove and return highest-priority task
 * OS: scheduler dispatches next process to CPU */
QTask* tq_dequeue(TaskQueue *q, QTask *out) {
    if (!q || q->size == 0) return NULL;

    /* Root is always the highest urgency task */
    *out = q->items[0];

    /* Replace root with last element, shrink heap */
    q->items[0] = q->items[q->size - 1];
    q->size--;
    q->total_dequeued++;

    /* Restore heap property: bubble down from root */
    if (q->size > 0) heap_bubble_down(q, 0);

    return out;
}

/* tq_peek() — look at highest priority without removing
 * OS: scheduler checking next process without dispatching */
QTask* tq_peek(TaskQueue *q) {
    if (!q || q->size == 0) return NULL;
    return &q->items[0];  /* root = highest urgency */
}

int tq_is_empty(const TaskQueue *q) { return !q || q->size == 0; }
int tq_size(const TaskQueue *q)     { return q ? q->size : 0; }

void tq_print(const TaskQueue *q) {
    if (!q || q->size == 0) { printf("  (queue empty)\n"); return; }
    printf("  Priority Queue (%d tasks, sorted by urgency):\n", q->size);
    printf("  %-4s  %-25s  %-10s  %8s  %6s\n",
           "ID", "NAME", "PRIORITY", "URGENCY", "MINS");
    for (int i = 0; i < q->size; i++) {
        /* Show priority label */
        const char *pnames[] = {"CRITICAL","HIGH","MEDIUM","LOW","OPTIONAL"};
        const char *pname = pnames[(int)q->items[i].priority < 5 ? (int)q->items[i].priority : 4];
        printf("  %-4u  %-25s  %-10s  %8.1f  %6.0f\n",
               q->items[i].id,
               q->items[i].name,
               pname,
               q->items[i].urgency_score,
               q->items[i].duration_minutes);
    }
}

/* Populate queue with representative sample tasks for demos */
void tq_populate_sample(TaskQueue *q) {
    static uint32_t next_id = 1;
    time_t now = time(NULL);

    struct { const char *name; TaskPriority p; float dur; float val; int deadline_h; } samples[] = {
        { "Team standup",          PRIORITY_CRITICAL,  15.0f, 8.0f,  1  },
        { "Write project report",  PRIORITY_HIGH,      90.0f, 9.0f,  8  },
        { "Exercise 30 min",       PRIORITY_HIGH,      30.0f, 8.5f,  0  },
        { "Code review PRs",       PRIORITY_MEDIUM,    45.0f, 7.0f,  24 },
        { "Deep work: algorithm",  PRIORITY_HIGH,     120.0f, 9.5f,  0  },
        { "Read research paper",   PRIORITY_MEDIUM,    40.0f, 7.5f,  0  },
        { "Reply to emails",       PRIORITY_LOW,       20.0f, 4.0f,  0  },
        { "Learning: new skill",   PRIORITY_MEDIUM,    60.0f, 8.0f,  0  },
        { "1:1 with manager",      PRIORITY_HIGH,      30.0f, 8.0f,  4  },
        { "Personal admin tasks",  PRIORITY_LOW,       30.0f, 3.5f,  0  },
    };

    for (int i = 0; i < 10; i++) {
        QTask t = {0};
        t.id               = next_id++;
        t.priority         = samples[i].p;
        t.status           = TASK_PENDING;
        t.energy_type      = ENERGY_MENTAL;
        t.duration_minutes = samples[i].dur;
        t.energy_required  = samples[i].dur * 0.2f;
        t.value_score      = samples[i].val;
        t.created_at       = now;
        t.deadline         = samples[i].deadline_h > 0
                             ? now + samples[i].deadline_h * 3600 : 0;
        strncpy(t.name, samples[i].name, MAX_TASK_NAME - 1);
        tq_enqueue(q, &t);
    }
}
