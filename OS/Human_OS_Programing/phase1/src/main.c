/*
 * main.c  —  Phase 1: Foundations Demo
 *
 * Build:  gcc -Wall -Wextra -o phase1 main.c task.c energy.c -lm && ./phase1
 *
 * This demo simulates a full workday using Human OS.
 * Watch how tasks are scheduled, energy is consumed, and the system
 * makes decisions based on available resources.
 *
 * OS Analogy:
 *   This is like booting an OS, loading processes, running them,
 *   and tracking CPU utilization across the day.
 */

#include "../include/task.h"
#include "../include/energy.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>  /* usleep */

/* ─────────────────────────────────────────────────────────────
 *  Simulate executing one task:
 *    1. Check energy availability (can we run this process?)
 *    2. Start the task (dispatch to CPU)
 *    3. Consume energy (task uses resources)
 *    4. Complete or fail based on energy (like CPU throttle)
 * ──────────────────────────────────────────────────────────── */
static void execute_task(TaskManager *tm, EnergyState *es, uint32_t task_id) {
    Task *t = task_find_by_id(tm, task_id);
    if (!t) return;

    printf("\n  ▶ Executing: #%u '%s'\n", t->id, t->name);

    /* Check if we have enough energy to run this task */
    float efficiency = energy_efficiency_factor(es);
    float effective_capacity = energy_get_overall(es);

    if (effective_capacity < ENERGY_CRITICAL) {
        printf("    ✗ Insufficient energy (%.0f%%) — deferring task\n",
               effective_capacity);
        task_pause(tm, task_id);
        return;
    }

    /* Start the task */
    if (task_start(tm, task_id) != 0) return;

    /* Simulate work: consume energy based on task type and efficiency */
    float energy_cost = t->energy_required * (1.0f + (1.0f - efficiency) * 0.5f);
    energy_consume_task(es, energy_cost, (int)t->energy_type);

    /* Update progress: efficiency affects how much gets done */
    t->progress_percent = 100.0f * efficiency;

    /* Complete if progress reached 100%, otherwise keep running */
    if (t->progress_percent >= 95.0f) {
        task_complete(tm, task_id);
    } else {
        printf("    ○ Task partially done (%.0f%%) — low energy\n",
               t->progress_percent);
    }
}

/* ─────────────────────────────────────────────────────────────
 *  Print a visual energy bar (like a progress bar in terminal)
 * ──────────────────────────────────────────────────────────── */
static void print_energy_bar(const char *label, float value, float capacity) {
    int bar_width = 30;
    int filled = (int)((value / capacity) * bar_width);
    if (filled > bar_width) filled = bar_width;

    printf("  %-10s [", label);
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            /* Use different chars for different energy levels */
            float pct = value / capacity * 100.0f;
            printf(pct > 60 ? "█" : pct > 30 ? "▓" : "░");
        } else {
            printf(" ");
        }
    }
    printf("] %.0f/%.0f\n", value, capacity);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Human OS — Phase 1: Foundations                    ║\n");
    printf("║  Simulating a complete workday                       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* ── BOOT: Initialize the Human OS ──────────────────────────── */
    printf("=== SYSTEM BOOT ===\n");
    TaskManager tm;
    EnergyState es;

    task_manager_init(&tm);
    energy_init(&es, 80.0f, 70.0f, 75.0f);  /* personal baselines */
    energy_log(&es, "morning start");

    /* ── LOAD PROCESSES: Register today's tasks ─────────────────── */
    printf("\n=== LOADING PROCESSES (Today's Tasks) ===\n");

    /* Create tasks with different priorities and energy requirements */
    Task *t1 = task_create(&tm, "Write project proposal",
                            "Research and write Q3 proposal doc",
                            PRIORITY_HIGH, 90.0f, 25.0f);
    if (t1) { t1->energy_type = ENERGY_MENTAL; t1->value_score = 9.0f; }

    Task *t2 = task_create(&tm, "Morning workout",
                            "30 min run + 20 min weights",
                            PRIORITY_HIGH, 50.0f, 20.0f);
    if (t2) { t2->energy_type = ENERGY_PHYSICAL; t2->value_score = 8.0f; }

    Task *t3 = task_create(&tm, "Code review PR #142",
                            "Review and comment on team PR",
                            PRIORITY_MEDIUM, 45.0f, 15.0f);
    if (t3) { t3->energy_type = ENERGY_MENTAL; }

    Task *t4 = task_create(&tm, "Team standup call",
                            "Daily sync with team",
                            PRIORITY_CRITICAL, 15.0f, 5.0f);
    if (t4) { t4->energy_type = ENERGY_EMOTIONAL; }

    Task *t5 = task_create(&tm, "Deep work: algorithm design",
                            "Design caching algorithm for user data",
                            PRIORITY_HIGH, 120.0f, 30.0f);
    if (t5) { t5->energy_type = ENERGY_MENTAL; t5->value_score = 9.5f; }

    Task *t6 = task_create(&tm, "Reply to emails",
                            "Process and respond to inbox",
                            PRIORITY_LOW, 30.0f, 10.0f);
    if (t6) { t6->energy_type = ENERGY_MIXED; }

    /* Print the loaded process table */
    task_manager_print_all(&tm);
    task_manager_print_by_priority(&tm);

    /* ── MORNING SESSION: Execute high-priority tasks ────────────── */
    printf("=== MORNING SESSION (9:00 - 12:00) ===\n");
    printf("Energy at session start:\n");
    print_energy_bar("Mental",   es.mental,   es.mental_capacity);
    print_energy_bar("Physical", es.physical, es.physical_capacity);
    print_energy_bar("Emotion",  es.emotional,es.emotional_capacity);
    printf("\n");

    /* Execute in priority order: CRITICAL first, then HIGH */
    execute_task(&tm, &es, t4->id);   /* standup — CRITICAL */
    execute_task(&tm, &es, t2->id);   /* workout — HIGH, physical */
    execute_task(&tm, &es, t1->id);   /* proposal — HIGH, mental */

    energy_log(&es, "after morning session");

    /* ── LUNCH BREAK: Recovery ───────────────────────────────────── */
    printf("\n=== LUNCH BREAK (12:00 - 13:00) ===\n");
    energy_recover(&es, 1.0f);  /* 1 hour rest */
    energy_restore(&es, 5.0f, 10.0f, 8.0f);  /* meal + break */
    energy_log(&es, "after lunch");

    print_energy_bar("Mental",   es.mental,   es.mental_capacity);
    print_energy_bar("Physical", es.physical, es.physical_capacity);
    print_energy_bar("Emotion",  es.emotional,es.emotional_capacity);

    /* ── AFTERNOON SESSION ───────────────────────────────────────── */
    printf("\n=== AFTERNOON SESSION (13:00 - 17:00) ===\n");

    execute_task(&tm, &es, t5->id);   /* deep work — HIGH */
    execute_task(&tm, &es, t3->id);   /* code review — MEDIUM */
    execute_task(&tm, &es, t6->id);   /* emails — LOW */

    energy_log(&es, "end of work day");

    /* ── END OF DAY REPORT ───────────────────────────────────────── */
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║  END OF DAY REPORT                                  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    task_manager_print_all(&tm);

    int completed = 0, pending = 0, failed = 0;
    for (int i = 0; i < tm.count; i++) {
        switch (tm.tasks[i].status) {
            case TASK_COMPLETED: completed++; break;
            case TASK_FAILED:    failed++;    break;
            default:             pending++;   break;
        }
    }
    printf("  Summary: %d completed, %d pending/paused, %d failed\n\n",
           completed, pending, failed);

    printf("  Final Energy Levels:\n");
    energy_print_status(&es);

    printf("  Energy History:\n");
    energy_print_history(&es, 10);

    printf("  Overall day efficiency: %.0f%%\n",
           energy_efficiency_factor(&es) * 100.0f);
    printf("\n  Day complete. System ready for overnight recovery.\n");

    return 0;
}
