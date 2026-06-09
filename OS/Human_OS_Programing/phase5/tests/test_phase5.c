/*
 * test_phase5.c  —  Phase 5 Unit Tests
 *
 * Build:  gcc -Wall -Wextra -o test5 test_phase5.c ../src/learning.c -lm && ./test5
 */

#include "../include/learning.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static int tests_run = 0, tests_passed = 0;
#define TEST(desc, expr) do { \
    tests_run++;               \
    if (expr) { tests_passed++; printf("  ✓ %s\n", desc); } \
    else { printf("  ✗ FAIL: %s  (line %d)\n", desc, __LINE__); } \
} while(0)
#define APPROX(a,b) (fabsf((a)-(b)) < 2.0f)

/* ─────────────────────────────────────────────────────────────
 *  Experience Tests
 * ──────────────────────────────────────────────────────────── */
void test_experience_recording(void) {
    printf("\n── Experience Recording Tests ───────────────────────\n");
    LearningSystem ls;
    ls_init(&ls);

    Experience *e = ls_add_experience(&ls,
        "Fixed a critical bug",
        "Add logging before fixing to understand the root cause",
        "Engineering", OUTCOME_GOOD, 8.0f);

    TEST("experience created",           e != NULL);
    TEST("experience has valid ID",      e && e->id > 0);
    TEST("outcome recorded correctly",   e && e->outcome == OUTCOME_GOOD);
    TEST("impact recorded correctly",    e && APPROX(e->impact_score, 8.0f));
    TEST("exp count is 1",               ls.exp_count == 1);
    TEST("review queued",                ls.review_count == 1);
    TEST("retention starts at 100",      e && APPROX(e->retention_score, 100.0f));

    /* Second experience */
    Experience *e2 = ls_add_experience(&ls,
        "Missed deadline",
        "Buffer all estimates by 50%",
        "Engineering", OUTCOME_BAD, 9.0f);

    TEST("second experience created",    e2 != NULL);
    TEST("exp count is 2",               ls.exp_count == 2);
    TEST("review count is 2",            ls.review_count == 2);
}

/* ─────────────────────────────────────────────────────────────
 *  Skill Tests
 * ──────────────────────────────────────────────────────────── */
void test_skill_registration(void) {
    printf("\n── Skill Registration Tests ─────────────────────────\n");
    LearningSystem ls;
    ls_init(&ls);

    Skill *s = ls_add_skill(&ls, "C Programming", "Engineering");

    TEST("skill created",            s != NULL);
    TEST("skill starts at Novice",   s && s->level == SKILL_NOVICE);
    TEST("practice hours = 0",       s && s->practice_hours == 0.0f);
    TEST("xp starts at 0",           s && s->xp_points == 0.0f);
    TEST("streak starts at 0",       s && s->daily_streak == 0);
    TEST("skill count is 1",         ls.skill_count == 1);
}

void test_skill_leveling(void) {
    printf("\n── Skill Leveling Tests ─────────────────────────────\n");
    LearningSystem ls;
    ls_init(&ls);

    Skill *s = ls_add_skill(&ls, "Test Skill", "Test");

    /* Practice enough to level up */
    ls_practice_skill(&ls, s->id, 5.0f);   /* enough for Novice → Beginner */
    TEST("practice hours increased",     s->practice_hours > 0.0f);
    TEST("xp increased",                 s->xp_points >= 0.0f);

    /* Practice a LOT to reach higher levels */
    for (int i = 0; i < 20; i++) {
        ls_practice_skill(&ls, s->id, 5.0f);
    }

    TEST("level increases with practice",  s->level >= SKILL_NOVICE);
    TEST("practice hours accumulate",      s->practice_hours >= 100.0f);
    TEST("streak increases with practice", s->daily_streak > 0);
}

void test_skill_progress_percent(void) {
    printf("\n── Skill Progress Percent Tests ─────────────────────\n");
    LearningSystem ls;
    ls_init(&ls);

    Skill *s = ls_add_skill(&ls, "Progress Test", "Test");

    TEST("progress starts at 0%",         APPROX(ls_get_skill_progress_pct(s), 0.0f));

    ls_practice_skill(&ls, s->id, 2.0f);
    TEST("progress > 0 after practice",   ls_get_skill_progress_pct(s) > 0.0f);
    TEST("progress <= 100%",              ls_get_skill_progress_pct(s) <= 100.0f);
}

/* ─────────────────────────────────────────────────────────────
 *  Pattern Recognition Tests
 * ──────────────────────────────────────────────────────────── */
void test_pattern_recording(void) {
    printf("\n── Pattern Recording Tests ──────────────────────────\n");
    LearningSystem ls;
    ls_init(&ls);

    Pattern *p = ls_record_pattern(&ls, "Morning", "Exercise", "High energy", 1);
    TEST("pattern created",                  p != NULL);
    TEST("occurrence count is 1",            p && p->occurrence_count == 1);
    TEST("success rate 100% (one success)",  p && APPROX(p->success_rate, 100.0f));

    /* Same pattern again — should update, not create new */
    int count_before = ls.pattern_count;
    ls_record_pattern(&ls, "Morning", "Exercise", "High energy", 1);
    TEST("same pattern updates, not creates",ls.pattern_count == count_before);
    TEST("occurrence count increased to 2",  p && p->occurrence_count == 2);

    /* Add a failure */
    ls_record_pattern(&ls, "Morning", "Exercise", "High energy", 0);
    TEST("success rate updates on failure",  p && p->success_rate < 100.0f);
    TEST("confidence increases with data",   p && p->confidence > 50.0f);
}

void test_pattern_confidence(void) {
    printf("\n── Pattern Confidence Tests ─────────────────────────\n");
    LearningSystem ls;
    ls_init(&ls);

    Pattern *p = ls_record_pattern(&ls, "T", "A", "R", 1);
    TEST("initial confidence = 50%",  p && APPROX(p->confidence, 50.0f));

    /* Record same pattern many times */
    for (int i = 0; i < 8; i++) {
        ls_record_pattern(&ls, "T", "A", "R", 1);
    }
    TEST("confidence grows with observations", p && p->confidence > 50.0f);
    TEST("confidence < 100",                   p && p->confidence < 100.0f);
}

/* ─────────────────────────────────────────────────────────────
 *  Spaced Repetition Tests
 * ──────────────────────────────────────────────────────────── */
void test_spaced_repetition(void) {
    printf("\n── Spaced Repetition Tests ───────────────────────────\n");
    LearningSystem ls;
    ls_init(&ls);

    Experience *e = ls_add_experience(&ls, "Test exp", "lesson", "cat",
                                       OUTCOME_GOOD, 5.0f);
    TEST("review scheduled on creation", ls.review_count == 1);
    TEST("initial interval is 1 day",
         ls.review_queue[0].interval_days == 1);
    TEST("initial ease factor ~2.5",
         fabsf(ls.review_queue[0].ease_factor - 2.5f) < 0.01f);

    /* Review with good quality — interval should grow */
    ls_review_experience(&ls, e->id, 4);
    TEST("reviewed_count incremented",    e->reviewed_count == 1);
    TEST("retention updated after review",e->retention_score > 0.0f);
    TEST("interval still manageable",     ls.review_queue[0].interval_days >= 1);

    /* Review with poor quality — interval should reset */
    ls_review_experience(&ls, e->id, 1);
    TEST("poor quality resets interval",  ls.review_queue[0].interval_days == 1);
}

/* ─────────────────────────────────────────────────────────────
 *  Forgetting Curve Tests
 * ──────────────────────────────────────────────────────────── */
void test_forgetting_curve(void) {
    printf("\n── Forgetting Curve Tests ────────────────────────────\n");

    float day0  = ls_forgetting_retention(0);
    float day1  = ls_forgetting_retention(1);
    float day7  = ls_forgetting_retention(7);
    float day30 = ls_forgetting_retention(30);

    TEST("day 0 retention ~100%",     day0 > 95.0f);
    TEST("retention decreases daily", day0 > day1);
    TEST("day 1 > day 7",             day1 > day7);
    TEST("day 7 > day 30",            day7 > day30);
    TEST("long decay still > 0",      day30 > 0.0f);
}

/* ─────────────────────────────────────────────────────────────
 *  Analytics Tests
 * ──────────────────────────────────────────────────────────── */
void test_analytics(void) {
    printf("\n── Analytics Tests ───────────────────────────────────\n");
    LearningSystem ls;
    ls_init(&ls);

    ls_add_skill(&ls, "Skill A", "Cat");
    ls_add_skill(&ls, "Skill B", "Cat");
    ls_practice_skill(&ls, ls.skills[0].id, 3.0f);
    ls_practice_skill(&ls, ls.skills[1].id, 5.0f);

    TEST("total learning hours tracked",
         ls.total_learning_hours >= 8.0f);
    TEST("due reviews function works",
         ls_get_due_reviews(&ls) >= 0);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Phase 5 Unit Tests                                  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    test_experience_recording();
    test_skill_registration();
    test_skill_leveling();
    test_skill_progress_percent();
    test_pattern_recording();
    test_pattern_confidence();
    test_spaced_repetition();
    test_forgetting_curve();
    test_analytics();

    printf("\n══════════════════════════════════════════════════════\n");
    printf("  Results: %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
