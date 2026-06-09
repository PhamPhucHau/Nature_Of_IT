/*
 * test_phase4.c  —  Phase 4 Unit Tests
 *
 * Build:  gcc -Wall -Wextra -o test4 test_phase4.c ../src/decision.c -lm && ./test4
 */

#include "../include/decision.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static int tests_run = 0, tests_passed = 0;
#define TEST(desc, expr) do { \
    tests_run++;               \
    if (expr) { tests_passed++; printf("  ✓ %s\n", desc); } \
    else { printf("  ✗ FAIL: %s  (line %d)\n", desc, __LINE__); } \
} while(0)

/* ─────────────────────────────────────────────────────────────
 *  Decision Engine Tests
 * ──────────────────────────────────────────────────────────── */
void test_decision_engine_init(void) {
    printf("\n── Decision Engine Init Tests ───────────────────────\n");
    DecisionEngine de;
    de_init(&de, 80.0f, 6.0f);

    TEST("criteria count is 0",   de.criteria_count == 0);
    TEST("option count is 0",     de.option_count == 0);
    TEST("best_option_idx is -1", de.best_option_idx == -1);
    TEST("energy set correctly",  de.current_energy == 80.0f);
    TEST("time set correctly",    de.current_time   == 6.0f);
}

void test_decision_add_criteria(void) {
    printf("\n── Add Criteria Tests ───────────────────────────────\n");
    DecisionEngine de;
    de_init(&de, 80.0f, 6.0f);

    de_add_criterion(&de, "Importance", 0.5f);
    de_add_criterion(&de, "Urgency",    0.3f);
    de_add_criterion(&de, "Energy fit", 0.2f);

    TEST("3 criteria added",            de.criteria_count == 3);
    TEST("first criterion name OK",     strcmp(de.criteria[0].name, "Importance") == 0);
    TEST("first criterion weight OK",   de.criteria[0].weight == 0.5f);
    TEST("second criterion weight OK",  de.criteria[1].weight == 0.3f);
}

void test_decision_feasibility(void) {
    printf("\n── Feasibility Tests ────────────────────────────────\n");
    DecisionEngine de;
    de_init(&de, 50.0f, 4.0f);  /* 50 energy, 4 hours */

    de_add_criterion(&de, "Value", 1.0f);

    float s_ok[]  = {8.0f};
    float s_too_tired[] = {8.0f};
    float s_too_long[]  = {8.0f};

    de_add_option(&de, "Feasible task",     30.0f, 2.0f, 70.0f, s_ok);
    de_add_option(&de, "Too energy costly", 80.0f, 2.0f, 90.0f, s_too_tired);
    de_add_option(&de, "Too time costly",   20.0f, 6.0f, 85.0f, s_too_long);

    TEST("feasible task is feasible",           de.options[0].feasible == 1);
    TEST("energy-too-high is not feasible",     de.options[1].feasible == 0);
    TEST("time-too-high is not feasible",       de.options[2].feasible == 0);
}

void test_decision_best_choice(void) {
    printf("\n── Best Choice Selection Tests ──────────────────────\n");
    DecisionEngine de;
    de_init(&de, 100.0f, 8.0f);

    de_add_criterion(&de, "Value",  0.6f);
    de_add_criterion(&de, "Growth", 0.4f);

    /* Option A has higher scores */
    float sa[] = {9.0f, 9.0f};   /* best scores */
    float sb[] = {5.0f, 5.0f};   /* medium scores */
    float sc[] = {2.0f, 2.0f};   /* low scores */

    de_add_option(&de, "Best Option",   20.0f, 2.0f, 80.0f, sa);
    de_add_option(&de, "Medium Option", 20.0f, 2.0f, 50.0f, sb);
    de_add_option(&de, "Worst Option",  20.0f, 2.0f, 20.0f, sc);

    int best = de_evaluate(&de);
    TEST("best option is index 0 (highest scores)", best == 0);
    TEST("best has highest total score",
         de.options[0].total_score > de.options[1].total_score);
    TEST("worst has lowest total score",
         de.options[1].total_score > de.options[2].total_score);
}

/* ─────────────────────────────────────────────────────────────
 *  Eisenhower Matrix Tests
 * ──────────────────────────────────────────────────────────── */
void test_eisenhower_matrix(void) {
    printf("\n── Eisenhower Matrix Tests ──────────────────────────\n");
    char q[64], a[64];

    /* Q1: Urgent + Important */
    eisenhower_classify(9.0f, 9.0f, q, a);
    TEST("Q1: high urgency + high importance",
         strstr(q, "Q1") != NULL);

    /* Q2: Not Urgent + Important */
    eisenhower_classify(2.0f, 9.0f, q, a);
    TEST("Q2: low urgency + high importance",
         strstr(q, "Q2") != NULL);

    /* Q3: Urgent + Not Important */
    eisenhower_classify(9.0f, 2.0f, q, a);
    TEST("Q3: high urgency + low importance",
         strstr(q, "Q3") != NULL);

    /* Q4: Neither */
    eisenhower_classify(2.0f, 2.0f, q, a);
    TEST("Q4: low urgency + low importance",
         strstr(q, "Q4") != NULL);
}

/* ─────────────────────────────────────────────────────────────
 *  Goal Management Tests
 * ──────────────────────────────────────────────────────────── */
void test_goal_management(void) {
    printf("\n── Goal Management Tests ─────────────────────────────\n");
    LifeManager lm;
    lm_init(&lm);

    Goal *g = lm_add_goal(&lm, "Learn C programming",
                            GOAL_LEARNING, 100.0f, 9.0f);

    TEST("goal created",          g != NULL);
    TEST("goal has valid ID",     g && g->id > 0);
    TEST("goal is active",        g && g->active == 1);
    TEST("progress starts at 0",  g && g->current_value == 0.0f);
    TEST("goal count is 1",       lm.goal_count == 1);

    lm_update_goal(&lm, g->id, 25.0f);
    TEST("goal progress updated to 25",  g->current_value == 25.0f);

    lm_update_goal(&lm, g->id, 80.0f);
    TEST("goal progress capped at target",
         g->current_value <= g->target_value);
}

/* ─────────────────────────────────────────────────────────────
 *  Habit Tracking Tests
 * ──────────────────────────────────────────────────────────── */
void test_habit_tracking(void) {
    printf("\n── Habit Tracking Tests ─────────────────────────────\n");
    LifeManager lm;
    lm_init(&lm);

    Habit *h = lm_add_habit(&lm, "Morning run", 5.0f, 1);

    TEST("habit created",              h != NULL);
    TEST("habit starts with 0 streak", h && h->streak_days == 0);
    TEST("habit is positive",          h && h->is_positive == 1);
    TEST("habit count is 1",           lm.habit_count == 1);

    lm_habit_done(&lm, h->id);
    TEST("streak increases to 1",      h->streak_days == 1);
    TEST("completions this week = 1",  h->completions_this_week == 1);
    TEST("best streak updated",        h->best_streak >= 1);

    lm_habit_done(&lm, h->id);
    lm_habit_done(&lm, h->id);
    TEST("streak increases to 3",      h->streak_days == 3);
    TEST("best streak = 3",            h->best_streak == 3);
}

/* ─────────────────────────────────────────────────────────────
 *  Cost-Benefit Tests
 * ──────────────────────────────────────────────────────────── */
void test_cost_benefit(void) {
    printf("\n── Cost-Benefit Tests ────────────────────────────────\n");

    float ratio_high = cost_benefit_ratio(100.0f, 10.0f,  1.0f, 20.0f);
    float ratio_low  = cost_benefit_ratio( 10.0f, 50.0f, 10.0f, 80.0f);

    TEST("high value = high ROI",   ratio_high > ratio_low);
    TEST("ratio > 0 for valid args",ratio_high > 0.0f);
    TEST("zero benefit = zero ROI", cost_benefit_ratio(0, 10, 1, 10) == 0.0f);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Phase 4 Unit Tests                                  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    test_decision_engine_init();
    test_decision_add_criteria();
    test_decision_feasibility();
    test_decision_best_choice();
    test_eisenhower_matrix();
    test_goal_management();
    test_habit_tracking();
    test_cost_benefit();

    printf("\n══════════════════════════════════════════════════════\n");
    printf("  Results: %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
