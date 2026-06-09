/*
 * main.c  —  Phase 3: Energy Management System Demo
 *
 * Build:  gcc -Wall -Wextra -o phase3 main.c energy_model.c -lm && ./phase3
 *
 * Simulates a full week of energy management:
 *   - Circadian rhythm effects on performance
 *   - Burnout accumulation and recovery
 *   - Optimal task scheduling based on energy state
 *   - Recovery activity recommendations
 *
 * OS Analogy: This is like watching a CPU's power management
 * across a week: P-states, C-states, thermal throttling, boost.
 */

#include "../include/energy_model.h"
#include <stdio.h>
#include <string.h>

/* ─────────────────────────────────────────────────────────────
 *  Simulate one work session
 * ──────────────────────────────────────────────────────────── */
static void work_session(EnergyManager *em, const char *task,
                          float mental, float physical, float emotional,
                          float duration_min) {
    printf("  ▶ Working: %-35s (%.0f min)\n", task, duration_min);
    em_work(em, mental, physical, emotional, duration_min);

    /* Auto-suggest break when ultradian cycle ends */
    if (em->ultradian_phase >= 90.0f) {
        printf("    ⏰ Ultradian cycle done! Take a break.\n");
    }
}

/* ─────────────────────────────────────────────────────────────
 *  Simulate one full day
 * ──────────────────────────────────────────────────────────── */
static void simulate_day(EnergyManager *em, const char *day_name,
                          int productive) {
    printf("\n══════════════════════════════════════════════════════\n");
    printf("  %s\n", day_name);
    printf("══════════════════════════════════════════════════════\n");

    em->hour_of_day = 9;  /* start at 9 AM */

    if (productive) {
        /* Morning peak (09:00–12:00) — deep work */
        printf("\n  [09:00] Morning Peak — Deep Work\n");
        work_session(em, "Write code / complex problem",  20.0f, 2.0f,  3.0f, 90.0f);
        em_rest(em, "Walk Outdoors", 15.0f);  /* ultradian break */

        work_session(em, "Architecture design session",   15.0f, 1.0f,  5.0f, 60.0f);

        /* Lunch break */
        printf("\n  [12:00] Lunch Break\n");
        em_rest(em, "Nutritious Meal", 45.0f);

        em->hour_of_day = 13;  /* afternoon */

        /* Afternoon dip (13:00–14:00) — admin tasks */
        printf("\n  [13:00] Afternoon Dip — Admin Work\n");
        work_session(em, "Email and messages",             5.0f,  1.0f,  8.0f, 30.0f);
        em_rest(em, "Power Nap (20min)", 20.0f);

        em->hour_of_day = 14;

        /* Second peak (14:00–17:00) — collaborative */
        printf("\n  [14:00] Afternoon Recovery — Meetings\n");
        work_session(em, "Team meeting / code review",    10.0f,  2.0f, 15.0f, 60.0f);
        work_session(em, "Write documentation",           15.0f,  1.0f,  5.0f, 45.0f);

        /* Evening exercise */
        printf("\n  [17:00] Evening — Exercise + Wind Down\n");
        em->hour_of_day = 17;
        em_rest(em, "Physical Exercise", 45.0f);
        work_session(em, "Light reading / learning",       8.0f,  1.0f,  3.0f, 30.0f);

    } else {
        /* Unproductive day — overworked, no breaks */
        printf("\n  ⚠ Overworked day — skipping breaks!\n");
        work_session(em, "Non-stop work session",         30.0f, 5.0f, 10.0f, 180.0f);
        work_session(em, "More work, no break",           25.0f, 4.0f,  8.0f, 120.0f);
        work_session(em, "Pushing through fatigue",       15.0f, 2.0f,  5.0f, 90.0f);
    }

    em_print_status(em);
    em_recommend_next_action(em);
    em_suggest_optimal_task_type(em);
    em_print_daily_summary(em);

    /* Night sleep */
    printf("\n  [22:00] Sleep\n");
    float sleep_hours = productive ? 8.0f : 6.0f;  /* poor day = poor sleep */
    em_sleep(em, sleep_hours);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Human OS — Phase 3: Energy Management System       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* Show the circadian energy curve first */
    printf("=== Circadian Rhythm Profile ===\n");
    em_print_circadian_curve();
    printf("  Best times for deep work:  08:00–12:00, 14:00–16:00\n");
    printf("  Worst times for deep work: 01:00–06:00, 13:00–14:00\n\n");

    /* Initialize the energy manager */
    EnergyManager em;
    em_init(&em, 80.0f, 70.0f, 75.0f);

    /* Simulate a week */
    printf("=== Simulating One Week ===\n");

    /* Monday–Wednesday: productive days with proper breaks */
    simulate_day(&em, "Monday (Productive — Proper Breaks)",    1);
    simulate_day(&em, "Tuesday (Productive — Proper Breaks)",   1);
    simulate_day(&em, "Wednesday (Productive — Proper Breaks)", 1);

    /* Thursday–Friday: overworked days (burnout risk) */
    simulate_day(&em, "Thursday (Overworked — No Breaks!)", 0);
    simulate_day(&em, "Friday (Overworked — No Breaks!)",   0);

    /* Weekend: recovery */
    printf("\n══════════════════════════════════════════════════════\n");
    printf("  Saturday — Recovery Day\n");
    printf("══════════════════════════════════════════════════════\n");
    em_rest(&em, "Physical Exercise", 60.0f);
    em_rest(&em, "Social Time",       120.0f);
    em_rest(&em, "Meditation",        20.0f);
    em_sleep(&em, 9.0f);

    printf("\n══════════════════════════════════════════════════════\n");
    printf("  Sunday — Recovery + Light Work\n");
    printf("══════════════════════════════════════════════════════\n");
    em_rest(&em, "Walk Outdoors",     45.0f);
    em_rest(&em, "Light Reading",     60.0f);
    em_sleep(&em, 8.0f);

    /* End of week report */
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║  END OF WEEK REPORT                                  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
    em_print_status(&em);

    printf("\n  Key Lessons:\n");
    printf("  1. Morning peak (9–12) is 2x more productive than afternoon dip\n");
    printf("  2. 90-min focus cycles + 15-min breaks prevent energy drain\n");
    printf("  3. Overworking 2 days accumulates burnout quickly\n");
    printf("  4. Weekend recovery restores energy for next week\n");
    printf("  5. OS lesson: CPU thermal throttle = human burnout\n");
    printf("     Both are solved by scheduled cooling (rest) periods\n");

    return 0;
}
