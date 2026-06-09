/*
 * test_phase3.c  —  Phase 3 Unit Tests
 *
 * Build:  gcc -Wall -Wextra -o test3 test_phase3.c ../src/energy_model.c -lm && ./test3
 */

#include "../include/energy_model.h"
#include <stdio.h>
#include <math.h>

static int tests_run = 0, tests_passed = 0;
#define TEST(desc, expr) do { \
    tests_run++;               \
    if (expr) { tests_passed++; printf("  ✓ %s\n", desc); } \
    else { printf("  ✗ FAIL: %s  (line %d)\n", desc, __LINE__); } \
} while(0)
#define APPROX(a, b) (fabsf((a) - (b)) < 1.0f)

void test_init(void) {
    printf("\n── Init Tests ───────────────────────────────────────\n");
    EnergyManager em;
    em_init(&em, 80.0f, 70.0f, 75.0f);

    TEST("mental at capacity",     APPROX(em.mental,    80.0f));
    TEST("physical at capacity",   APPROX(em.physical,  70.0f));
    TEST("emotional at capacity",  APPROX(em.emotional, 75.0f));
    TEST("burnout starts at 0",    em.burnout_score == 0.0f);
    TEST("overall ~= 100%",        em_get_overall(&em) > 95.0f);
    TEST("performance ~= 1.0",     APPROX(em_get_performance_factor(&em), 1.0f));
}

void test_work_depletes(void) {
    printf("\n── Work Depletion Tests ─────────────────────────────\n");
    EnergyManager em;
    em_init(&em, 100.0f, 100.0f, 100.0f);

    float m_before = em.mental;
    em_work(&em, 20.0f, 0.0f, 0.0f, 60.0f);

    TEST("mental depleted after work",  em.mental < m_before);
    TEST("work minutes tracked",        em.total_work_minutes > 0);
    TEST("ultradian phase advances",    em.ultradian_phase > 0.0f);
}

void test_recovery(void) {
    printf("\n── Recovery Tests ───────────────────────────────────\n");
    EnergyManager em;
    em_init(&em, 100.0f, 100.0f, 100.0f);

    /* Deplete */
    em_work(&em, 40.0f, 30.0f, 20.0f, 120.0f);
    float m_after_work = em.mental;

    /* Recover */
    em_rest(&em, "Meditation", 20.0f);
    TEST("mental improves after rest", em.mental > m_after_work);

    float burnout_before_sleep = em.burnout_score;
    em_sleep(&em, 8.0f);
    TEST("sleep reduces burnout",      em.burnout_score < burnout_before_sleep);
    TEST("sleep restores energy",      em.mental > m_after_work + 10.0f);
}

void test_burnout_accumulation(void) {
    printf("\n── Burnout Accumulation Tests ───────────────────────\n");
    EnergyManager em;
    em_init(&em, 80.0f, 70.0f, 75.0f);

    /* Work a LOT without recovery */
    em_work(&em, 60.0f, 40.0f, 30.0f, 300.0f);
    TEST("burnout accumulates", em.burnout_score > 0.0f);

    /* Overwork for 3 days */
    for (int i = 0; i < 3; i++) {
        em_work(&em, 50.0f, 30.0f, 20.0f, 480.0f);
        em_sleep(&em, 5.0f);  /* poor sleep */
    }
    TEST("chronic overwork raises burnout",  em.burnout_score >= 20.0f);
}

void test_sleep_quality(void) {
    printf("\n── Sleep Quality Tests ──────────────────────────────\n");
    EnergyManager em1, em2;
    em_init(&em1, 100.0f, 100.0f, 100.0f);
    em_init(&em2, 100.0f, 100.0f, 100.0f);

    /* Deplete both equally */
    em_work(&em1, 60.0f, 50.0f, 40.0f, 240.0f);
    em_work(&em2, 60.0f, 50.0f, 40.0f, 240.0f);

    /* em1 gets 8h sleep, em2 gets 4h */
    em_sleep(&em1, 8.0f);
    em_sleep(&em2, 4.0f);

    TEST("more sleep = more recovery",       em1.mental > em2.mental);
    TEST("8h sleep beats 4h sleep",         em1.physical > em2.physical);
}

void test_circadian_multiplier(void) {
    printf("\n── Circadian Rhythm Tests ────────────────────────────\n");

    float morning = em_get_circadian_multiplier(10);  /* 10 AM */
    float slump   = em_get_circadian_multiplier(13);  /* 1 PM */
    float midnight= em_get_circadian_multiplier(2);   /* 2 AM */

    TEST("morning peak is high",       morning > 0.8f);
    TEST("afternoon slump is lower",   slump   < morning);
    TEST("midnight is lowest",         midnight < slump);
    TEST("morning > midnight",         morning  > midnight);
}

void test_performance_factor(void) {
    printf("\n── Performance Factor Tests ─────────────────────────\n");
    EnergyManager em;
    em_init(&em, 100.0f, 100.0f, 100.0f);

    TEST("full energy = 1.0 performance", APPROX(em_get_performance_factor(&em), 1.0f));

    em_work(&em, 60.0f, 50.0f, 40.0f, 300.0f);
    float depleted_perf = em_get_performance_factor(&em);
    TEST("low energy reduces performance", depleted_perf < 1.0f);
    TEST("performance > 0",                depleted_perf > 0.0f);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Phase 3 Unit Tests                                  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    test_init();
    test_work_depletes();
    test_recovery();
    test_burnout_accumulation();
    test_sleep_quality();
    test_circadian_multiplier();
    test_performance_factor();

    printf("\n══════════════════════════════════════════════════════\n");
    printf("  Results: %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
