/*
 * scheduler.c  —  Phase 2: Scheduling Algorithm Implementations
 *
 * Build:  gcc -Wall -Wextra -o scheduler main.c scheduler.c task_queue.c -lm
 *
 * OS Concepts implemented:
 *
 *  FIFO  — batch scheduling. Process enters → runs to completion.
 *           No preemption. Good when tasks have similar duration.
 *           Problem: "convoy effect" — one long task blocks all others.
 *
 *  Priority — preemptive priority scheduling.
 *             Highest priority always runs first.
 *             Problem: "starvation" — low-priority tasks never run.
 *             Solution: "aging" — boost priority of waiting tasks over time.
 *
 *  Round Robin — each task gets a fixed time quantum (time slice).
 *                Best for time-sharing systems (Linux desktop).
 *                Context switch cost = switching between tasks.
 *
 *  SJF — Shortest Job First. Minimizes average waiting time.
 *         Provably optimal for average wait time.
 *         Problem: need to predict burst time (in practice, unknown).
 *
 *  EDF — Earliest Deadline First. Optimal for real-time systems.
 *         Guarantees all deadlines met if schedulable.
 *         Used in: real-time kernels, multimedia systems.
 */

#include "../include/scheduler.h"
#include "../include/task_queue.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ─────────────────────────────────────────────────────────────
 *  Helpers
 * ──────────────────────────────────────────────────────────── */
const char* sched_algorithm_name(SchedAlgorithm a) {
    switch (a) {
        case SCHED_FIFO:     return "FIFO (First-In-First-Out)";
        case SCHED_PRIORITY: return "Priority Scheduling";
        case SCHED_RR:       return "Round Robin";
        case SCHED_SJF:      return "Shortest Job First";
        case SCHED_EDF:      return "Earliest Deadline First";
        default:             return "Unknown";
    }
}

void schedule_init(Schedule *s, SchedAlgorithm algo) {
    memset(s, 0, sizeof(Schedule));
    s->algorithm          = algo;
    s->context_switch_cost = 2.0f; /* 2 minutes per context switch (human task switching) */
}

/* Add a work slot to the schedule */
static void add_slot(Schedule *s, uint32_t task_id,
                      float duration_min, const char *label) {
    if (s->slot_count >= MAX_TIME_SLOTS) return;

    TimeSlot *slot = &s->slots[s->slot_count++];
    slot->task_id          = task_id;
    slot->duration_minutes = duration_min;
    slot->is_break         = 0;
    strncpy(slot->label, label, 31);
    s->total_duration_minutes += duration_min;
}

/* Add a break/recovery slot */
static void add_break(Schedule *s, float duration_min) {
    if (s->slot_count >= MAX_TIME_SLOTS) return;

    TimeSlot *slot = &s->slots[s->slot_count++];
    slot->task_id          = 0;
    slot->duration_minutes = duration_min;
    slot->is_break         = 1;
    strncpy(slot->label, "BREAK", 31);
    s->total_break_minutes += duration_min;
    s->total_duration_minutes += duration_min;
}

/* Make a temporary copy of the queue for non-destructive iteration */
static void copy_queue(const TaskQueue *src, TaskQueue *dst) {
    *dst = *src;
}

/* ─────────────────────────────────────────────────────────────
 *  FIFO SCHEDULING
 *  Process tasks in creation order (queue order).
 *  OS: like a simple batch system.
 *  Human OS: "work the queue from top to bottom".
 * ──────────────────────────────────────────────────────────── */
int sched_fifo(Schedule *s, TaskQueue *q, float hours) {
    TaskQueue work;
    copy_queue(q, &work);

    float remaining_minutes = hours * 60.0f;
    float elapsed = 0.0f;
    int   tasks_scheduled = 0;

    printf("  [FIFO] Scheduling %d tasks for %.0f hours:\n",
           work.size, hours);

    /* Process tasks in order — no sorting, just dequeue (heap order ~= creation order) */
    while (!tq_is_empty(&work) && remaining_minutes > 0) {
        QTask task;
        if (!tq_dequeue(&work, &task)) break;

        float duration = fminf(task.duration_minutes, remaining_minutes);

        add_slot(s, task.id, duration, task.name);

        /* Add small context switch cost after each task */
        if (!tq_is_empty(&work) && remaining_minutes > 2.0f) {
            remaining_minutes -= s->context_switch_cost;
            elapsed           += s->context_switch_cost;
            s->total_duration_minutes += s->context_switch_cost;
        }

        remaining_minutes -= duration;
        elapsed           += duration;
        tasks_scheduled++;

        /* Add Pomodoro-style break every 90 minutes */
        if (elapsed >= 90.0f && remaining_minutes > 15.0f) {
            add_break(s, 15.0f);
            remaining_minutes -= 15.0f;
            elapsed = 0.0f;  /* reset timer after break */
        }
    }

    printf("  [FIFO] Scheduled %d tasks in %.0f min work + %.0f min breaks\n",
           tasks_scheduled, s->total_duration_minutes - s->total_break_minutes,
           s->total_break_minutes);
    return tasks_scheduled;
}

/* ─────────────────────────────────────────────────────────────
 *  PRIORITY SCHEDULING
 *  Always run highest urgency task first.
 *  OS: non-preemptive priority scheduling.
 *  Human OS: Eisenhower Matrix — Urgent+Important first.
 * ──────────────────────────────────────────────────────────── */
int sched_priority(Schedule *s, TaskQueue *q, float hours) {
    TaskQueue work;
    copy_queue(q, &work);

    float remaining_minutes = hours * 60.0f;
    int   tasks_scheduled   = 0;

    printf("  [Priority] Scheduling tasks by urgency (highest first):\n");

    while (!tq_is_empty(&work) && remaining_minutes > 0) {
        QTask task;
        /* Heap automatically gives us highest urgency */
        if (!tq_dequeue(&work, &task)) break;

        printf("    → Task '%s' (urgency=%.1f)\n", task.name, task.urgency_score);

        float duration = fminf(task.duration_minutes, remaining_minutes);
        add_slot(s, task.id, duration, task.name);

        remaining_minutes -= duration;
        tasks_scheduled++;

        /* Context switch overhead */
        if (!tq_is_empty(&work) && remaining_minutes > 2.0f) {
            remaining_minutes -= s->context_switch_cost;
        }
    }
    return tasks_scheduled;
}

/* ─────────────────────────────────────────────────────────────
 *  ROUND ROBIN
 *  Each task gets a fixed time quantum (like Linux time-sharing).
 *  OS: prevents starvation — every task makes some progress.
 *  Human OS: "spend 25 min on each task, rotate" (Pomodoro variant).
 *
 *  quantum = time slice per task (like OS time quantum: ~10-100ms,
 *            here: 25-50 minutes of human focus)
 * ──────────────────────────────────────────────────────────── */
int sched_rr(Schedule *s, TaskQueue *q, float hours, float quantum) {
    /* Convert heap to array for round-robin (need to revisit tasks) */
    QTask tasks[MAX_QUEUE_SIZE];
    float remaining_each[MAX_QUEUE_SIZE];
    int   n = q->size;

    TaskQueue work;
    copy_queue(q, &work);

    for (int i = 0; i < n; i++) {
        tq_dequeue(&work, &tasks[i]);
        remaining_each[i] = tasks[i].duration_minutes;
    }

    float total_minutes = hours * 60.0f;
    float elapsed       = 0.0f;
    int   tasks_done    = 0;
    int   round         = 0;

    printf("  [RR] Round Robin: quantum=%.0f min over %.0f hours:\n",
           quantum, hours);

    while (elapsed < total_minutes) {
        int any_progress = 0;
        round++;

        for (int i = 0; i < n; i++) {
            if (remaining_each[i] <= 0) continue; /* already done */

            float work_this_slot = fminf(quantum, remaining_each[i]);
            work_this_slot = fminf(work_this_slot, total_minutes - elapsed);
            if (work_this_slot <= 0) break;

            add_slot(s, tasks[i].id, work_this_slot, tasks[i].name);
            remaining_each[i] -= work_this_slot;
            elapsed            += work_this_slot;
            elapsed            += s->context_switch_cost; /* context switch */
            any_progress = 1;

            if (remaining_each[i] <= 0) tasks_done++;

            /* Pomodoro break between rounds */
            if (i == n - 1 && elapsed < total_minutes - 15.0f) {
                add_break(s, 5.0f);  /* 5 min micro-break between rounds */
                elapsed += 5.0f;
            }
        }

        if (!any_progress) break;  /* all tasks complete */
    }

    printf("  [RR] %d rounds, %d/%d tasks completed\n", round, tasks_done, n);
    return tasks_done;
}

/* ─────────────────────────────────────────────────────────────
 *  SHORTEST JOB FIRST
 *  Sort tasks by duration (shortest first).
 *  Provably minimizes average waiting time.
 *  OS: SJF is optimal but requires knowing burst time in advance.
 *  Human OS: "do quick wins first, build momentum".
 * ──────────────────────────────────────────────────────────── */
int sched_sjf(Schedule *s, TaskQueue *q, float hours) {
    /* Extract all tasks, sort by duration */
    QTask tasks[MAX_QUEUE_SIZE];
    int   n = q->size;

    TaskQueue work;
    copy_queue(q, &work);
    for (int i = 0; i < n; i++) {
        tq_dequeue(&work, &tasks[i]);
    }

    /* Insertion sort by duration ascending */
    for (int i = 1; i < n; i++) {
        QTask key = tasks[i];
        int j = i - 1;
        while (j >= 0 && tasks[j].duration_minutes > key.duration_minutes) {
            tasks[j + 1] = tasks[j];
            j--;
        }
        tasks[j + 1] = key;
    }

    float remaining_minutes = hours * 60.0f;
    int   tasks_scheduled   = 0;

    printf("  [SJF] Scheduling shortest tasks first:\n");

    for (int i = 0; i < n && remaining_minutes > 0; i++) {
        float duration = fminf(tasks[i].duration_minutes, remaining_minutes);
        printf("    → '%s' (%.0f min)\n", tasks[i].name, duration);
        add_slot(s, tasks[i].id, duration, tasks[i].name);
        remaining_minutes -= duration + s->context_switch_cost;
        tasks_scheduled++;
    }
    return tasks_scheduled;
}

/* ─────────────────────────────────────────────────────────────
 *  EARLIEST DEADLINE FIRST
 *  Sort tasks by deadline (earliest first).
 *  OS: Used in real-time systems. Optimal if system is schedulable.
 *  Human OS: "handle everything that is due soonest first".
 * ──────────────────────────────────────────────────────────── */
int sched_edf(Schedule *s, TaskQueue *q, float hours) {
    QTask tasks[MAX_QUEUE_SIZE];
    int   n = q->size;
    time_t now = time(NULL);

    TaskQueue work;
    copy_queue(q, &work);
    for (int i = 0; i < n; i++) tq_dequeue(&work, &tasks[i]);

    /* Sort: tasks with deadline first (earliest deadline first),
     * tasks without deadline go last */
    for (int i = 1; i < n; i++) {
        QTask key = tasks[i];
        int j = i - 1;
        while (j >= 0) {
            time_t a_dl = tasks[j].deadline > 0 ? tasks[j].deadline : (time_t)LONG_MAX;
            time_t b_dl = key.deadline      > 0 ? key.deadline      : (time_t)LONG_MAX;
            if (a_dl > b_dl) { tasks[j + 1] = tasks[j]; j--; }
            else break;
        }
        tasks[j + 1] = key;
    }

    float remaining_minutes = hours * 60.0f;
    int   tasks_scheduled   = 0;

    printf("  [EDF] Scheduling by earliest deadline:\n");

    for (int i = 0; i < n && remaining_minutes > 0; i++) {
        double secs_left = tasks[i].deadline > 0
                           ? difftime(tasks[i].deadline, now) : -1;
        if (secs_left > 0)
            printf("    → '%s' (deadline in %.0f h)\n",
                   tasks[i].name, secs_left / 3600.0);
        else
            printf("    → '%s' (no deadline)\n", tasks[i].name);

        float duration = fminf(tasks[i].duration_minutes, remaining_minutes);
        add_slot(s, tasks[i].id, duration, tasks[i].name);
        remaining_minutes -= duration + s->context_switch_cost;
        tasks_scheduled++;
    }
    return tasks_scheduled;
}

/* ─────────────────────────────────────────────────────────────
 *  schedule_build() — dispatch to chosen algorithm
 * ──────────────────────────────────────────────────────────── */
int schedule_build(Schedule *s, TaskQueue *q, float available_hours) {
    switch (s->algorithm) {
        case SCHED_FIFO:     return sched_fifo    (s, q, available_hours);
        case SCHED_PRIORITY: return sched_priority(s, q, available_hours);
        case SCHED_RR:       return sched_rr      (s, q, available_hours, 25.0f);
        case SCHED_SJF:      return sched_sjf     (s, q, available_hours);
        case SCHED_EDF:      return sched_edf     (s, q, available_hours);
        default:             return sched_priority(s, q, available_hours);
    }
}

/* ─────────────────────────────────────────────────────────────
 *  schedule_compute_metrics() — analyse schedule quality
 *  OS: used to compare scheduler performance
 * ──────────────────────────────────────────────────────────── */
SchedMetrics schedule_compute_metrics(const Schedule *s) {
    SchedMetrics m = {0};
    if (!s || s->slot_count == 0) return m;

    int work_slots = 0;
    float wait_acc = 0.0f, elapsed = 0.0f;

    for (int i = 0; i < s->slot_count; i++) {
        if (!s->slots[i].is_break) {
            wait_acc += elapsed;  /* waiting = time before this slot starts */
            elapsed  += s->slots[i].duration_minutes;
            work_slots++;
        } else {
            elapsed  += s->slots[i].duration_minutes;
        }
    }

    float total_work = s->total_duration_minutes - s->total_break_minutes;

    m.avg_waiting_time    = work_slots > 0 ? wait_acc / work_slots : 0.0f;
    m.avg_turnaround_time = work_slots > 0 ? elapsed  / work_slots : 0.0f;
    m.cpu_utilization     = s->total_duration_minutes > 0
                            ? (total_work / s->total_duration_minutes) * 100.0f : 0.0f;
    m.throughput          = s->total_duration_minutes > 0
                            ? work_slots / (s->total_duration_minutes / 60.0f) : 0.0f;
    m.context_switches    = work_slots > 1 ? work_slots - 1 : 0;

    return m;
}

/* ─────────────────────────────────────────────────────────────
 *  schedule_print() — show the day plan (Gantt chart style)
 * ──────────────────────────────────────────────────────────── */
void schedule_print(const Schedule *s) {
    if (!s) return;

    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║  Schedule: %-40s║\n", sched_algorithm_name(s->algorithm));
    printf("╚══════════════════════════════════════════════════════╝\n");

    float elapsed = 0.0f;
    int   slot_num = 1;

    for (int i = 0; i < s->slot_count; i++) {
        const TimeSlot *sl = &s->slots[i];
        int h = (int)(elapsed / 60);
        int m = (int)(elapsed) % 60;

        if (sl->is_break) {
            printf("  %02d:%02d  ☕ BREAK         (%5.0f min)\n",
                   h, m, sl->duration_minutes);
        } else {
            printf("  %02d:%02d  [%2d] %-25s (%5.0f min)\n",
                   h, m, slot_num++, sl->label, sl->duration_minutes);
        }
        elapsed += sl->duration_minutes;
    }

    SchedMetrics met = schedule_compute_metrics(s);
    printf("\n  ─── Metrics ─────────────────────────────────────────\n");
    printf("  Work time:        %.0f min  |  Breaks: %.0f min\n",
           s->total_duration_minutes - s->total_break_minutes,
           s->total_break_minutes);
    printf("  Utilization:      %.1f%%\n",            met.cpu_utilization);
    printf("  Avg wait time:    %.1f min\n",          met.avg_waiting_time);
    printf("  Throughput:       %.2f tasks/hour\n",   met.throughput);
    printf("  Context switches: %d\n\n",              met.context_switches);
}

/* ─────────────────────────────────────────────────────────────
 *  schedule_compare() — run all algorithms on the same queue,
 *  show which performs best.
 *  OS: used to justify algorithm selection for a workload type.
 * ──────────────────────────────────────────────────────────── */
void schedule_compare(TaskQueue *q, float hours) {
    SchedAlgorithm algos[] = {
        SCHED_FIFO, SCHED_PRIORITY, SCHED_RR, SCHED_SJF, SCHED_EDF
    };

    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║  Algorithm Comparison (%.0f hours available)          \n", hours);
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf("  %-30s  %8s  %8s  %8s\n",
           "Algorithm", "Util%", "Avg Wait", "Thruput");
    printf("  %-30s  %8s  %8s  %8s\n",
           "──────────────────────────────",
           "────────", "────────", "────────");

    for (int i = 0; i < 5; i++) {
        Schedule s;
        schedule_init(&s, algos[i]);
        schedule_build(&s, q, hours);
        SchedMetrics m = schedule_compute_metrics(&s);
        printf("  %-30s  %7.1f%%  %7.1f m  %7.2f/h\n",
               sched_algorithm_name(algos[i]),
               m.cpu_utilization,
               m.avg_waiting_time,
               m.throughput);
    }
    printf("\n");
}
