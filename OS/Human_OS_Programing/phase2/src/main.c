/*
 * main.c  —  Phase 2: Task Management & Scheduling Demo
 *
 * Build:  gcc -Wall -Wextra -o phase2 main.c scheduler.c task_queue.c -lm && ./phase2
 *
 * Demonstrates all 5 scheduling algorithms applied to the same set of tasks.
 * Shows which algorithm works best for different life scenarios.
 */

#include "../include/scheduler.h"
#include "../include/task_queue.h"
#include <stdio.h>

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Human OS — Phase 2: Task Scheduling Algorithms     ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* ── 1. Build the ready queue with today's tasks ─────────── */
    printf("=== Building Ready Queue ===\n");
    TaskQueue q;
    tq_init(&q);
    tq_populate_sample(&q);

    printf("\nReady Queue (sorted by urgency):\n");
    tq_print(&q);

    /* ── 2. Demo: Priority Scheduling (recommended daily use) ─── */
    printf("\n=== Demo 1: Priority Scheduling (Best for Daily Use) ===\n");
    Schedule priority_sched;
    schedule_init(&priority_sched, SCHED_PRIORITY);
    schedule_build(&priority_sched, &q, 8.0f);
    schedule_print(&priority_sched);

    /* ── 3. Demo: Round Robin (good for project variety) ──────── */
    printf("=== Demo 2: Round Robin (25-min Pomodoro style) ===\n");
    Schedule rr_sched;
    schedule_init(&rr_sched, SCHED_RR);
    schedule_build(&rr_sched, &q, 4.0f);  /* half-day demo */
    schedule_print(&rr_sched);

    /* ── 4. Demo: EDF (deadline crunch mode) ─────────────────── */
    printf("=== Demo 3: EDF (Deadline Mode) ===\n");
    Schedule edf_sched;
    schedule_init(&edf_sched, SCHED_EDF);
    schedule_build(&edf_sched, &q, 6.0f);
    schedule_print(&edf_sched);

    /* ── 5. Algorithm comparison ─────────────────────────────── */
    schedule_compare(&q, 8.0f);

    /* ── 6. Real-world lesson ─────────────────────────────────── */
    printf("=== When to Use Each Algorithm ===\n\n");
    printf("  FIFO:     Linear workflows, no urgency differences\n");
    printf("  Priority: Normal productive day (recommended default)\n");
    printf("  RR:       Multiple projects needing daily progress\n");
    printf("  SJF:      Low energy / need quick wins to build momentum\n");
    printf("  EDF:      Deadline crunch — must meet specific commitments\n\n");
    printf("  OS lesson: Real Linux uses CFS (Completely Fair Scheduler)\n");
    printf("  which is a weighted Round Robin using a red-black tree.\n");
    printf("  For humans: Priority scheduling matches the Eisenhower Matrix.\n");

    return 0;
}
