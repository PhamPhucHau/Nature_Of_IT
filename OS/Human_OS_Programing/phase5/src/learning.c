/*
 * learning.c  —  Phase 5: Learning & Improvement System Implementation
 *
 * Build:  gcc -Wall -Wextra -o phase5 main.c learning.c -lm && ./phase5
 *
 * Core algorithms:
 *
 *  1. SPACED REPETITION (SM-2 algorithm)
 *     Reviews are scheduled at increasing intervals:
 *     1 day → 3 days → 7 days → 14 days → 30 days → ...
 *     OS analogy: cache eviction policy with LRU + access frequency.
 *     Items reviewed at the right time stay in "long-term memory cache".
 *
 *  2. SKILL XP SYSTEM
 *     Skills level up as practice hours accumulate.
 *     XP required doubles each level (like CPU cache sizing).
 *     Growth rate tracks learning efficiency over time.
 *
 *  3. PATTERN LEARNING
 *     Records (trigger → action → result) triples.
 *     Computes success rate and confidence as data accumulates.
 *     OS: hardware branch predictor learning from instruction history.
 *
 *  4. EBBINGHAUS FORGETTING CURVE
 *     R(t) = e^(-t/S) where:
 *       R = retention (0–1)
 *       t = time since last review (days)
 *       S = memory strength (increases with each review)
 *     OS: cache line decay — recently used data is more likely to stay.
 */

#include "../include/learning.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ─────────────────────────────────────────────────────────────
 *  Label helpers
 * ──────────────────────────────────────────────────────────── */
const char* skill_level_name(SkillLevel l) {
    switch (l) {
        case SKILL_NOVICE:     return "Novice";
        case SKILL_BEGINNER:   return "Beginner";
        case SKILL_COMPETENT:  return "Competent";
        case SKILL_PROFICIENT: return "Proficient";
        case SKILL_EXPERT:     return "Expert";
        default:               return "Unknown";
    }
}

const char* outcome_name(OutcomeRating o) {
    switch (o) {
        case OUTCOME_GREAT:    return "Great";
        case OUTCOME_GOOD:     return "Good";
        case OUTCOME_NEUTRAL:  return "Neutral";
        case OUTCOME_BAD:      return "Bad";
        case OUTCOME_TERRIBLE: return "Terrible";
        default:               return "Unknown";
    }
}

/* ─────────────────────────────────────────────────────────────
 *  ls_init()
 * ──────────────────────────────────────────────────────────── */
void ls_init(LearningSystem *ls) {
    memset(ls, 0, sizeof(LearningSystem));
    ls->next_id = 1;
    printf("[LearningSystem] Initialized — experience DB ready\n");
}

/* ─────────────────────────────────────────────────────────────
 *  ls_add_experience()
 *  Record a lesson learned from an event.
 *  OS: writing a structured log entry to kernel ring buffer.
 * ──────────────────────────────────────────────────────────── */
Experience* ls_add_experience(LearningSystem *ls, const char *title,
                               const char *lesson, const char *category,
                               OutcomeRating outcome, float impact) {
    if (!ls || ls->exp_count >= MAX_EXPERIENCES) return NULL;

    Experience *e = &ls->experiences[ls->exp_count++];
    memset(e, 0, sizeof(Experience));

    e->id             = ls->next_id++;
    e->outcome        = outcome;
    e->impact_score   = impact;
    e->timestamp      = time(NULL);
    e->reviewed_count = 0;
    e->retention_score= 100.0f;  /* perfect retention when just learned */

    strncpy(e->title,    title,    MAX_EXP_TITLE    - 1);
    strncpy(e->lesson,   lesson,   MAX_EXP_LESSON   - 1);
    strncpy(e->category, category, MAX_SKILL_CATEGORY - 1);

    /* Schedule first review in 1 day */
    ls_schedule_review(ls, e->id);
    ls->total_experiences++;

    printf("[Experience] Recorded: '%s' (impact=%.1f, outcome=%s)\n",
           title, impact, outcome_name(outcome));
    return e;
}

/* ─────────────────────────────────────────────────────────────
 *  ls_review_experience()
 *  SM-2 Spaced Repetition Algorithm.
 *
 *  quality: 0–5 (how well you remembered)
 *    0–2: failed — reset interval to 1 day
 *    3:   OK with effort — keep current interval
 *    4–5: easy — increase interval (ease_factor adjustment)
 *
 *  New interval = old_interval × ease_factor
 *  ease_factor adjusts based on performance:
 *    EF = EF + (0.1 - (5 - q) × (0.08 + (5 - q) × 0.02))
 *
 *  OS: cache line replacement policy — recently accessed AND
 *  frequently accessed items stay longer in cache.
 * ──────────────────────────────────────────────────────────── */
void ls_review_experience(LearningSystem *ls, uint32_t exp_id, int quality) {
    if (!ls || quality < 0 || quality > 5) return;

    /* Find experience */
    Experience *e = NULL;
    for (int i = 0; i < ls->exp_count; i++) {
        if (ls->experiences[i].id == exp_id) {
            e = &ls->experiences[i];
            break;
        }
    }
    if (!e) return;

    e->reviewed_count++;

    /* Find and update review item */
    for (int i = 0; i < ls->review_count; i++) {
        if (ls->review_queue[i].experience_id == exp_id) {
            ReviewItem *r = &ls->review_queue[i];

            /* Adjust ease factor (SM-2 formula) */
            r->ease_factor += 0.1f - (5 - quality) * (0.08f + (5 - quality) * 0.02f);
            if (r->ease_factor < 1.3f) r->ease_factor = 1.3f;

            if (quality < 3) {
                /* Failed — reset to short interval */
                r->interval_days = 1;
            } else if (e->reviewed_count == 1) {
                r->interval_days = 1;
            } else if (e->reviewed_count == 2) {
                r->interval_days = 6;
            } else {
                r->interval_days = (int)(r->interval_days * r->ease_factor);
            }

            /* Schedule next review */
            r->due_date = time(NULL) + r->interval_days * 86400;

            /* Update retention score */
            e->retention_score = 50.0f + (quality / 5.0f) * 50.0f;

            printf("[Review] '%s': quality=%d, next in %d days\n",
                   e->title, quality, r->interval_days);
            return;
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 *  ls_schedule_review() — add to spaced repetition queue
 * ──────────────────────────────────────────────────────────── */
void ls_schedule_review(LearningSystem *ls, uint32_t exp_id) {
    if (!ls || ls->review_count >= MAX_REVIEW_ITEMS) return;

    ReviewItem *r = &ls->review_queue[ls->review_count++];
    r->experience_id  = exp_id;
    r->interval_days  = 1;           /* first review: tomorrow */
    r->ease_factor    = 2.5f;        /* SM-2 default ease factor */
    r->due_date       = time(NULL) + 86400;  /* 1 day */
}

/* ─────────────────────────────────────────────────────────────
 *  ls_get_due_reviews() — count reviews due today
 * ──────────────────────────────────────────────────────────── */
int ls_get_due_reviews(const LearningSystem *ls) {
    if (!ls) return 0;
    time_t now = time(NULL);
    int due = 0;
    for (int i = 0; i < ls->review_count; i++) {
        if (ls->review_queue[i].due_date <= now) due++;
    }
    return due;
}

/* ─────────────────────────────────────────────────────────────
 *  ls_add_skill()
 *  Register a new skill to track.
 *  OS: create a new performance counter.
 * ──────────────────────────────────────────────────────────── */
Skill* ls_add_skill(LearningSystem *ls, const char *name, const char *category) {
    if (!ls || ls->skill_count >= MAX_SKILLS) return NULL;

    Skill *s = &ls->skills[ls->skill_count++];
    memset(s, 0, sizeof(Skill));

    s->id             = ls->next_id++;
    s->level          = SKILL_NOVICE;
    s->xp_points      = 0.0f;
    s->xp_to_next_level = 100.0f;   /* first level requires 100 XP */
    s->practice_hours = 0.0f;
    s->daily_streak   = 0;
    s->growth_rate    = 0.0f;

    strncpy(s->name,     name,     MAX_SKILL_NAME     - 1);
    strncpy(s->category, category, MAX_SKILL_CATEGORY - 1);

    printf("[Skill] Registered: '%s' (%s)\n", name, category);
    return s;
}

/* ─────────────────────────────────────────────────────────────
 *  ls_practice_skill()
 *  Record practice time and award XP.
 *  XP formula: base_xp = hours × 30 (diminishing returns near level cap)
 *  Level up when xp_points >= xp_to_next_level.
 *  Each level requires 1.5× more XP than the previous.
 *
 *  OS: performance counter increment, triggering level-up events.
 * ──────────────────────────────────────────────────────────── */
void ls_practice_skill(LearningSystem *ls, uint32_t skill_id, float hours) {
    if (!ls) return;

    Skill *s = NULL;
    for (int i = 0; i < ls->skill_count; i++) {
        if (ls->skills[i].id == skill_id) { s = &ls->skills[i]; break; }
    }
    if (!s) return;

    float old_hours = s->practice_hours;
    s->practice_hours += hours;

    /* Award XP — diminishing returns as level increases */
    float level_factor = 1.0f / (1.0f + (float)s->level * 0.2f);
    float xp_gained = hours * 30.0f * level_factor;
    s->xp_points += xp_gained;

    /* Track growth rate (XP per hour, rolling average) */
    float total_hours = s->practice_hours;
    if (total_hours > 0) {
        s->growth_rate = (s->growth_rate * old_hours + xp_gained) / total_hours;
    }

    /* Check for level up */
    while (s->xp_points >= s->xp_to_next_level &&
           s->level < SKILL_EXPERT) {
        s->xp_points      -= s->xp_to_next_level;
        s->xp_to_next_level *= 1.5f;   /* each level costs 1.5× more */
        s->level = (SkillLevel)((int)s->level + 1);
        printf("[Skill] LEVEL UP! '%s' → %s 🎉\n",
               s->name, skill_level_name(s->level));
    }

    /* Update streak */
    time_t now = time(NULL);
    if (s->last_practiced > 0) {
        double days_since = difftime(now, s->last_practiced) / 86400.0;
        if (days_since <= 1.5) s->daily_streak++;
        else                   s->daily_streak = 1;  /* streak broken */
    } else {
        s->daily_streak = 1;
    }
    s->last_practiced = now;
    ls->total_learning_hours += hours;

    printf("[Skill] Practiced '%s' for %.1fh (+%.1f XP, streak=%d days)\n",
           s->name, hours, xp_gained, s->daily_streak);
}

/* ─────────────────────────────────────────────────────────────
 *  ls_get_skill_progress_pct()
 *  Returns 0–100% progress toward next level.
 * ──────────────────────────────────────────────────────────── */
float ls_get_skill_progress_pct(const Skill *s) {
    if (!s || s->xp_to_next_level == 0) return 100.0f;
    return (s->xp_points / s->xp_to_next_level) * 100.0f;
}

/* ─────────────────────────────────────────────────────────────
 *  ls_record_pattern()
 *  Record a (trigger → action → result) pattern.
 *  If pattern already exists, update its statistics.
 *  OS: hardware branch predictor updating its prediction table.
 * ──────────────────────────────────────────────────────────── */
Pattern* ls_record_pattern(LearningSystem *ls, const char *trigger,
                             const char *action, const char *result,
                             int success) {
    if (!ls) return NULL;

    /* Check if pattern already exists (same trigger + action) */
    for (int i = 0; i < ls->pattern_count; i++) {
        Pattern *p = &ls->patterns[i];
        if (strncmp(p->trigger, trigger, MAX_SKILL_CATEGORY - 1) == 0 &&
            strncmp(p->action,  action,  MAX_PATTERN_DESC  - 1) == 0) {
            /* Update existing pattern statistics */
            p->occurrence_count++;
            float old_rate = p->success_rate;
            p->success_rate = (old_rate * (p->occurrence_count - 1) +
                               (success ? 100.0f : 0.0f)) / p->occurrence_count;
            /* Confidence increases with more observations */
            p->confidence = fminf(99.0f,
                50.0f + (float)p->occurrence_count * 5.0f);
            return p;
        }
    }

    /* New pattern */
    if (ls->pattern_count >= MAX_PATTERNS) return NULL;
    Pattern *p = &ls->patterns[ls->pattern_count++];
    memset(p, 0, sizeof(Pattern));

    p->id               = ls->next_id++;
    p->occurrence_count = 1;
    p->success_rate     = success ? 100.0f : 0.0f;
    p->confidence       = 50.0f;   /* low confidence with 1 observation */

    strncpy(p->trigger, trigger, MAX_SKILL_CATEGORY - 1);
    strncpy(p->action,  action,  MAX_PATTERN_DESC   - 1);
    strncpy(p->result,  result,  MAX_PATTERN_DESC   - 1);

    return p;
}

/* ─────────────────────────────────────────────────────────────
 *  Ebbinghaus Forgetting Curve
 *  R(t) = e^(-t / S)
 *  where S = 1.0 (decays to ~37% in 1 day without review)
 *
 *  OS: cache line freshness decay function.
 * ──────────────────────────────────────────────────────────── */
float ls_forgetting_retention(int days_since_last_review) {
    float S = 2.0f;   /* memory strength factor */
    return expf(-(float)days_since_last_review / S) * 100.0f;
}

/* ─────────────────────────────────────────────────────────────
 *  Print functions
 * ──────────────────────────────────────────────────────────── */
void ls_print_experiences(const LearningSystem *ls, int top_n) {
    if (!ls || ls->exp_count == 0) { printf("  (no experiences)\n"); return; }

    int limit = top_n > 0 && top_n < ls->exp_count ? top_n : ls->exp_count;
    printf("\n  Experience Database (showing %d):\n", limit);
    printf("  %-4s  %-40s  %-10s  %6s  %6s\n",
           "ID", "Title", "Outcome", "Impact", "Reten%");

    for (int i = ls->exp_count - 1; i >= ls->exp_count - limit; i--) {
        const Experience *e = &ls->experiences[i];
        printf("  %-4u  %-40s  %-10s  %6.1f  %5.0f%%\n",
               e->id, e->title,
               outcome_name(e->outcome),
               e->impact_score,
               e->retention_score);
    }
    printf("\n");
}

void ls_print_skills(const LearningSystem *ls) {
    if (!ls || ls->skill_count == 0) { printf("  (no skills tracked)\n"); return; }

    printf("\n  Skills Dashboard:\n");
    printf("  %-28s  %-12s  %-12s  %7s  %6s  %5s\n",
           "Skill", "Level", "Category", "Hours", "XP%", "Stk");
    printf("  %-28s  %-12s  %-12s  %7s  %6s  %5s\n",
           "────────────────────────────",
           "────────────", "────────────",
           "───────", "──────", "─────");

    for (int i = 0; i < ls->skill_count; i++) {
        const Skill *s = &ls->skills[i];
        printf("  %-28s  %-12s  %-12s  %7.1f  %5.0f%%  %4d\n",
               s->name,
               skill_level_name(s->level),
               s->category,
               s->practice_hours,
               ls_get_skill_progress_pct(s),
               s->daily_streak);
    }
    printf("\n");
}

void ls_print_patterns(const LearningSystem *ls) {
    if (!ls || ls->pattern_count == 0) { printf("  (no patterns yet)\n"); return; }

    printf("\n  Recognized Patterns:\n");
    printf("  %-20s  %-30s  %6s  %8s\n",
           "Trigger", "Action", "Times", "Success%");
    for (int i = 0; i < ls->pattern_count; i++) {
        const Pattern *p = &ls->patterns[i];
        printf("  %-20s  %-30s  %6d  %7.0f%%\n",
               p->trigger, p->action,
               p->occurrence_count, p->success_rate);
    }
    printf("\n");
}

void ls_print_due_reviews(const LearningSystem *ls) {
    if (!ls) return;
    time_t now = time(NULL);
    int count  = 0;

    printf("\n  Due Reviews:\n");
    for (int i = 0; i < ls->review_count; i++) {
        const ReviewItem *r = &ls->review_queue[i];
        if (r->due_date <= now) {
            /* Find the experience */
            for (int j = 0; j < ls->exp_count; j++) {
                if (ls->experiences[j].id == r->experience_id) {
                    printf("    [DUE] '%s'\n", ls->experiences[j].title);
                    count++;
                    break;
                }
            }
        }
    }
    if (count == 0) printf("    No reviews due today.\n");
    printf("\n");
}

void ls_print_learning_curve(const Skill *s) {
    if (!s) return;

    printf("\n  Learning Curve: '%s'\n", s->name);
    printf("  Hours → Level (Dreyfus model)\n");

    float xp_req[] = {100.0f, 150.0f, 225.0f, 337.5f, 506.25f};
    float hours_per_xp = 1.0f / 30.0f;
    printf("  Novice     : 0    – %.0fh\n", xp_req[0] * hours_per_xp);
    printf("  Beginner   : %.0f – %.0fh\n",
           xp_req[0] * hours_per_xp,
           (xp_req[0] + xp_req[1]) * hours_per_xp);
    printf("  Competent  : %.0f – %.0fh\n",
           (xp_req[0] + xp_req[1]) * hours_per_xp,
           (xp_req[0]+xp_req[1]+xp_req[2]) * hours_per_xp);
    printf("  Current: %.1fh / level=%s\n",
           s->practice_hours, skill_level_name(s->level));
    printf("\n");
}

void ls_print_analytics(const LearningSystem *ls) {
    if (!ls) return;

    printf("\n  ┌─ Learning Analytics ─────────────────────────────\n");
    printf("  │  Total experiences: %d\n",   ls->total_experiences);
    printf("  │  Total skills:      %d\n",   ls->skill_count);
    printf("  │  Learning hours:    %.1f\n", ls->total_learning_hours);
    printf("  │  Due reviews:       %d\n",   ls_get_due_reviews(ls));
    printf("  │  Patterns found:    %d\n",   ls->pattern_count);

    /* Most-practiced skill */
    if (ls->skill_count > 0) {
        int best = 0;
        for (int i = 1; i < ls->skill_count; i++) {
            if (ls->skills[i].practice_hours > ls->skills[best].practice_hours)
                best = i;
        }
        printf("  │  Top skill:         %s (%.1fh, %s)\n",
               ls->skills[best].name,
               ls->skills[best].practice_hours,
               skill_level_name(ls->skills[best].level));
    }
    printf("  └──────────────────────────────────────────────────\n\n");
}
