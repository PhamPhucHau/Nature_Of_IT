/*
 * energy_model.h  —  Phase 3: Advanced Energy Management System
 *
 * OS Analogy: CPU power management (ACPI, P-states, C-states)
 *
 *   C0 (Active)       → Working at full capacity
 *   C1 (Halt)         → Light break, alert
 *   C3 (Sleep)        → Deep break, nap, meditation
 *   C6 (Deep Sleep)   → Full night sleep
 *   P0 (Turbo boost)  → Flow state / peak performance
 *   P1-P15 (lower)    → Degraded performance under fatigue
 *
 * This phase models:
 *   1. Circadian rhythm (energy across 24 hours)
 *   2. Ultradian rhythm (90-min focus cycles)
 *   3. Energy type interactions (mental drains physical, etc.)
 *   4. Recovery optimization (sleep, nutrition, exercise)
 *   5. Burnout detection and prevention
 */

#ifndef ENERGY_MODEL_H
#define ENERGY_MODEL_H

#include <time.h>

/* ─────────────────────────────────────────────────────────────
 *  CIRCADIAN PHASE — time-of-day energy profile
 *  Like CPU P-states varying by thermal/load conditions
 * ──────────────────────────────────────────────────────────── */
typedef enum {
    CIRCADIAN_WAKE_UP      = 0,   /* 06:00-08:00 rising energy           */
    CIRCADIAN_MORNING_PEAK = 1,   /* 08:00-12:00 peak mental performance  */
    CIRCADIAN_AFTERNOON_DIP= 2,   /* 12:00-14:00 post-lunch slump         */
    CIRCADIAN_AFTERNOON_2  = 3,   /* 14:00-17:00 second peak              */
    CIRCADIAN_EVENING      = 4,   /* 17:00-21:00 winding down             */
    CIRCADIAN_SLEEP        = 5,   /* 21:00-06:00 recovery                 */
} CircadianPhase;

/* ─────────────────────────────────────────────────────────────
 *  RECOVERY ACTIVITY — actions that restore energy
 *  OS: Different sleep states restore CPU at different rates
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    const char *name;
    float mental_rate;     /* energy/hour restored for mental   */
    float physical_rate;   /* energy/hour restored for physical */
    float emotional_rate;  /* energy/hour restored for emotional*/
    int   min_duration;    /* minimum effective duration (min)  */
} RecoveryActivity;

#define NUM_RECOVERY_ACTIVITIES 8

/* ─────────────────────────────────────────────────────────────
 *  ENERGY MANAGER — full energy lifecycle
 * ──────────────────────────────────────────────────────────── */
#define MAX_HISTORY 288   /* 5-min intervals × 24 hours */

typedef struct {
    /* Current levels */
    float mental;
    float physical;
    float emotional;

    /* Capacities */
    float mental_cap;
    float physical_cap;
    float emotional_cap;

    /* Burnout accumulator — builds over days without proper recovery */
    float burnout_score;    /* 0 = none, 100 = full burnout    */
    float burnout_threshold;/* alert at 70, critical at 90     */

    /* Ultradian cycle tracking (90-min focus waves) */
    float ultradian_phase;  /* 0–90 minutes into current cycle */
    int   focus_cycles_today;

    /* Daily metrics */
    float peak_mental_today;
    float min_mental_today;
    float total_work_minutes;
    float total_rest_minutes;

    /* Hourly energy history for circadian analysis */
    float history_mental[MAX_HISTORY];
    float history_physical[MAX_HISTORY];
    int   history_idx;

    time_t  last_updated;
    int     hour_of_day;     /* 0–23 */
} EnergyManager;

/* ─────────────────────────────────────────────────────────────
 *  FUNCTION DECLARATIONS
 * ──────────────────────────────────────────────────────────── */

/* Initialization */
void  em_init(EnergyManager *em, float mental, float physical, float emotional);

/* Work and rest */
void  em_work(EnergyManager *em, float mental, float physical, float emotional,
              float duration_minutes);
void  em_rest(EnergyManager *em, const char *activity, float duration_minutes);
void  em_sleep(EnergyManager *em, float hours);

/* Queries */
float em_get_overall(const EnergyManager *em);
float em_get_performance_factor(const EnergyManager *em);
CircadianPhase em_get_circadian_phase(int hour_of_day);
float em_get_circadian_multiplier(int hour_of_day);
int   em_is_burnout_risk(const EnergyManager *em);

/* Recommendations */
void  em_recommend_next_action(const EnergyManager *em);
void  em_suggest_optimal_task_type(const EnergyManager *em);
void  em_plan_recovery(EnergyManager *em);

/* Display */
void  em_print_status(const EnergyManager *em);
void  em_print_circadian_curve(void);
void  em_print_daily_summary(const EnergyManager *em);
const char* em_circadian_phase_name(CircadianPhase p);

#endif /* ENERGY_MODEL_H */
