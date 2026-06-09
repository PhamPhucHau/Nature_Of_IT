/*
 * test_phase1.c  —  Phase 1 Unit Tests
 *
 * Build:  gcc -Wall -Wextra -o test1 test_phase1.c ../src/task.c ../src/energy.c -lm && ./test1
 */

#include "../include/task.h"
#include "../include/energy.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Simple test framework */
static int tests_run = 0, tests_passed = 0;
#define TEST(desc, expr) do {                              \
    tests_run++;                                           \
    if (expr) { tests_passed++; printf("  ✓ %s\n",desc); }\
    else { printf("  ✗ FAIL: %s  (line %d)\n",desc,__LINE__); } \
} while(0)

void test_task_creation(void) {
    printf("\n── Task Creation Tests ──────────────────────────────\n");
    TaskManager tm;
    task_manager_init(&tm);

    Task *t = task_create(&tm, "Test Task", "desc", PRIORITY_HIGH, 30.0f, 10.0f);
    TEST("task created successfully", t != NULL);
    TEST("task has non-zero id",      t && t->id > 0);
    TEST("task status is PENDING",    t && t->status == TASK_PENDING);
    TEST("task priority is HIGH",     t && t->priority == PRIORITY_HIGH);
    TEST("task count incremented",    tm.count == 1);

    Task *t2 = task_create(&tm, "Task2", NULL, PRIORITY_LOW, 15.0f, 5.0f);
    TEST("second task created",       t2 != NULL);
    TEST("IDs are unique",            t && t2 && t->id != t2->id);
    TEST("count is 2",                tm.count == 2);
}

void test_task_lifecycle(void) {
    printf("\n── Task Lifecycle Tests ─────────────────────────────\n");
    TaskManager tm;
    task_manager_init(&tm);

    Task *t = task_create(&tm, "Lifecycle", "test", PRIORITY_MEDIUM, 60.0f, 20.0f);
    TEST("task starts PENDING",   t && t->status == TASK_PENDING);

    task_start(&tm, t->id);
    TEST("task is RUNNING",       t && t->status == TASK_RUNNING);
    TEST("started_at recorded",   t && t->started_at > 0);

    task_complete(&tm, t->id);
    TEST("task is COMPLETED",     t && t->status == TASK_COMPLETED);
    TEST("progress is 100%",      t && t->progress_percent == 100.0f);
    TEST("completed_at recorded", t && t->completed_at > 0);
}

void test_task_pause(void) {
    printf("\n── Task Pause Tests ─────────────────────────────────\n");
    TaskManager tm;
    task_manager_init(&tm);

    Task *t = task_create(&tm, "Pause Me", NULL, PRIORITY_LOW, 45.0f, 15.0f);
    task_start(&tm, t->id);
    TEST("running before pause",   t && t->status == TASK_RUNNING);

    task_pause(&tm, t->id);
    TEST("task is PAUSED",         t && t->status == TASK_PAUSED);

    task_start(&tm, t->id);
    TEST("can restart paused task",t && t->status == TASK_RUNNING);
}

void test_task_urgency(void) {
    printf("\n── Urgency Score Tests ──────────────────────────────\n");
    TaskManager tm;
    task_manager_init(&tm);

    Task *high   = task_create(&tm, "High", NULL, PRIORITY_HIGH,     30.0f, 0);
    Task *medium = task_create(&tm, "Med",  NULL, PRIORITY_MEDIUM,   30.0f, 0);
    Task *low    = task_create(&tm, "Low",  NULL, PRIORITY_LOW,      30.0f, 0);

    TEST("HIGH urgency > MEDIUM",  task_urgency_score(high) > task_urgency_score(medium));
    TEST("MEDIUM urgency > LOW",   task_urgency_score(medium) > task_urgency_score(low));

    /* Test overdue adds urgency */
    Task *overdue = task_create(&tm, "Overdue", NULL, PRIORITY_MEDIUM, 30.0f, 0);
    overdue->deadline = time(NULL) - 3600;  /* 1 hour in the past */
    TEST("overdue task more urgent than non-overdue medium",
         task_urgency_score(overdue) > task_urgency_score(medium));
}

void test_energy_init(void) {
    printf("\n── Energy Init Tests ────────────────────────────────\n");
    EnergyState es;
    energy_init(&es, 80.0f, 70.0f, 75.0f);

    TEST("mental at capacity",    es.mental    == es.mental_capacity);
    TEST("physical at capacity",  es.physical  == es.physical_capacity);
    TEST("emotional at capacity", es.emotional == es.emotional_capacity);
    TEST("overall is ~100%",      energy_get_overall(&es) > 95.0f);
    TEST("not critical",          !energy_is_critical(&es));
    TEST("efficiency = 1.0",      energy_efficiency_factor(&es) == 1.0f);
}

void test_energy_consume(void) {
    printf("\n── Energy Consumption Tests ─────────────────────────\n");
    EnergyState es;
    energy_init(&es, 100.0f, 100.0f, 100.0f);

    energy_consume(&es, 30.0f, 20.0f, 10.0f);
    TEST("mental reduced",    es.mental    == 70.0f);
    TEST("physical reduced",  es.physical  == 80.0f);
    TEST("emotional reduced", es.emotional == 90.0f);

    /* Can't go below zero */
    energy_consume(&es, 200.0f, 0.0f, 0.0f);
    TEST("mental doesn't go below 0", es.mental == ENERGY_MIN);
}

void test_energy_recovery(void) {
    printf("\n── Energy Recovery Tests ────────────────────────────\n");
    EnergyState es;
    energy_init(&es, 100.0f, 100.0f, 100.0f);

    energy_consume(&es, 50.0f, 50.0f, 50.0f);
    float mental_before = es.mental;

    energy_recover(&es, 4.0f);  /* 4 hours rest */
    TEST("mental recovered after rest",   es.mental > mental_before);
    TEST("cannot exceed capacity",        es.mental <= es.mental_capacity);

    /* Full rest */
    energy_recover(&es, 24.0f);
    TEST("full rest restores to capacity",
         es.mental >= es.mental_capacity * 0.95f);
}

void test_energy_efficiency(void) {
    printf("\n── Energy Efficiency Tests ──────────────────────────\n");
    EnergyState es;
    energy_init(&es, 100.0f, 100.0f, 100.0f);

    TEST("full energy = 1.0 efficiency",  energy_efficiency_factor(&es) == 1.0f);

    energy_consume(&es, 50.0f, 50.0f, 50.0f);
    TEST("50% energy = 0.75 efficiency",  energy_efficiency_factor(&es) == 0.75f);

    energy_consume(&es, 20.0f, 20.0f, 20.0f);
    TEST("low energy = 0.50 efficiency",  energy_efficiency_factor(&es) == 0.50f);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Phase 1 Unit Tests                                  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    test_task_creation();
    test_task_lifecycle();
    test_task_pause();
    test_task_urgency();
    test_energy_init();
    test_energy_consume();
    test_energy_recovery();
    test_energy_efficiency();

    printf("\n══════════════════════════════════════════════════════\n");
    printf("  Results: %d/%d passed\n", tests_passed, tests_run);
    if (tests_passed == tests_run) {
        printf("  ✅ All tests passed!\n");
        return 0;
    } else {
        printf("  ❌ %d test(s) failed\n", tests_run - tests_passed);
        return 1;
    }
}
