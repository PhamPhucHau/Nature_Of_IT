/*
 * decision.h  —  Phase 4: Decision-Making Engine
 *
 * OS Analogy: Interrupt handlers and policy engines.
 *
 *   When an OS receives an interrupt, it must decide:
 *     1. Is this interrupt important enough to handle now?
 *     2. Which handler to invoke?
 *     3. Should the current task be preempted?
 *
 *   Human OS decision-making mirrors this:
 *     1. Evaluate options (like interrupt priority)
 *     2. Apply decision criteria (like interrupt routing table)
 *     3. Choose best action (like dispatch to handler)
 *
 * Decision algorithms implemented:
 *   - Weighted scoring (multi-criteria decision analysis)
 *   - Eisenhower Matrix (urgent/important quadrants)
 *   - Cost-benefit analysis (ROI calculation)
 *   - Decision tree (conditional logic)
 *   - Goal alignment check (strategic filter)
 */

#ifndef DECISION_H
#define DECISION_H

#include <stdint.h>
#include <time.h>

/* ─────────────────────────────────────────────────────────────
 *  CONSTANTS
 * ──────────────────────────────────────────────────────────── */
#define MAX_OPTIONS      20
#define MAX_CRITERIA      8
#define MAX_GOALS        10
#define MAX_HABITS       20
#define MAX_OPTION_NAME  64
#define MAX_GOAL_NAME    64
#define MAX_HABIT_NAME   64

/* ─────────────────────────────────────────────────────────────
 *  DECISION OPTION — one choice to evaluate
 *  OS: like one candidate process for CPU dispatch
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    char    name[MAX_OPTION_NAME];
    float   scores[MAX_CRITERIA];   /* raw scores for each criterion   */
    float   total_score;            /* weighted final score            */
    float   energy_cost;            /* energy needed to execute        */
    float   time_cost_hours;        /* time investment required        */
    float   expected_value;         /* expected benefit/ROI            */
    int     feasible;               /* 1 = can do it now               */
    char    reason[128];            /* why this is/isn't feasible      */
} DecisionOption;

/* ─────────────────────────────────────────────────────────────
 *  DECISION CRITERIA — what dimensions to evaluate
 *  OS: interrupt priority levels and routing rules
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    char  name[32];
    float weight;    /* 0.0–1.0, all weights should sum to 1.0 */
} Criterion;

/* ─────────────────────────────────────────────────────────────
 *  DECISION ENGINE — evaluates options using weighted criteria
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    Criterion      criteria[MAX_CRITERIA];
    int            criteria_count;
    DecisionOption options[MAX_OPTIONS];
    int            option_count;
    int            best_option_idx;  /* index of winner */
    float          current_energy;   /* available energy */
    float          current_time;     /* available hours today */
} DecisionEngine;

/* ─────────────────────────────────────────────────────────────
 *  GOAL — a desired outcome to optimize toward
 *  OS: system performance targets / SLAs
 * ──────────────────────────────────────────────────────────── */
typedef enum {
    GOAL_CAREER    = 0,
    GOAL_HEALTH    = 1,
    GOAL_LEARNING  = 2,
    GOAL_FINANCE   = 3,
    GOAL_SOCIAL    = 4,
    GOAL_PERSONAL  = 5,
} GoalCategory;

typedef struct {
    uint32_t     id;
    char         name[MAX_GOAL_NAME];
    GoalCategory category;
    float        target_value;     /* what "done" looks like (0–100)   */
    float        current_value;    /* current progress (0–100)         */
    time_t       deadline;
    float        importance;       /* 0–10 how important this is       */
    int          active;           /* 1 = currently pursuing           */
} Goal;

/* ─────────────────────────────────────────────────────────────
 *  HABIT — a recurring behavior pattern
 *  OS: scheduled recurring processes (cron jobs)
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    uint32_t id;
    char     name[MAX_HABIT_NAME];
    float    frequency_per_week;    /* how often per week (e.g. 5 = weekdays) */
    int      completions_this_week; /* how many times done this week          */
    int      streak_days;           /* current consecutive day streak         */
    int      best_streak;           /* longest ever streak                    */
    float    completion_rate;       /* % of scheduled times actually done     */
    time_t   last_completed;
    int      is_positive;           /* 1 = build this habit, 0 = break it     */
} Habit;

/* ─────────────────────────────────────────────────────────────
 *  GOAL & HABIT MANAGER
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    Goal   goals[MAX_GOALS];
    int    goal_count;
    Habit  habits[MAX_HABITS];
    int    habit_count;
    uint32_t next_id;
} LifeManager;

/* ─────────────────────────────────────────────────────────────
 *  FUNCTION DECLARATIONS
 * ──────────────────────────────────────────────────────────── */

/* Decision Engine */
void   de_init(DecisionEngine *de, float energy, float available_hours);
void   de_add_criterion(DecisionEngine *de, const char *name, float weight);
void   de_add_option(DecisionEngine *de, const char *name,
                      float energy_cost, float time_cost,
                      float expected_value, float scores[]);
int    de_evaluate(DecisionEngine *de);
void   de_print_results(const DecisionEngine *de);

/* Eisenhower Matrix */
void   eisenhower_classify(float urgency, float importance,
                            char *quadrant_out, char *action_out);
void   eisenhower_matrix_demo(void);

/* Cost-Benefit Analysis */
float  cost_benefit_ratio(float benefit, float cost,
                           float time_hours, float energy_units);

/* Goals */
void   lm_init(LifeManager *lm);
Goal*  lm_add_goal(LifeManager *lm, const char *name,
                    GoalCategory cat, float target, float importance);
void   lm_update_goal(LifeManager *lm, uint32_t id, float progress);
float  lm_check_goal_alignment(const LifeManager *lm, const char *task_category);
void   lm_print_goals(const LifeManager *lm);

/* Habits */
Habit* lm_add_habit(LifeManager *lm, const char *name,
                     float freq_per_week, int is_positive);
void   lm_habit_done(LifeManager *lm, uint32_t habit_id);
void   lm_print_habits(const LifeManager *lm);

/* Display */
const char* goal_category_name(GoalCategory c);

#endif /* DECISION_H */
