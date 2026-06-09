/*
 * main.c  —  Phase 5: Learning System Demo
 *
 * Build:  gcc -Wall -Wextra -o phase5 main.c learning.c -lm && ./phase5
 *
 * Simulates 3 months of deliberate skill development:
 *   - Recording experiences and lessons
 *   - Tracking skill progression with XP system
 *   - Pattern recognition from repeated actions
 *   - Spaced repetition for retention
 */

#include "../include/learning.h"
#include <stdio.h>

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Human OS — Phase 5: Learning & Improvement System  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    LearningSystem ls;
    ls_init(&ls);

    /* ── 1. Register skills to track ──────────────────────────── */
    printf("=== Registering Skills ===\n");

    Skill *c_prog     = ls_add_skill(&ls, "C Programming",       "Engineering");
    Skill *systems    = ls_add_skill(&ls, "Systems Design",       "Engineering");
    Skill *writing    = ls_add_skill(&ls, "Technical Writing",    "Communication");
    Skill *leadership = ls_add_skill(&ls, "Engineering Leadership","Soft Skills");
    Skill *fitness    = ls_add_skill(&ls, "Physical Fitness",      "Health");

    /* ── 2. Record experiences (lessons from real situations) ─── */
    printf("\n=== Recording Experiences ===\n");

    ls_add_experience(&ls,
        "Debugged memory corruption bug for 3 hours",
        "Always use Valgrind from day 1. Pointer bugs compound over time.",
        "Engineering", OUTCOME_GOOD, 8.5f);

    ls_add_experience(&ls,
        "Missed project deadline due to poor estimation",
        "Add 50% buffer to all estimates. Tasks always expand to fill time.",
        "Engineering", OUTCOME_BAD, 9.0f);

    ls_add_experience(&ls,
        "Got promoted after writing clear design docs",
        "Writing is thinking. Clear writing = clear thinking = leadership.",
        "Communication", OUTCOME_GREAT, 9.5f);

    ls_add_experience(&ls,
        "Skipped gym for 2 weeks — energy crashed",
        "Physical exercise is not optional. It's maintenance for the CPU.",
        "Health", OUTCOME_TERRIBLE, 8.0f);

    ls_add_experience(&ls,
        "Mentored junior developer — learned by teaching",
        "Teaching accelerates your own learning by 2x. Teach early.",
        "Soft Skills", OUTCOME_GREAT, 9.0f);

    ls_add_experience(&ls,
        "Used OS scheduling concepts to plan workday",
        "Priority scheduling + time boxing = 40% more output per day.",
        "Personal Development", OUTCOME_GREAT, 8.0f);

    ls_print_experiences(&ls, 6);

    /* ── 3. Practice skills over 3 months ─────────────────────── */
    printf("=== Simulating 3 Months of Skill Practice ===\n\n");

    /* Month 1: Starting out — lots of C programming */
    printf("  Month 1:\n");
    for (int week = 0; week < 4; week++) {
        ls_practice_skill(&ls, c_prog->id,     5.0f);  /* 5h/week coding    */
        ls_practice_skill(&ls, writing->id,    1.0f);  /* 1h/week writing   */
        ls_practice_skill(&ls, fitness->id,    3.0f);  /* 3h/week gym       */
    }

    /* Month 2: Adding systems design and leadership */
    printf("\n  Month 2:\n");
    for (int week = 0; week < 4; week++) {
        ls_practice_skill(&ls, c_prog->id,     4.0f);
        ls_practice_skill(&ls, systems->id,    3.0f);
        ls_practice_skill(&ls, writing->id,    2.0f);
        ls_practice_skill(&ls, leadership->id, 1.0f);
        ls_practice_skill(&ls, fitness->id,    3.5f);
    }

    /* Month 3: More balanced — all skills */
    printf("\n  Month 3:\n");
    for (int week = 0; week < 4; week++) {
        ls_practice_skill(&ls, c_prog->id,     3.0f);
        ls_practice_skill(&ls, systems->id,    4.0f);
        ls_practice_skill(&ls, writing->id,    2.0f);
        ls_practice_skill(&ls, leadership->id, 2.0f);
        ls_practice_skill(&ls, fitness->id,    4.0f);
    }

    /* ── 4. Print skill progress ───────────────────────────────── */
    ls_print_skills(&ls);
    ls_print_learning_curve(c_prog);

    /* ── 5. Record behavior patterns ──────────────────────────── */
    printf("=== Recording Behavior Patterns ===\n");

    /* Record same pattern multiple times to build confidence */
    for (int i = 0; i < 5; i++) {
        ls_record_pattern(&ls,
            "Morning",
            "Exercise before starting work",
            "High energy all day, better focus",
            1);  /* success = 1 */
    }
    for (int i = 0; i < 3; i++) {
        ls_record_pattern(&ls,
            "Late night",
            "Work past midnight on complex code",
            "More bugs, slower next morning",
            0);  /* success = 0 */
    }
    for (int i = 0; i < 7; i++) {
        ls_record_pattern(&ls,
            "Pre-work",
            "Write daily plan before starting tasks",
            "Complete 30% more tasks per day",
            1);
    }
    ls_record_pattern(&ls, "Meeting prep", "Prepare agenda 24h before", "Meeting is 2x faster", 1);
    ls_record_pattern(&ls, "Meeting prep", "Prepare agenda 24h before", "Meeting is 2x faster", 1);
    ls_record_pattern(&ls, "Meeting prep", "Prepare agenda 24h before", "Meeting is 2x faster", 1);

    ls_print_patterns(&ls);

    /* ── 6. Spaced repetition review simulation ───────────────── */
    printf("=== Spaced Repetition Reviews ===\n");
    printf("  Simulating reviews of recorded experiences:\n\n");

    /* Review first experience with quality 4 (good recall) */
    ls_review_experience(&ls, 1, 4);
    ls_review_experience(&ls, 2, 5);  /* perfect recall */
    ls_review_experience(&ls, 3, 2);  /* poor recall — resets interval */

    ls_print_due_reviews(&ls);

    /* ── 7. Forgetting curve visualization ────────────────────── */
    printf("=== Ebbinghaus Forgetting Curve ===\n");
    printf("  How much you retain without review:\n");
    int days[] = {1, 3, 7, 14, 30};
    for (int i = 0; i < 5; i++) {
        float retention = ls_forgetting_retention(days[i]);
        int bars = (int)(retention / 5);
        printf("  Day %-3d: [", days[i]);
        for (int b = 0; b < 20; b++) printf(b < bars ? "█" : " ");
        printf("] %.0f%%\n", retention);
    }
    printf("\n  Review on Day 1 → resets the curve → builds long-term memory\n\n");

    /* ── 8. Final analytics ────────────────────────────────────── */
    printf("=== Learning System Analytics ===\n");
    ls_print_analytics(&ls);

    /* ── 9. Key lessons ────────────────────────────────────────── */
    printf("=== Key Learning System Lessons ===\n\n");
    printf("  1. Record experiences immediately while memory is fresh\n");
    printf("  2. Spaced repetition beats cramming for long-term retention\n");
    printf("  3. Consistent small practice beats occasional large sessions\n");
    printf("  4. Pattern recognition requires multiple data points\n");
    printf("  5. OS lesson: hardware prefetcher = spaced repetition\n");
    printf("     Both predict what knowledge you'll need and pre-load it\n");

    return 0;
}
