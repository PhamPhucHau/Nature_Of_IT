/*
 * decision.c  —  Phase 4: Decision-Making Engine Implementation
 *
 * Build:  gcc -Wall -Wextra -o phase4 main.c decision.c -lm && ./phase4
 *
 * Decision-making is one of the most expensive cognitive operations.
 * This system reduces decision fatigue by applying algorithms.
 *
 * OS analogy: Just as an OS uses a scheduler policy to decide
 * which process gets CPU time, we use decision algorithms to
 * choose which action to take.
 */

#include "../include/decision.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ─────────────────────────────────────────────────────────────
 *  DecisionEngine
 * ──────────────────────────────────────────────────────────── */
void de_init(DecisionEngine *de, float energy, float available_hours) {
    memset(de, 0, sizeof(DecisionEngine));
    de->current_energy   = energy;
    de->current_time     = available_hours;
    de->best_option_idx  = -1;
}

void de_add_criterion(DecisionEngine *de, const char *name, float weight) {
    if (!de || de->criteria_count >= MAX_CRITERIA) return;
    Criterion *c = &de->criteria[de->criteria_count++];
    strncpy(c->name, name, 31);
    c->weight = weight;
}

void de_add_option(DecisionEngine *de, const char *name,
                    float energy_cost, float time_cost,
                    float expected_value, float scores[]) {
    if (!de || de->option_count >= MAX_OPTIONS) return;
    DecisionOption *opt = &de->options[de->option_count++];
    memset(opt, 0, sizeof(DecisionOption));

    strncpy(opt->name,           name,  MAX_OPTION_NAME - 1);
    opt->energy_cost       = energy_cost;
    opt->time_cost_hours   = time_cost;
    opt->expected_value    = expected_value;

    /* Copy scores for each criterion */
    for (int i = 0; i < de->criteria_count && i < MAX_CRITERIA; i++) {
        opt->scores[i] = scores[i];
    }

    /* Check feasibility */
    opt->feasible = 1;
    if (energy_cost > de->current_energy) {
        opt->feasible = 0;
        snprintf(opt->reason, 127, "Insufficient energy (need %.0f, have %.0f)",
                 energy_cost, de->current_energy);
    } else if (time_cost > de->current_time) {
        opt->feasible = 0;
        snprintf(opt->reason, 127, "Insufficient time (need %.1fh, have %.1fh)",
                 time_cost, de->current_time);
    }
}

/* ─────────────────────────────────────────────────────────────
 *  de_evaluate() — compute weighted scores and find best option
 *
 *  OS: Like the CFS scheduler computing vruntime for each process.
 *  Algorithm: weighted sum of normalized scores.
 *
 *  score(option) = Σ(criteria[i].weight × option.scores[i])
 *                  × feasibility_multiplier
 *                  × energy_availability_bonus
 * ──────────────────────────────────────────────────────────── */
int de_evaluate(DecisionEngine *de) {
    if (!de || de->option_count == 0) return -1;

    float best_score = -1.0f;
    de->best_option_idx = 0;

    for (int i = 0; i < de->option_count; i++) {
        DecisionOption *opt = &de->options[i];

        /* Compute weighted total score */
        float total = 0.0f;
        for (int j = 0; j < de->criteria_count; j++) {
            /* Normalize score to 0–1 range (assumes input is 0–10) */
            total += de->criteria[j].weight * (opt->scores[j] / 10.0f);
        }

        /* Add expected value bonus (ROI component) */
        total += (opt->expected_value / 100.0f) * 0.2f;

        /* Penalty for feasibility failure */
        if (!opt->feasible) total *= 0.1f;

        /* Penalty for high resource cost */
        float cost_penalty = (opt->energy_cost / 100.0f + opt->time_cost_hours / 8.0f) * 0.1f;
        total -= cost_penalty;

        opt->total_score = fmaxf(0.0f, total);

        if (opt->total_score > best_score) {
            best_score = opt->total_score;
            de->best_option_idx = i;
        }
    }

    return de->best_option_idx;
}

/* ─────────────────────────────────────────────────────────────
 *  de_print_results() — show decision analysis table
 * ──────────────────────────────────────────────────────────── */
void de_print_results(const DecisionEngine *de) {
    if (!de) return;

    printf("\n  Decision Analysis Results:\n");
    printf("  %-30s  %7s  %6s  %5s  %8s\n",
           "Option", "Score", "Energy", "Time", "Feasible");
    printf("  %-30s  %7s  %6s  %5s  %8s\n",
           "──────────────────────────────",
           "───────", "──────", "─────", "────────");

    for (int i = 0; i < de->option_count; i++) {
        const DecisionOption *opt = &de->options[i];
        char star = (i == de->best_option_idx) ? '*' : ' ';
        printf("  %c %-28s  %7.3f  %6.0f  %5.1fh  %s\n",
               star, opt->name,
               opt->total_score,
               opt->energy_cost,
               opt->time_cost_hours,
               opt->feasible ? "YES" : "NO");
        if (!opt->feasible)
            printf("       ↳ %s\n", opt->reason);
    }

    if (de->best_option_idx >= 0) {
        printf("\n  ★ BEST CHOICE: %s (score=%.3f)\n",
               de->options[de->best_option_idx].name,
               de->options[de->best_option_idx].total_score);
    }
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Eisenhower Matrix
 *
 *  The Eisenhower Matrix categorizes tasks by:
 *    - Urgency:    time-sensitive, deadline-driven
 *    - Importance: aligned with long-term goals
 *
 *  OS analogy: real-time vs best-effort, high vs low priority.
 *
 *  Quadrants:
 *    Q1 (Urgent + Important)    → DO NOW   (fires/crises)
 *    Q2 (Not Urgent + Important)→ SCHEDULE (growth, planning)
 *    Q3 (Urgent + Not Important)→ DELEGATE (interruptions)
 *    Q4 (Neither)               → ELIMINATE(busywork)
 * ──────────────────────────────────────────────────────────── */
void eisenhower_classify(float urgency, float importance,
                          char *quadrant_out, char *action_out) {
    int u = (urgency   >= 5.0f);
    int i = (importance >= 5.0f);

    if (u && i) {
        if (quadrant_out) strncpy(quadrant_out, "Q1: Urgent+Important",    63);
        if (action_out)   strncpy(action_out,   "DO NOW (crisis mode)",     63);
    } else if (!u && i) {
        if (quadrant_out) strncpy(quadrant_out, "Q2: Not Urgent+Important", 63);
        if (action_out)   strncpy(action_out,   "SCHEDULE (invest here)",   63);
    } else if (u && !i) {
        if (quadrant_out) strncpy(quadrant_out, "Q3: Urgent+Not Important", 63);
        if (action_out)   strncpy(action_out,   "DELEGATE if possible",     63);
    } else {
        if (quadrant_out) strncpy(quadrant_out, "Q4: Neither",              63);
        if (action_out)   strncpy(action_out,   "ELIMINATE/minimize",       63);
    }
}

void eisenhower_matrix_demo(void) {
    struct {
        const char *task;
        float urgency;
        float importance;
    } tasks[] = {
        { "Production server down",      9.5f, 10.0f },
        { "Write Q4 strategy plan",      3.0f,  9.5f },
        { "Respond to non-critical emails",7.0f, 3.0f },
        { "Scroll social media",          2.0f,  1.0f },
        { "Exercise / health",            3.5f,  9.0f },
        { "Meeting that could be email",  7.0f,  2.5f },
        { "Learn new programming skill",  2.0f,  8.5f },
        { "Fix critical security bug",    9.0f,  9.5f },
    };

    printf("\n  ┌──────────────────────────────────────────────────────\n");
    printf("  │         EISENHOWER MATRIX ANALYSIS\n");
    printf("  │  %-35s  %-25s  %s\n",
           "Task", "Quadrant", "Action");
    printf("  │  %-35s  %-25s  %s\n",
           "───────────────────────────────────",
           "─────────────────────────",
           "──────────────────────");

    for (int i = 0; i < 8; i++) {
        char q[64], a[64];
        eisenhower_classify(tasks[i].urgency, tasks[i].importance, q, a);
        printf("  │  %-35s  %-25s  %s\n", tasks[i].task, q, a);
    }
    printf("  └──────────────────────────────────────────────────────\n\n");

    printf("  KEY INSIGHT: Spend 60-70%% of time in Q2 (Important, Not Urgent)\n");
    printf("  Q2 is where growth, learning, and meaningful work happens.\n");
    printf("  Most people spend too much time in Q1 (crises) and Q3 (other priorities).\n\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Cost-Benefit Analysis
 * ──────────────────────────────────────────────────────────── */
float cost_benefit_ratio(float benefit, float cost,
                           float time_hours, float energy_units) {
    float total_cost = cost + (time_hours * 5.0f) + (energy_units * 0.5f);
    return total_cost > 0 ? benefit / total_cost : 0.0f;
}

/* ─────────────────────────────────────────────────────────────
 *  Life Manager — Goals and Habits
 * ──────────────────────────────────────────────────────────── */
void lm_init(LifeManager *lm) {
    memset(lm, 0, sizeof(LifeManager));
    lm->next_id = 1;
    printf("[LifeManager] Initialized — goal & habit tracking ready\n");
}

const char* goal_category_name(GoalCategory c) {
    switch (c) {
        case GOAL_CAREER:   return "Career";
        case GOAL_HEALTH:   return "Health";
        case GOAL_LEARNING: return "Learning";
        case GOAL_FINANCE:  return "Finance";
        case GOAL_SOCIAL:   return "Social";
        case GOAL_PERSONAL: return "Personal";
        default:            return "Other";
    }
}

/* ─────────────────────────────────────────────────────────────
 *  Goals
 * ──────────────────────────────────────────────────────────── */
Goal* lm_add_goal(LifeManager *lm, const char *name,
                   GoalCategory cat, float target, float importance) {
    if (!lm || lm->goal_count >= MAX_GOALS) return NULL;
    Goal *g = &lm->goals[lm->goal_count++];
    memset(g, 0, sizeof(Goal));

    g->id             = lm->next_id++;
    g->category       = cat;
    g->target_value   = target;
    g->current_value  = 0.0f;
    g->importance     = importance;
    g->active         = 1;
    strncpy(g->name, name, MAX_GOAL_NAME - 1);

    printf("[Goal] Created: '%s' (%s, importance=%.1f)\n",
           name, goal_category_name(cat), importance);
    return g;
}

void lm_update_goal(LifeManager *lm, uint32_t id, float progress) {
    if (!lm) return;
    for (int i = 0; i < lm->goal_count; i++) {
        if (lm->goals[i].id == id) {
            float old = lm->goals[i].current_value;
            lm->goals[i].current_value = fminf(
                lm->goals[i].target_value,
                lm->goals[i].current_value + progress);
            printf("[Goal] Updated '%s': %.1f → %.1f / %.1f\n",
                   lm->goals[i].name, old,
                   lm->goals[i].current_value,
                   lm->goals[i].target_value);
            return;
        }
    }
}

/* Check how well a task aligns with current goals.
 * Returns alignment score 0.0–1.0.
 * OS: checking if a process serves a high-priority SLA */
float lm_check_goal_alignment(const LifeManager *lm, const char *task_category) {
    if (!lm || !task_category) return 0.5f;

    float total_weight = 0.0f, aligned_weight = 0.0f;
    for (int i = 0; i < lm->goal_count; i++) {
        if (!lm->goals[i].active) continue;
        total_weight += lm->goals[i].importance;

        /* Check if task category matches goal category */
        const char *gcat = goal_category_name(lm->goals[i].category);
        if (strncasecmp(gcat, task_category, strlen(task_category)) == 0) {
            float progress_pct = lm->goals[i].target_value > 0 ?
                lm->goals[i].current_value / lm->goals[i].target_value : 0;
            /* More weight to less-complete goals */
            aligned_weight += lm->goals[i].importance * (1.0f - progress_pct * 0.5f);
        }
    }
    return total_weight > 0 ? aligned_weight / total_weight : 0.5f;
}

void lm_print_goals(const LifeManager *lm) {
    if (!lm) return;
    printf("\n  Goals Progress:\n");
    printf("  %-30s  %-10s  %8s  %8s  %7s\n",
           "Goal", "Category", "Progress", "Target", "Import");
    for (int i = 0; i < lm->goal_count; i++) {
        const Goal *g = &lm->goals[i];
        if (!g->active) continue;
        float pct = g->target_value > 0 ?
                    g->current_value / g->target_value * 100.0f : 0.0f;
        printf("  %-30s  %-10s  %7.1f%%  %7.1f   %.1f/10\n",
               g->name, goal_category_name(g->category),
               pct, g->target_value, g->importance);
    }
    printf("\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Habits — recurring scheduled behaviors
 *  OS: cron jobs / periodic kernel tasks
 * ──────────────────────────────────────────────────────────── */
Habit* lm_add_habit(LifeManager *lm, const char *name,
                     float freq_per_week, int is_positive) {
    if (!lm || lm->habit_count >= MAX_HABITS) return NULL;
    Habit *h = &lm->habits[lm->habit_count++];
    memset(h, 0, sizeof(Habit));

    h->id               = lm->next_id++;
    h->frequency_per_week = freq_per_week;
    h->is_positive      = is_positive;
    h->completion_rate  = 0.0f;
    strncpy(h->name, name, MAX_HABIT_NAME - 1);

    printf("[Habit] Registered: '%s' (%s, %.0fx/week)\n",
           name, is_positive ? "BUILD" : "BREAK", freq_per_week);
    return h;
}

void lm_habit_done(LifeManager *lm, uint32_t habit_id) {
    if (!lm) return;
    for (int i = 0; i < lm->habit_count; i++) {
        Habit *h = &lm->habits[i];
        if (h->id == habit_id) {
            h->completions_this_week++;
            h->streak_days++;
            if (h->streak_days > h->best_streak)
                h->best_streak = h->streak_days;
            h->last_completed = time(NULL);
            h->completion_rate = fminf(100.0f,
                (float)h->completions_this_week / h->frequency_per_week * 100.0f);
            printf("[Habit] Done: '%s' — streak=%d days (best=%d)\n",
                   h->name, h->streak_days, h->best_streak);
            return;
        }
    }
}

void lm_print_habits(const LifeManager *lm) {
    if (!lm) return;
    printf("\n  Habit Tracker:\n");
    printf("  %-30s  %-5s  %-6s  %6s  %5s\n",
           "Habit", "Type", "Done/Wk", "Streak", "Rate%");
    for (int i = 0; i < lm->habit_count; i++) {
        const Habit *h = &lm->habits[i];
        printf("  %-30s  %-5s  %3d/%-3.0f  %6d  %4.0f%%\n",
               h->name,
               h->is_positive ? "BUILD" : "BREAK",
               h->completions_this_week,
               h->frequency_per_week,
               h->streak_days,
               h->completion_rate);
    }
    printf("\n");
}
