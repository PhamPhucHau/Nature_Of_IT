/*
 * task.c  —  Phase 1: Task Management Implementation
 *
 * Build:  gcc -Wall -Wextra -o task_demo task.c energy.c main.c
 *
 * OS Concept: This file implements the "Process Control Block" (PCB)
 * management — creating, transitioning, and querying tasks.
 *
 * Every function maps to an OS kernel operation:
 *   task_create()   → fork() / exec() creating a new process
 *   task_start()    → scheduler dispatching a process to CPU
 *   task_complete() → process calling exit(0)
 *   task_fail()     → process calling exit(non-zero) or SIGSEGV
 *   task_pause()    → SIGSTOP — suspending a process
 */

#include "../include/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ─────────────────────────────────────────────────────────────
 *  STRING HELPERS
 * ──────────────────────────────────────────────────────────── */
const char* task_status_name(TaskStatus s) {
    switch (s) {
        case TASK_PENDING:   return "PENDING";
        case TASK_RUNNING:   return "RUNNING";
        case TASK_BLOCKED:   return "BLOCKED";
        case TASK_COMPLETED: return "COMPLETED";
        case TASK_FAILED:    return "FAILED";
        case TASK_PAUSED:    return "PAUSED";
        default:             return "UNKNOWN";
    }
}

const char* task_priority_name(TaskPriority p) {
    switch (p) {
        case PRIORITY_CRITICAL: return "CRITICAL";
        case PRIORITY_HIGH:     return "HIGH";
        case PRIORITY_MEDIUM:   return "MEDIUM";
        case PRIORITY_LOW:      return "LOW";
        case PRIORITY_OPTIONAL: return "OPTIONAL";
        default:                return "UNKNOWN";
    }
}

const char* task_energy_type_name(EnergyType e) {
    switch (e) {
        case ENERGY_MENTAL:    return "MENTAL";
        case ENERGY_PHYSICAL:  return "PHYSICAL";
        case ENERGY_EMOTIONAL: return "EMOTIONAL";
        case ENERGY_MIXED:     return "MIXED";
        default:               return "UNKNOWN";
    }
}

/* ─────────────────────────────────────────────────────────────
 *  task_manager_init()
 *  OS: Initialize the process table at kernel boot.
 *  Human OS: Start a new day / session with empty task table.
 * ──────────────────────────────────────────────────────────── */
void task_manager_init(TaskManager *tm) {
    if (!tm) return;

    /* Zero out the entire process table */
    memset(tm, 0, sizeof(TaskManager));

    /* PID counter starts at 1 (PID 0 is reserved in real OS for idle process) */
    tm->next_id = 1;
    tm->count   = 0;

    printf("[TaskManager] Initialized — process table ready (max %d tasks)\n",
           MAX_TASKS);
}

/* ─────────────────────────────────────────────────────────────
 *  task_create()
 *  OS: fork() + exec() — create a new process and load it
 *  Human OS: Add a new task to your to-do system
 *
 *  Returns: pointer to the new Task, or NULL if table is full
 * ──────────────────────────────────────────────────────────── */
Task* task_create(TaskManager *tm, const char *name, const char *desc,
                  TaskPriority priority, float duration_min, float energy) {
    if (!tm || !name) return NULL;

    /* Check if process table is full (OS: no more PID slots) */
    if (tm->count >= MAX_TASKS) {
        fprintf(stderr, "[TaskManager] ERROR: task table full (%d/%d)\n",
                tm->count, MAX_TASKS);
        return NULL;
    }

    /* Allocate the next available slot in the process table */
    Task *t = &tm->tasks[tm->count];
    memset(t, 0, sizeof(Task));  /* zero-initialize like calloc */

    /* Assign a unique PID (auto-incrementing, never reused) */
    t->id = tm->next_id++;

    /* Copy name and description */
    strncpy(t->name,        name, MAX_TASK_NAME - 1);
    strncpy(t->description, desc ? desc : "", MAX_TASK_DESC - 1);
    strncpy(t->category,    "general", MAX_TASK_CATEGORY - 1);

    /* Set scheduling parameters */
    t->priority            = priority;
    t->status              = TASK_PENDING;   /* NEW → READY in OS terms */
    t->energy_type         = ENERGY_MENTAL;  /* default */
    t->duration_minutes    = duration_min;
    t->energy_required     = energy;
    t->progress_percent    = 0.0f;
    t->value_score         = 5.0f;           /* default mid-range value */

    /* Record creation timestamp (like OS recording process creation time) */
    t->created_at = time(NULL);
    t->deadline   = 0;  /* no deadline by default */

    t->dependency_count = 0;
    t->is_recurring     = 0;

    tm->count++;

    printf("[TaskManager] Created task #%u '%s' (priority=%s, est=%.0fmin)\n",
           t->id, t->name, task_priority_name(t->priority), t->duration_minutes);

    return t;
}

/* ─────────────────────────────────────────────────────────────
 *  task_find_by_id()
 *  OS: Look up a process by PID in the process table.
 *  Returns NULL if not found.
 * ──────────────────────────────────────────────────────────── */
Task* task_find_by_id(TaskManager *tm, uint32_t task_id) {
    if (!tm) return NULL;

    /* Linear search — real OS uses hash table or PID-indexed array */
    for (int i = 0; i < tm->count; i++) {
        if (tm->tasks[i].id == task_id) {
            return &tm->tasks[i];
        }
    }
    return NULL;  /* ESRCH — No such process */
}

/* ─────────────────────────────────────────────────────────────
 *  task_can_start()
 *  OS: Check if all prerequisite processes have completed.
 *  Like checking that a process's required libraries are loaded.
 * ──────────────────────────────────────────────────────────── */
int task_can_start(const TaskManager *tm, const Task *t) {
    if (!tm || !t) return 0;

    /* If there are no dependencies, the task can always start */
    if (t->dependency_count == 0) return 1;

    /* Check each dependency: it must be COMPLETED */
    for (int i = 0; i < t->dependency_count; i++) {
        /* Find the dependency in the process table */
        for (int j = 0; j < tm->count; j++) {
            if (tm->tasks[j].id == t->dependencies[i]) {
                if (tm->tasks[j].status != TASK_COMPLETED) {
                    /* Dependency not done yet — task is BLOCKED */
                    return 0;
                }
                break;
            }
        }
    }
    return 1;  /* All dependencies satisfied */
}

/* ─────────────────────────────────────────────────────────────
 *  task_start()
 *  OS: Scheduler dispatches process → CPU (NEW/READY → RUNNING).
 *  Records start time for performance tracking.
 * ──────────────────────────────────────────────────────────── */
int task_start(TaskManager *tm, uint32_t task_id) {
    Task *t = task_find_by_id(tm, task_id);
    if (!t) {
        fprintf(stderr, "[TaskManager] ERROR: task #%u not found\n", task_id);
        return -1;  /* ESRCH */
    }

    /* Only PENDING or PAUSED tasks can be started */
    if (t->status != TASK_PENDING && t->status != TASK_PAUSED) {
        fprintf(stderr, "[TaskManager] ERROR: task #%u is not in PENDING/PAUSED state (is %s)\n",
                task_id, task_status_name(t->status));
        return -1;
    }

    /* Check dependencies (like checking shared libraries before exec) */
    if (!task_can_start(tm, t)) {
        t->status = TASK_BLOCKED;  /* OS: process BLOCKED waiting for I/O */
        printf("[TaskManager] Task #%u '%s' BLOCKED — dependencies not met\n",
               t->id, t->name);
        return -1;
    }

    /* Transition: PENDING → RUNNING (OS: context switch, dispatch to CPU) */
    t->status     = TASK_RUNNING;
    t->started_at = time(NULL);

    printf("[TaskManager] Started task #%u '%s'\n", t->id, t->name);
    return 0;
}

/* ─────────────────────────────────────────────────────────────
 *  task_complete()
 *  OS: Process calls exit(0) — normal termination.
 *  Records completion time and calculates actual duration.
 * ──────────────────────────────────────────────────────────── */
int task_complete(TaskManager *tm, uint32_t task_id) {
    Task *t = task_find_by_id(tm, task_id);
    if (!t) return -1;

    /* Must be RUNNING to complete (you can't exit what hasn't started) */
    if (t->status != TASK_RUNNING) {
        fprintf(stderr, "[TaskManager] WARN: task #%u is not RUNNING (is %s)\n",
                task_id, task_status_name(t->status));
    }

    /* Record completion */
    t->status           = TASK_COMPLETED;
    t->completed_at     = time(NULL);
    t->progress_percent = 100.0f;

    /* Calculate actual duration (like OS recording process CPU time used) */
    if (t->started_at > 0) {
        double elapsed_seconds = difftime(t->completed_at, t->started_at);
        t->actual_duration_minutes = (float)(elapsed_seconds / 60.0);
    }

    printf("[TaskManager] Completed task #%u '%s' (actual: %.1f min, estimated: %.1f min)\n",
           t->id, t->name,
           t->actual_duration_minutes,
           t->duration_minutes);

    return 0;
}

/* ─────────────────────────────────────────────────────────────
 *  task_fail()
 *  OS: Process exits with non-zero code / crashes.
 * ──────────────────────────────────────────────────────────── */
int task_fail(TaskManager *tm, uint32_t task_id) {
    Task *t = task_find_by_id(tm, task_id);
    if (!t) return -1;

    t->status       = TASK_FAILED;
    t->completed_at = time(NULL);

    printf("[TaskManager] FAILED task #%u '%s'\n", t->id, t->name);
    return 0;
}

/* ─────────────────────────────────────────────────────────────
 *  task_pause()
 *  OS: SIGSTOP — suspend a running process.
 *  Human OS: take a break mid-task.
 * ──────────────────────────────────────────────────────────── */
int task_pause(TaskManager *tm, uint32_t task_id) {
    Task *t = task_find_by_id(tm, task_id);
    if (!t || t->status != TASK_RUNNING) return -1;

    t->status = TASK_PAUSED;
    printf("[TaskManager] Paused task #%u '%s'\n", t->id, t->name);
    return 0;
}

/* ─────────────────────────────────────────────────────────────
 *  task_delete()
 *  OS: waitpid() — reap a terminated process, free its PCB.
 *  We remove the task by swapping with last element (O(1)).
 * ──────────────────────────────────────────────────────────── */
int task_delete(TaskManager *tm, uint32_t task_id) {
    if (!tm) return -1;

    for (int i = 0; i < tm->count; i++) {
        if (tm->tasks[i].id == task_id) {
            /* Swap with last task and shrink the array */
            tm->tasks[i] = tm->tasks[tm->count - 1];
            tm->count--;
            printf("[TaskManager] Deleted task #%u\n", task_id);
            return 0;
        }
    }
    return -1;  /* not found */
}

/* ─────────────────────────────────────────────────────────────
 *  task_is_overdue()
 *  Returns 1 if deadline has passed and task is not completed.
 * ──────────────────────────────────────────────────────────── */
int task_is_overdue(const Task *t) {
    if (!t || t->deadline == 0) return 0;
    if (t->status == TASK_COMPLETED || t->status == TASK_FAILED) return 0;
    return difftime(time(NULL), t->deadline) > 0;
}

/* ─────────────────────────────────────────────────────────────
 *  task_urgency_score()
 *  Combines priority + deadline proximity into one urgency number.
 *  Higher score = more urgent (like OS priority + aging factor).
 *
 *  Formula: base_priority + time_pressure_bonus
 *    - base: CRITICAL=100, HIGH=75, MEDIUM=50, LOW=25, OPTIONAL=10
 *    - bonus: inversely proportional to time remaining
 * ──────────────────────────────────────────────────────────── */
float task_urgency_score(const Task *t) {
    if (!t) return 0.0f;

    /* Base score from priority */
    float base[] = {100.0f, 75.0f, 50.0f, 25.0f, 10.0f};
    float score = base[(int)t->priority];

    /* Add urgency from deadline proximity */
    if (t->deadline > 0) {
        double seconds_left = difftime(t->deadline, time(NULL));
        if (seconds_left <= 0) {
            /* Overdue: maximum urgency bonus */
            score += 50.0f;
        } else if (seconds_left < 3600) {
            /* Less than 1 hour: high urgency */
            score += 40.0f;
        } else if (seconds_left < 86400) {
            /* Less than 1 day: medium urgency */
            score += 20.0f;
        } else if (seconds_left < 604800) {
            /* Less than 1 week: mild urgency */
            score += 10.0f;
        }
    }

    return score;
}

/* ─────────────────────────────────────────────────────────────
 *  task_print()
 *  Display a single task like `ps aux` shows one process.
 * ──────────────────────────────────────────────────────────── */
void task_print(const Task *t) {
    if (!t) return;

    char deadline_str[32] = "none";
    if (t->deadline > 0) {
        struct tm *dt = localtime(&t->deadline);
        strftime(deadline_str, sizeof(deadline_str), "%Y-%m-%d %H:%M", dt);
    }

    printf("  ┌─ Task #%-4u ─────────────────────────────────────────\n", t->id);
    printf("  │  Name:     %s\n", t->name);
    printf("  │  Status:   %-12s  Priority: %-10s\n",
           task_status_name(t->status), task_priority_name(t->priority));
    printf("  │  Energy:   %-10s    Required: %.1f units\n",
           task_energy_type_name(t->energy_type), t->energy_required);
    printf("  │  Duration: %.0f min (est)  %.1f min (actual)\n",
           t->duration_minutes, t->actual_duration_minutes);
    printf("  │  Progress: %.0f%%   Deadline: %s%s\n",
           t->progress_percent, deadline_str,
           task_is_overdue(t) ? " [OVERDUE!]" : "");
    printf("  │  Urgency:  %.1f    Value: %.1f\n",
           task_urgency_score(t), t->value_score);
    if (t->description[0]) {
        printf("  │  Desc:     %s\n", t->description);
    }
    printf("  └──────────────────────────────────────────────────────\n");
}

/* ─────────────────────────────────────────────────────────────
 *  task_manager_print_all()
 *  Like `ps aux` — show all processes.
 * ──────────────────────────────────────────────────────────── */
void task_manager_print_all(const TaskManager *tm) {
    if (!tm) return;

    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║  Task Table (%d/%d tasks)                            \n",
           tm->count, MAX_TASKS);
    printf("╚══════════════════════════════════════════════════════╝\n");

    if (tm->count == 0) {
        printf("  (no tasks)\n\n");
        return;
    }

    /* Header row: like `ps` column headers */
    printf("  %-4s  %-25s  %-10s  %-10s  %5s  %6s\n",
           "ID", "NAME", "STATUS", "PRIORITY", "EST", "PROG%");
    printf("  %-4s  %-25s  %-10s  %-10s  %5s  %6s\n",
           "────", "─────────────────────────",
           "──────────", "──────────", "─────", "──────");

    for (int i = 0; i < tm->count; i++) {
        const Task *t = &tm->tasks[i];
        printf("  %-4u  %-25s  %-10s  %-10s  %4.0fm  %5.0f%%\n",
               t->id, t->name,
               task_status_name(t->status),
               task_priority_name(t->priority),
               t->duration_minutes,
               t->progress_percent);
    }
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
 *  task_manager_print_by_priority()
 *  Like `ps --sort=-priority` — show tasks sorted by urgency.
 *  Implements insertion sort (good for small N, readable).
 * ──────────────────────────────────────────────────────────── */
void task_manager_print_by_priority(const TaskManager *tm) {
    if (!tm || tm->count == 0) return;

    /* Create index array for sorting (avoid copying full Task structs) */
    int order[MAX_TASKS];
    for (int i = 0; i < tm->count; i++) order[i] = i;

    /* Sort by urgency score descending (insertion sort) */
    for (int i = 1; i < tm->count; i++) {
        int key = order[i];
        int j   = i - 1;
        while (j >= 0 &&
               task_urgency_score(&tm->tasks[order[j]]) <
               task_urgency_score(&tm->tasks[key])) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }

    printf("\n  Tasks by Urgency (highest first):\n");
    printf("  %-4s  %-25s  %-10s  %8s\n",
           "ID", "NAME", "PRIORITY", "URGENCY");
    printf("  %-4s  %-25s  %-10s  %8s\n",
           "────", "─────────────────────────",
           "──────────", "────────");

    for (int i = 0; i < tm->count; i++) {
        const Task *t = &tm->tasks[order[i]];
        if (t->status == TASK_COMPLETED || t->status == TASK_FAILED) continue;
        printf("  %-4u  %-25s  %-10s  %8.1f\n",
               t->id, t->name,
               task_priority_name(t->priority),
               task_urgency_score(t));
    }
    printf("\n");
}
