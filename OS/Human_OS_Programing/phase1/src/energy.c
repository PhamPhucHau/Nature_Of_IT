/*
 * energy.c  —  Phase 1: Energy Management Implementation
 *
 * OS Concept: This models CPU resource management.
 *
 * Key mappings:
 *   energy_consume()      → process uses CPU cycles (utilization)
 *   energy_recover()      → CPU idle time / power-save mode
 *   energy_is_critical()  → CPU at 100%, throttling needed
 *   energy_efficiency()   → CPU performance per watt (IPC)
 *
 * The energy model uses three independent "processors":
 *   Mental    → like a GPU (complex parallel computation)
 *   Physical  → like a CPU (general-purpose computation)
 *   Emotional → like network bandwidth (connection capacity)
 */

#include "../include/energy.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ─────────────────────────────────────────────────────────────
 *  energy_init()
 *  OS: Initialize CPU scheduler with max available cycles.
 *  Human OS: Set personal energy baselines at start of day.
 * ──────────────────────────────────────────────────────────── */
void energy_init(EnergyState *es, float mental_cap, float physical_cap,
                 float emotional_cap) {
    if (!es) return;

    memset(es, 0, sizeof(EnergyState));

    /* Set personal capacity (everyone has different energy limits) */
    es->mental_capacity    = mental_cap;
    es->physical_capacity  = physical_cap;
    es->emotional_capacity = emotional_cap;

    /* Start the day fully charged (OS: fresh boot = full resources) */
    es->mental    = mental_cap;
    es->physical  = physical_cap;
    es->emotional = emotional_cap;

    /* Recovery rates (units per hour) — individual settings */
    /* Mental recovers slowest (sleep is essential for cognitive recovery) */
    es->mental_recovery_rate    = 8.0f;   /* ~8 hours to fully recover */
    es->physical_recovery_rate  = 12.0f;  /* ~6 hours for physical */
    es->emotional_recovery_rate = 10.0f;  /* ~7 hours for emotional */

    es->last_updated = time(NULL);
    es->day_start    = time(NULL);

    printf("[Energy] Initialized: mental=%.0f, physical=%.0f, emotional=%.0f\n",
           mental_cap, physical_cap, emotional_cap);
}

/* ─────────────────────────────────────────────────────────────
 *  energy_consume()
 *  OS: Process uses CPU/memory/I-O resources.
 *  Human OS: Task drains energy from one or more types.
 *
 *  Returns 0 on success, -1 if energy depleted (OS: EAGAIN)
 * ──────────────────────────────────────────────────────────── */
int energy_consume(EnergyState *es, float mental, float physical, float emotional) {
    if (!es) return -1;

    /* Clamp consumption: cannot go below zero (OS: can't use more CPU than exists) */
    float new_mental    = es->mental    - mental;
    float new_physical  = es->physical  - physical;
    float new_emotional = es->emotional - emotional;

    if (new_mental    < ENERGY_MIN) new_mental    = ENERGY_MIN;
    if (new_physical  < ENERGY_MIN) new_physical  = ENERGY_MIN;
    if (new_emotional < ENERGY_MIN) new_emotional = ENERGY_MIN;

    /* Track daily usage totals */
    es->mental_used_today    += (es->mental    - new_mental);
    es->physical_used_today  += (es->physical  - new_physical);
    es->emotional_used_today += (es->emotional - new_emotional);

    /* Apply consumption */
    es->mental    = new_mental;
    es->physical  = new_physical;
    es->emotional = new_emotional;
    es->last_updated = time(NULL);

    /* Warn if entering critical zone (OS: low-memory warning) */
    if (energy_is_critical(es)) {
        printf("[Energy] ⚠ CRITICAL! Overall energy at %.0f%% — rest recommended!\n",
               energy_get_overall(es));
    }

    return 0;
}

/* ─────────────────────────────────────────────────────────────
 *  energy_consume_task()
 *  Consume energy for a specific energy type task.
 * ──────────────────────────────────────────────────────────── */
int energy_consume_task(EnergyState *es, float amount, int energy_type) {
    if (!es) return -1;

    float m = 0, p = 0, e = 0;
    switch (energy_type) {
        case 0: m = amount; break;               /* ENERGY_MENTAL   */
        case 1: p = amount; break;               /* ENERGY_PHYSICAL */
        case 2: e = amount; break;               /* ENERGY_EMOTIONAL*/
        case 3: m = amount * 0.4f;               /* ENERGY_MIXED    */
                p = amount * 0.3f;
                e = amount * 0.3f;
                break;
        default: m = amount; break;
    }
    return energy_consume(es, m, p, e);
}

/* ─────────────────────────────────────────────────────────────
 *  energy_recover()
 *  OS: CPU in idle/sleep state — power consumption drops, ready
 *  for next burst. Like a CPU's C-states (C0=active, C6=deep sleep).
 *
 *  Recovery amount = rate × hours × (1 - current_level/capacity)
 *  This models diminishing returns: harder to recover when already high.
 * ──────────────────────────────────────────────────────────── */
void energy_recover(EnergyState *es, float hours_rested) {
    if (!es || hours_rested <= 0) return;

    /* Diminishing returns: less recovery benefit when already high */
    float mental_deficit    = es->mental_capacity    - es->mental;
    float physical_deficit  = es->physical_capacity  - es->physical;
    float emotional_deficit = es->emotional_capacity - es->emotional;

    float mental_gain    = fminf(mental_deficit,
                                 es->mental_recovery_rate    * hours_rested);
    float physical_gain  = fminf(physical_deficit,
                                 es->physical_recovery_rate  * hours_rested);
    float emotional_gain = fminf(emotional_deficit,
                                 es->emotional_recovery_rate * hours_rested);

    es->mental    = fminf(es->mental_capacity,    es->mental    + mental_gain);
    es->physical  = fminf(es->physical_capacity,  es->physical  + physical_gain);
    es->emotional = fminf(es->emotional_capacity, es->emotional + emotional_gain);
    es->last_updated = time(NULL);

    printf("[Energy] Recovered %.1fh: mental +%.1f, physical +%.1f, emotional +%.1f\n",
           hours_rested, mental_gain, physical_gain, emotional_gain);
}

/* ─────────────────────────────────────────────────────────────
 *  energy_restore()
 *  Direct restoration (like a power-up item in a game).
 *  Use for: meals, meditation, exercise (paradoxically restores physical).
 * ──────────────────────────────────────────────────────────── */
void energy_restore(EnergyState *es, float mental, float physical, float emotional) {
    if (!es) return;

    es->mental    = fminf(es->mental_capacity,    es->mental    + mental);
    es->physical  = fminf(es->physical_capacity,  es->physical  + physical);
    es->emotional = fminf(es->emotional_capacity, es->emotional + emotional);
    es->last_updated = time(NULL);
}

/* ─────────────────────────────────────────────────────────────
 *  energy_new_day()
 *  OS: New boot cycle — reset daily counters.
 *  Human OS: Start of a new day. Energy recovers overnight.
 * ──────────────────────────────────────────────────────────── */
void energy_new_day(EnergyState *es) {
    if (!es) return;

    /* Log the day's end state before reset */
    energy_log(es, "end of day");

    /* Overnight recovery (~8 hours sleep) */
    energy_recover(es, 8.0f);

    /* Reset daily usage counters */
    es->mental_used_today    = 0;
    es->physical_used_today  = 0;
    es->emotional_used_today = 0;
    es->day_start = time(NULL);

    printf("[Energy] New day started — energy levels after sleep:\n");
    energy_print_status(es);
}

/* ─────────────────────────────────────────────────────────────
 *  energy_get_overall()
 *  Weighted average of all energy types.
 *  Mental is weighted highest (most impactful for productivity).
 * ──────────────────────────────────────────────────────────── */
float energy_get_overall(const EnergyState *es) {
    if (!es) return 0.0f;

    /* Normalize each type to 0–100 relative to its capacity */
    float m_pct = (es->mental_capacity    > 0) ?
                  (es->mental    / es->mental_capacity)    * 100.0f : 0.0f;
    float p_pct = (es->physical_capacity  > 0) ?
                  (es->physical  / es->physical_capacity)  * 100.0f : 0.0f;
    float e_pct = (es->emotional_capacity > 0) ?
                  (es->emotional / es->emotional_capacity) * 100.0f : 0.0f;

    /* Weighted: mental 40%, physical 35%, emotional 25% */
    return m_pct * 0.40f + p_pct * 0.35f + e_pct * 0.25f;
}

/* ─────────────────────────────────────────────────────────────
 *  energy_is_critical()  /  energy_is_low()
 *  Like CPU thermal throttle thresholds.
 * ──────────────────────────────────────────────────────────── */
int energy_is_critical(const EnergyState *es) {
    if (!es) return 1;
    return energy_get_overall(es) < ENERGY_CRITICAL;
}

int energy_is_low(const EnergyState *es) {
    if (!es) return 1;
    return energy_get_overall(es) < ENERGY_LOW;
}

/* ─────────────────────────────────────────────────────────────
 *  energy_efficiency_factor()
 *  Returns a multiplier 0.0–1.0 for how effectively work gets done.
 *  OS: CPU IPC (instructions per cycle) drops under thermal throttle.
 * ──────────────────────────────────────────────────────────── */
float energy_efficiency_factor(const EnergyState *es) {
    if (!es) return 0.0f;

    float overall = energy_get_overall(es);

    if (overall >= ENERGY_OPTIMAL_MIN) return 1.0f;       /* peak performance   */
    if (overall >= ENERGY_LOW)         return 0.75f;       /* slightly degraded  */
    if (overall >= ENERGY_CRITICAL)    return 0.50f;       /* significantly slow */
    return 0.25f;                                          /* barely functional  */
}

/* ─────────────────────────────────────────────────────────────
 *  energy_log()
 *  Record the current energy state for trend analysis.
 *  OS: CPU performance counter snapshot.
 * ──────────────────────────────────────────────────────────── */
void energy_log(EnergyState *es, const char *note) {
    if (!es || es->history_count >= MAX_ENERGY_LOGS) return;

    EnergyLog *log = &es->history[es->history_count++];
    log->timestamp      = time(NULL);
    log->mental_level   = es->mental;
    log->physical_level = es->physical;
    log->emotional_level= es->emotional;
    log->overall_level  = energy_get_overall(es);
    strncpy(log->note, note ? note : "", sizeof(log->note) - 1);
}

/* ─────────────────────────────────────────────────────────────
 *  energy_level_label()
 *  Human-readable label for an energy percentage.
 * ──────────────────────────────────────────────────────────── */
const char* energy_level_label(float level) {
    if (level >= ENERGY_OPTIMAL_MIN) return "OPTIMAL";
    if (level >= ENERGY_LOW)         return "LOW";
    if (level >= ENERGY_CRITICAL)    return "CRITICAL";
    return "DEPLETED";
}

/* ─────────────────────────────────────────────────────────────
 *  energy_print_status()
 *  Display current energy state — like `top` for CPU utilization.
 * ──────────────────────────────────────────────────────────── */
void energy_print_status(const EnergyState *es) {
    if (!es) return;

    float m_pct = (es->mental_capacity   > 0) ?
                  (es->mental   / es->mental_capacity)   * 100.0f : 0.0f;
    float p_pct = (es->physical_capacity > 0) ?
                  (es->physical / es->physical_capacity) * 100.0f : 0.0f;
    float e_pct = (es->emotional_capacity> 0) ?
                  (es->emotional/ es->emotional_capacity)* 100.0f : 0.0f;

    printf("\n  ┌─ Energy Status ───────────────────────────────────\n");
    printf("  │  Mental:    %5.1f / %5.1f  (%5.1f%%)  [%s]\n",
           es->mental,    es->mental_capacity,    m_pct,
           energy_level_label(m_pct));
    printf("  │  Physical:  %5.1f / %5.1f  (%5.1f%%)  [%s]\n",
           es->physical,  es->physical_capacity,  p_pct,
           energy_level_label(p_pct));
    printf("  │  Emotional: %5.1f / %5.1f  (%5.1f%%)  [%s]\n",
           es->emotional, es->emotional_capacity, e_pct,
           energy_level_label(e_pct));
    printf("  │  Overall:   %5.1f%%  [%s]  Efficiency: %.0f%%\n",
           energy_get_overall(es),
           energy_level_label(energy_get_overall(es)),
           energy_efficiency_factor(es) * 100.0f);
    printf("  │  Used today: mental=%.1f  physical=%.1f  emotional=%.1f\n",
           es->mental_used_today,
           es->physical_used_today,
           es->emotional_used_today);
    printf("  └───────────────────────────────────────────────────\n\n");
}

/* ─────────────────────────────────────────────────────────────
 *  energy_print_history()
 *  Print the last N energy log entries — like CPU utilization graph.
 * ──────────────────────────────────────────────────────────── */
void energy_print_history(const EnergyState *es, int last_n) {
    if (!es || es->history_count == 0) {
        printf("  (no energy history)\n");
        return;
    }

    int start = es->history_count - last_n;
    if (start < 0) start = 0;

    printf("\n  Energy History (last %d entries):\n", es->history_count - start);
    printf("  %-20s  %6s  %6s  %6s  %7s  %s\n",
           "TIME", "MENTAL", "PHYS", "EMOT", "OVERALL", "NOTE");

    for (int i = start; i < es->history_count; i++) {
        const EnergyLog *log = &es->history[i];
        char time_str[20];
        struct tm *dt = localtime(&log->timestamp);
        strftime(time_str, sizeof(time_str), "%m-%d %H:%M:%S", dt);
        printf("  %-20s  %6.1f  %6.1f  %6.1f  %7.1f  %s\n",
               time_str,
               log->mental_level,
               log->physical_level,
               log->emotional_level,
               log->overall_level,
               log->note);
    }
    printf("\n");
}
