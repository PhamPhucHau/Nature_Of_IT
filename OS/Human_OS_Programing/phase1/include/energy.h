/*
 * energy.h  —  Phase 1: Energy (CPU Resource) Tracking
 *
 * OS Analogy:
 *   CPU time       → Energy units
 *   CPU utilization → Energy consumption rate
 *   Idle CPU        → Rest / recovery time
 *   CPU throttle    → Low energy → slower, less complex tasks
 *   CPU overload    → Burnout
 *
 * The human body has three energy "processors":
 *   Mental   → prefrontal cortex (cognition, decisions, creativity)
 *   Physical → musculoskeletal system (movement, labor)
 *   Emotional → limbic system (relationships, motivation, resilience)
 */

#ifndef ENERGY_H
#define ENERGY_H

#include <time.h>

/* ─────────────────────────────────────────────────────────────
 *  ENERGY LIMITS
 * ──────────────────────────────────────────────────────────── */
#define ENERGY_MAX          100.0f   /* full capacity (like 100% CPU available) */
#define ENERGY_MIN            0.0f   /* depleted                                 */
#define ENERGY_CRITICAL      20.0f   /* danger zone — stop complex work          */
#define ENERGY_LOW           40.0f   /* reduced performance                      */
#define ENERGY_OPTIMAL_MIN   60.0f   /* sweet spot for best work                 */
#define ENERGY_OPTIMAL_MAX   90.0f   /* peak performance zone                    */
#define MAX_ENERGY_LOGS     200      /* max history entries                      */

/* ─────────────────────────────────────────────────────────────
 *  ENERGY LOG ENTRY — one data point in energy history
 *
 *  OS: CPU utilization time-series log
 *  Human OS: energy level over time
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    time_t    timestamp;
    float     mental_level;    /* 0–100 */
    float     physical_level;  /* 0–100 */
    float     emotional_level; /* 0–100 */
    float     overall_level;   /* weighted average */
    char      note[64];        /* optional context */
} EnergyLog;

/* ─────────────────────────────────────────────────────────────
 *  ENERGY STATE — current resource levels
 *
 *  OS: CPU available cycles per second
 *  Human OS: available energy per hour
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    /* Current levels (0.0 – 100.0) */
    float mental;       /* brain power — for focus, decisions, learning */
    float physical;     /* body power — for movement, exercise, manual  */
    float emotional;    /* heart power — for relationships, motivation  */

    /* Daily totals consumed */
    float mental_used_today;
    float physical_used_today;
    float emotional_used_today;

    /* Recovery rates per hour (how fast each type recharges) */
    float mental_recovery_rate;    /* units/hour when resting */
    float physical_recovery_rate;
    float emotional_recovery_rate;

    /* Baseline capacity (individual differences — some people have more) */
    float mental_capacity;         /* personal max mental energy   */
    float physical_capacity;       /* personal max physical energy */
    float emotional_capacity;      /* personal max emotional energy*/

    /* History for trend analysis */
    EnergyLog  history[MAX_ENERGY_LOGS];
    int        history_count;

    /* Time tracking */
    time_t     last_updated;
    time_t     day_start;
} EnergyState;

/* ─────────────────────────────────────────────────────────────
 *  FUNCTION DECLARATIONS
 * ──────────────────────────────────────────────────────────── */

/* Initialization */
void  energy_init(EnergyState *es, float mental_cap, float physical_cap, float emotional_cap);

/* Consumption — OS: CPU burst uses cycles */
int   energy_consume(EnergyState *es, float mental, float physical, float emotional);
int   energy_consume_task(EnergyState *es, float amount, int energy_type);

/* Recovery — OS: CPU idle, waiting for next process */
void  energy_recover(EnergyState *es, float hours_rested);
void  energy_restore(EnergyState *es, float mental, float physical, float emotional);
void  energy_new_day(EnergyState *es);   /* reset daily counters at midnight */

/* Query */
float energy_get_overall(const EnergyState *es);
int   energy_is_critical(const EnergyState *es);
int   energy_is_low(const EnergyState *es);
float energy_efficiency_factor(const EnergyState *es);  /* 0.0–1.0 multiplier */

/* Logging */
void  energy_log(EnergyState *es, const char *note);
void  energy_print_status(const EnergyState *es);
void  energy_print_history(const EnergyState *es, int last_n);
const char* energy_level_label(float level);

#endif /* ENERGY_H */
