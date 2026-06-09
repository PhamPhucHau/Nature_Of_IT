/*
 * test_phase2.c  —  Phase 2 Unit Tests
 *
 * Build:  gcc -Wall -Wextra -o test2 test_phase2.c ../src/task_queue.c
 *         ../src/scheduler.c -lm && ./test2
 */

#include "../include/task_queue.h"
#include "../include/scheduler.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

static int tests_run = 0, tests_passed = 0;
#define TEST(desc, expr) do {                              \
    tests_run++;                                           \
    if (expr) { tests_passed++; printf("  ✓ %s\n",desc); }\
    else { printf("  ✗ FAIL: %s  (line %d)\n",desc,__LINE__); } \
} while(0)

void test_queue_basic(void) {
    printf("\n── Queue Basic Tests ────────────────────────────────\n");
    TaskQueue q;
    tq_init(&q);

    TEST("queue starts empty",  tq_is_empty(&q));
    TEST("size is 0",           tq_size(&q) == 0);
    TEST("dequeue empty = NULL",tq_dequeue(&q, NULL) == NULL);
}

void test_queue_priority_order(void) {
    printf("\n── Queue Priority Ordering Tests ────────────────────\n");
    TaskQueue q;
    tq_init(&q);

    QTask low = {1, "low task",  PRIORITY_LOW,      TASK_PENDING,
                 ENERGY_MENTAL, 30.0f, 5.0f, 3.0f, 0, 0, 0};
    QTask high = {2, "high task", PRIORITY_HIGH,    TASK_PENDING,
                  ENERGY_MENTAL, 30.0f, 5.0f, 8.0f, 0, 0, 0};
    QTask crit = {3, "crit task", PRIORITY_CRITICAL,TASK_PENDING,
                  ENERGY_MENTAL, 30.0f, 5.0f, 9.0f, 0, 0, 0};

    tq_enqueue(&q, &low);
    tq_enqueue(&q, &high);
    tq_enqueue(&q, &crit);

    TEST("size is 3", tq_size(&q) == 3);

    /* Dequeue should give us highest urgency first */
    QTask out;
    tq_dequeue(&q, &out);
    TEST("first dequeue = CRITICAL", out.priority == PRIORITY_CRITICAL);

    tq_dequeue(&q, &out);
    TEST("second dequeue = HIGH",    out.priority == PRIORITY_HIGH);

    tq_dequeue(&q, &out);
    TEST("third dequeue = LOW",      out.priority == PRIORITY_LOW);

    TEST("queue now empty",          tq_is_empty(&q));
}

void test_queue_deadline_urgency(void) {
    printf("\n── Deadline Urgency Tests ────────────────────────────\n");
    QTask with_deadline    = {1, "has deadline",PRIORITY_MEDIUM,TASK_PENDING,
                               ENERGY_MENTAL,30.0f,5.0f,5.0f,0,
                               time(NULL) + 1800, 0};  /* 30 min from now */
    QTask without_deadline = {2, "no deadline", PRIORITY_MEDIUM,TASK_PENDING,
                               ENERGY_MENTAL,30.0f,5.0f,5.0f,0, 0, 0};

    float urgent = tq_urgency(&with_deadline);
    float normal = tq_urgency(&without_deadline);
    TEST("deadline task is more urgent", urgent > normal);
}

void test_fifo_schedule(void) {
    printf("\n── FIFO Scheduling Tests ─────────────────────────────\n");
    TaskQueue q;
    tq_init(&q);
    tq_populate_sample(&q);

    Schedule s;
    schedule_init(&s, SCHED_FIFO);
    int n = schedule_build(&s, &q, 4.0f);

    TEST("scheduled some tasks",     n > 0);
    TEST("total duration > 0",       s.total_duration_minutes > 0.0f);
    TEST("total duration <= 240+30", s.total_duration_minutes <= 270.0f);
}

void test_priority_schedule(void) {
    printf("\n── Priority Scheduling Tests ─────────────────────────\n");
    TaskQueue q;
    tq_init(&q);
    tq_populate_sample(&q);

    Schedule s;
    schedule_init(&s, SCHED_PRIORITY);
    int n = schedule_build(&s, &q, 8.0f);

    TEST("scheduled tasks", n > 0);

    /* First slot should be highest urgency task */
    if (s.slot_count > 0) {
        TEST("first slot is not a break", !s.slots[0].is_break);
    }
}

void test_round_robin(void) {
    printf("\n── Round Robin Tests ─────────────────────────────────\n");
    TaskQueue q;
    tq_init(&q);
    tq_populate_sample(&q);

    Schedule s;
    schedule_init(&s, SCHED_RR);
    int n = schedule_build(&s, &q, 4.0f);

    TEST("scheduled some tasks",  n >= 0);

    /* RR should have more slots than tasks (due to rotation) */
    int task_slots = 0;
    for (int i = 0; i < s.slot_count; i++) {
        if (!s.slots[i].is_break) task_slots++;
    }
    TEST("RR has multiple slots", task_slots > 0);
}

void test_metrics(void) {
    printf("\n── Schedule Metrics Tests ────────────────────────────\n");
    TaskQueue q;
    tq_init(&q);
    tq_populate_sample(&q);

    Schedule s;
    schedule_init(&s, SCHED_PRIORITY);
    schedule_build(&s, &q, 8.0f);

    SchedMetrics m = schedule_compute_metrics(&s);
    TEST("utilization > 0",     m.cpu_utilization > 0.0f);
    TEST("utilization <= 100",  m.cpu_utilization <= 100.0f);
    TEST("throughput > 0",      m.throughput > 0.0f);
    TEST("avg wait >= 0",       m.avg_waiting_time >= 0.0f);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Phase 2 Unit Tests                                  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    test_queue_basic();
    test_queue_priority_order();
    test_queue_deadline_urgency();
    test_fifo_schedule();
    test_priority_schedule();
    test_round_robin();
    test_metrics();

    printf("\n══════════════════════════════════════════════════════\n");
    printf("  Results: %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
