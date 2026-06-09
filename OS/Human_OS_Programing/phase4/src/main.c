/*
 * main.c  —  Phase 4: Decision-Making Engine Demo
 *
 * Build:  gcc -Wall -Wextra -o phase4 main.c decision.c -lm && ./phase4
 *
 * Demonstrates:
 *   1. Multi-criteria weighted decision analysis
 *   2. Eisenhower Matrix classification
 *   3. Goal tracking and alignment
 *   4. Habit formation system
 */

#include "../include/decision.h"
#include <stdio.h>

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Human OS — Phase 4: Decision-Making Engine         ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* ── DEMO 1: Multi-Criteria Decision — Which job offer? ──── */
    printf("=== Demo 1: Job Offer Decision Analysis ===\n");
    printf("  Criteria: salary, growth, work-life-balance, learning, culture\n\n");

    DecisionEngine de;
    de_init(&de, 75.0f, 8.0f);  /* 75 energy, 8 hours available */

    /* Add weighted criteria (weights must sum to ~1.0) */
    de_add_criterion(&de, "Salary",            0.25f);
    de_add_criterion(&de, "Career Growth",     0.30f);
    de_add_criterion(&de, "Work-Life Balance", 0.20f);
    de_add_criterion(&de, "Learning Oppty",    0.15f);
    de_add_criterion(&de, "Culture Fit",       0.10f);

    /* Option scores for each criterion [0–10] */
    float scores_a[] = {9.0f, 7.0f, 5.0f, 8.0f, 7.0f};  /* BigTech offer */
    float scores_b[] = {6.0f, 9.0f, 8.0f, 9.0f, 9.0f};  /* Startup offer */
    float scores_c[] = {7.0f, 6.0f, 9.0f, 5.0f, 8.0f};  /* Current job+  */

    de_add_option(&de, "BigTech Company",      40.0f, 2.0f, 85.0f, scores_a);
    de_add_option(&de, "Startup Opportunity",  35.0f, 2.0f, 90.0f, scores_b);
    de_add_option(&de, "Stay + Promotion",     20.0f, 1.0f, 60.0f, scores_c);

    de_evaluate(&de);
    de_print_results(&de);

    /* ── DEMO 2: Eisenhower Matrix ───────────────────────────── */
    printf("=== Demo 2: Eisenhower Matrix — Task Classification ===\n");
    eisenhower_matrix_demo();

    /* ── DEMO 3: What to do RIGHT NOW? ─────────────────────────*/
    printf("=== Demo 3: What to do RIGHT NOW? (Energy=60, Time=3h) ===\n");

    DecisionEngine now;
    de_init(&now, 60.0f, 3.0f);  /* limited time and energy */

    de_add_criterion(&now, "Energy match",    0.40f);
    de_add_criterion(&now, "Value/ROI",       0.35f);
    de_add_criterion(&now, "Deadline",        0.25f);

    float s1[] = {8.0f, 9.0f, 7.0f};  /* deep work: coding */
    float s2[] = {4.0f, 5.0f, 8.0f};  /* deadline task */
    float s3[] = {2.0f, 3.0f, 3.0f};  /* email cleanup */
    float s4[] = {6.0f, 7.0f, 4.0f};  /* learning session */
    float s5[] = {1.0f, 8.0f, 2.0f};  /* requires too much energy */

    de_add_option(&now, "Deep work: write code",   55.0f, 2.5f, 80.0f, s1);
    de_add_option(&now, "Deadline task (urgent)",  30.0f, 1.5f, 70.0f, s2);
    de_add_option(&now, "Clear email inbox",       15.0f, 0.5f, 30.0f, s3);
    de_add_option(&now, "Watch learning video",    20.0f, 1.0f, 60.0f, s4);
    de_add_option(&now, "Major project planning",  80.0f, 4.0f, 90.0f, s5); /* infeasible */

    de_evaluate(&now);
    de_print_results(&now);

    /* ── DEMO 4: Goals & Habit Tracking ──────────────────────── */
    printf("=== Demo 4: Goals and Habit System ===\n");

    LifeManager lm;
    lm_init(&lm);

    /* Define life goals (OS: performance targets / KPIs) */
    Goal *g1 = lm_add_goal(&lm, "Complete advanced C course",
                             GOAL_LEARNING, 100.0f, 9.0f);
    Goal *g2 = lm_add_goal(&lm, "Get promoted to Senior Engineer",
                             GOAL_CAREER, 100.0f, 10.0f);
    Goal *g3 = lm_add_goal(&lm, "Run 5km without stopping",
                             GOAL_HEALTH, 100.0f, 8.0f);
    Goal *g4 = lm_add_goal(&lm, "Save 6-month emergency fund",
                             GOAL_FINANCE, 100.0f, 7.5f);

    /* Simulate a week of progress */
    lm_update_goal(&lm, g1->id, 15.0f);  /* did 15% of C course */
    lm_update_goal(&lm, g2->id, 5.0f);   /* small career progress */
    lm_update_goal(&lm, g3->id, 20.0f);  /* ran 1km today */
    lm_update_goal(&lm, g4->id, 8.0f);   /* saved some money */

    lm_print_goals(&lm);

    /* Define habits (OS: recurring cron jobs) */
    Habit *h1 = lm_add_habit(&lm, "Morning exercise",          5.0f, 1);
    Habit *h2 = lm_add_habit(&lm, "Read for 30 min",           7.0f, 1);
    Habit *h3 = lm_add_habit(&lm, "Daily code practice",       5.0f, 1);
    Habit *h4 = lm_add_habit(&lm, "Weekly review / planning",  1.0f, 1);
    Habit *h5 = lm_add_habit(&lm, "Late-night phone scrolling",7.0f, 0);

    /* Simulate completing habits over 3 days */
    for (int day = 0; day < 3; day++) {
        lm_habit_done(&lm, h1->id);  /* exercise */
        lm_habit_done(&lm, h2->id);  /* reading */
        lm_habit_done(&lm, h3->id);  /* coding */
    }
    lm_habit_done(&lm, h4->id);  /* weekly review once */

    lm_print_habits(&lm);

    /* Goal alignment check for a potential task */
    printf("  Goal alignment for 'Learning' task: %.0f%%\n",
           lm_check_goal_alignment(&lm, "Learning") * 100.0f);
    printf("  Goal alignment for 'Career' task:   %.0f%%\n\n",
           lm_check_goal_alignment(&lm, "Career") * 100.0f);

    /* ── KEY LESSONS ─────────────────────────────────────────── */
    printf("=== Key Decision-Making Lessons ===\n\n");
    printf("  1. Reduce decision fatigue: use algorithms, not willpower\n");
    printf("  2. Q2 tasks (important, not urgent) = highest ROI\n");
    printf("  3. Align daily decisions with long-term goals\n");
    printf("  4. Habits automate good decisions (no willpower needed)\n");
    printf("  5. OS lesson: good policy (decision rule) beats case-by-case\n");
    printf("     scheduling — the kernel doesn't decide per-process;\n");
    printf("     it applies a consistent policy.\n");

    return 0;
}
