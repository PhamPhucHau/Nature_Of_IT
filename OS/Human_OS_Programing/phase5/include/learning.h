/*
 * learning.h  —  Phase 5: Learning & Improvement System
 *
 * OS Analogy: CPU performance counters, cache optimization,
 * and adaptive algorithms (hardware prefetcher, branch predictor).
 *
 *   Branch predictor    → predict which habit/action leads to success
 *   CPU cache           → fast access to frequently-used knowledge
 *   Performance counter → skill level tracking
 *   Prefetcher          → proactively learn skills before you need them
 *   L1/L2/L3 hierarchy  → working memory → short-term → long-term knowledge
 *
 * This module implements:
 *   1. Experience Database  — store and retrieve lessons learned
 *   2. Skill Tracker        — track competency growth over time
 *   3. Pattern Recognizer   — find what works (reinforcement learning)
 *   4. Review Scheduler     — spaced repetition (Ebbinghaus forgetting curve)
 *   5. Learning Analytics   — progress metrics and insights
 */

#ifndef LEARNING_H
#define LEARNING_H

#include <time.h>
#include <stdint.h>

/* ─────────────────────────────────────────────────────────────
 *  CONSTANTS
 * ──────────────────────────────────────────────────────────── */
#define MAX_EXPERIENCES     200
#define MAX_SKILLS           50
#define MAX_PATTERNS         100
#define MAX_REVIEW_ITEMS     200
#define MAX_EXP_TITLE        80
#define MAX_EXP_LESSON      256
#define MAX_SKILL_NAME       64
#define MAX_PATTERN_DESC    128
#define MAX_SKILL_CATEGORY   32

/* ─────────────────────────────────────────────────────────────
 *  SKILL LEVEL — Dreyfus model of skill acquisition
 *  OS analogy: CPU instruction pipeline efficiency levels
 * ──────────────────────────────────────────────────────────── */
typedef enum {
    SKILL_NOVICE        = 1,   /* aware of concept, needs guidance       */
    SKILL_BEGINNER      = 2,   /* can do with help, following rules      */
    SKILL_COMPETENT     = 3,   /* can work independently, plans ahead    */
    SKILL_PROFICIENT    = 4,   /* sees big picture, handles exceptions   */
    SKILL_EXPERT        = 5,   /* intuitive mastery, creates new rules   */
} SkillLevel;

/* ─────────────────────────────────────────────────────────────
 *  OUTCOME — result of an action/experience
 * ──────────────────────────────────────────────────────────── */
typedef enum {
    OUTCOME_GREAT    = 5,
    OUTCOME_GOOD     = 4,
    OUTCOME_NEUTRAL  = 3,
    OUTCOME_BAD      = 2,
    OUTCOME_TERRIBLE = 1,
} OutcomeRating;

/* ─────────────────────────────────────────────────────────────
 *  EXPERIENCE ENTRY — one lesson learned
 *  OS: a kernel log entry with structured metadata
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    uint32_t     id;
    char         title[MAX_EXP_TITLE];     /* what happened               */
    char         lesson[MAX_EXP_LESSON];   /* what was learned            */
    char         category[MAX_SKILL_CATEGORY]; /* skill/domain            */
    OutcomeRating outcome;                 /* how it went                 */
    float        impact_score;             /* how significant (0–10)      */
    time_t       timestamp;
    int          reviewed_count;           /* how many times reviewed     */
    time_t       next_review;              /* spaced repetition date      */
    float        retention_score;          /* 0–100 how well remembered   */
} Experience;

/* ─────────────────────────────────────────────────────────────
 *  SKILL — a tracked competency
 *  OS: hardware performance counter (instructions, cache hits, etc.)
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    uint32_t   id;
    char       name[MAX_SKILL_NAME];
    char       category[MAX_SKILL_CATEGORY];
    SkillLevel level;
    float      xp_points;          /* experience points (like XP in games) */
    float      xp_to_next_level;   /* XP needed for next level             */
    float      practice_hours;     /* total hours invested                 */
    int        daily_streak;       /* consecutive days practiced           */
    time_t     last_practiced;
    float      growth_rate;        /* XP/hour average                     */
} Skill;

/* ─────────────────────────────────────────────────────────────
 *  PATTERN — a recognized correlation between action and outcome
 *  OS: hardware prefetcher learning memory access patterns
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    uint32_t id;
    char     trigger[MAX_SKILL_CATEGORY];    /* what condition             */
    char     action[MAX_PATTERN_DESC];       /* what was done              */
    char     result[MAX_PATTERN_DESC];       /* what happened              */
    int      occurrence_count;              /* how many times seen        */
    float    success_rate;                  /* % of times outcome was good*/
    float    confidence;                    /* statistical confidence      */
} Pattern;

/* ─────────────────────────────────────────────────────────────
 *  REVIEW ITEM — spaced repetition queue entry
 *  OS: cache prefetch queue
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    uint32_t experience_id;
    time_t   due_date;
    int      interval_days;    /* current review interval */
    float    ease_factor;      /* SM-2 algorithm ease factor */
} ReviewItem;

/* ─────────────────────────────────────────────────────────────
 *  LEARNING SYSTEM — aggregates all components
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    Experience  experiences[MAX_EXPERIENCES];
    int         exp_count;

    Skill       skills[MAX_SKILLS];
    int         skill_count;

    Pattern     patterns[MAX_PATTERNS];
    int         pattern_count;

    ReviewItem  review_queue[MAX_REVIEW_ITEMS];
    int         review_count;

    uint32_t    next_id;

    /* Overall learning metrics */
    float       total_learning_hours;
    int         total_experiences;
    float       avg_retention_score;
} LearningSystem;

/* ─────────────────────────────────────────────────────────────
 *  FUNCTION DECLARATIONS
 * ──────────────────────────────────────────────────────────── */

/* System */
void       ls_init(LearningSystem *ls);

/* Experiences */
Experience* ls_add_experience(LearningSystem *ls, const char *title,
                               const char *lesson, const char *category,
                               OutcomeRating outcome, float impact);
void        ls_review_experience(LearningSystem *ls, uint32_t exp_id, int quality);
void        ls_print_experiences(const LearningSystem *ls, int top_n);

/* Skills */
Skill*      ls_add_skill(LearningSystem *ls, const char *name, const char *category);
void        ls_practice_skill(LearningSystem *ls, uint32_t skill_id, float hours);
float       ls_get_skill_progress_pct(const Skill *s);
void        ls_print_skills(const LearningSystem *ls);

/* Patterns */
Pattern*    ls_record_pattern(LearningSystem *ls, const char *trigger,
                               const char *action, const char *result,
                               int success);
void        ls_print_patterns(const LearningSystem *ls);

/* Review (Spaced Repetition) */
void        ls_schedule_review(LearningSystem *ls, uint32_t exp_id);
int         ls_get_due_reviews(const LearningSystem *ls);
void        ls_print_due_reviews(const LearningSystem *ls);

/* Analytics */
void        ls_print_analytics(const LearningSystem *ls);
void        ls_print_learning_curve(const Skill *s);
float       ls_forgetting_retention(int days_since_last_review);

/* Labels */
const char* skill_level_name(SkillLevel l);
const char* outcome_name(OutcomeRating o);

#endif /* LEARNING_H */
