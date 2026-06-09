/*
 * energy_model.c  —  Phase 3: Advanced Energy Model Implementation
 *
 * Build:  gcc -Wall -Wextra -o phase3 main.c energy_model.c -lm && ./phase3
 *
 * Key concepts:
 *  1. Circadian multiplier: energy effectiveness varies by time of day
 *  2. Ultradian cycles: 90-min focus waves — rest after each cycle
 *  3. Burnout: accumulates from chronic under-recovery
 *  4. Recovery activities: different activities restore different energies
 */

#include "../include/energy_model.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ─────────────────────────────────────────────────────────────
 *  Recovery activity database
 *  OS: CPU C-state definitions (power state → recovery rate)
 * ──────────────────────────────────────────────────────────── */
static const RecoveryActivity RECOVERY_DB[NUM_RECOVERY_ACTIVITIES] = {
    /* name,              mental, physical, emotional, min_duration */
    { "Deep Sleep",        15.0f,   12.0f,    14.0f,    360  }, /* 6+ hours */
    { "Power Nap (20min)",  8.0f,    5.0f,     6.0f,     15  }, /* 15-25 min */
    { "Meditation",        12.0f,    4.0f,    10.0f,     10  }, /* 10+ min */
    { "Physical Exercise",  4.0f,   18.0f,     8.0f,     20  }, /* 20+ min */
    { "Social Time",        2.0f,    1.0f,    15.0f,     30  }, /* 30+ min */
    { "Light Reading",      5.0f,    2.0f,     4.0f,     15  }, /* any */
    { "Walk Outdoors",      6.0f,   10.0f,     8.0f,     15  }, /* 15+ min */
    { "Nutritious Meal",    5.0f,    8.0f,     4.0f,     20  }, /* 20 min */
};

/* ─────────────────────────────────────────────────────────────
 *  Circadian energy profile (energy % by hour of day)
 *  Based on chronobiology research.
 *  OS analogy: CPU base clock vs turbo frequencies across time.
 * ──────────────────────────────────────────────────────────── */
static const float CIRCADIAN_PROFILE[24] = {
    /* 00 01  02  03  04  05  06  07  08  09  10  11 */
      0.3f,0.2f,0.2f,0.2f,0.3f,0.4f,0.6f,0.8f,0.9f,1.0f,1.0f,0.95f,
    /* 12  13  14  15  16  17  18  19  20  21  22  23 */
      0.8f,0.6f,0.55f,0.65f,0.75f,0.8f,0.75f,0.7f,0.6f,0.5f,0.4f,0.35f,
};

/* ─────────────────────────────────────────────────────────────
 *  em_init()
 * ──────────────────────────────────────────────────────────── */
void em_init(EnergyManager *em, float mental, float physical, float emotional) {
    memset(em, 0, sizeof(EnergyManager));

    em->mental_cap    = mental;
    em->physical_cap  = physical;
    em->emotional_cap = emotional;
    em->mental        = mental;
    em->physical      = physical;
    em->emotional     = emotional;

    em->burnout_threshold = 70.0f;
    em->peak_mental_today = mental;
    em->min_mental_today  = mental;
    em->last_updated = time(NULL);

    /* Get hour from system time */
    struct tm *lt = localtime(&em->last_updated);
    em->hour_of_day = lt ? lt->tm_hour : 9;

    printf("[EnergyManager] Initialized — circadian phase: %s\n",
           em_circadian_phase_name(em_get_circadian_phase(em->hour_of_day)));
}

/* ─────────────────────────────────────────────────────────────
 *  em_work()
 *  Consume energy while working on a task.
 *  OS: CPU executing instructions (P-state active).
 *
 *  Energy consumption is modulated by:
 *    1. Circadian phase: less effective at night
 *    2. Current energy level: work costs more when tired
 *    3. Ultradian cycle: gradually reduces effectiveness
 * ──────────────────────────────────────────────────────────── */
void em_work(EnergyManager *em, float mental, float physical, float emotional,
             float duration_minutes) {
    if (!em) return;

    /* Circadian modulation: same task costs more energy at off-peak times */
    float circ   = em_get_circadian_multiplier(em->hour_of_day);
    float fatigue = 1.0f + (1.0f - em_get_performance_factor(em)) * 0.3f;
    float cost    = fatigue / fmaxf(circ, 0.1f);

    em->mental    -= mental    * cost;
    em->physical  -= physical  * cost;
    em->emotional -= emotional * cost;

    /* Clamp to zero */
    if (em->mental    < 0) em->mental    = 0;
    if (em->physical  < 0) em->physical  = 0;
    if (em->emotional < 0) em->emotional = 0;

    /* Update ultradian cycle */
    em->ultradian_phase += duration_minutes;
    em->total_work_minutes += duration_minutes;

    /* Burnout accumulates from working without recovery */
    float burnout_gain = (duration_minutes / 60.0f) * (1.0f - em_get_performance_factor(em));
    em->burnout_score = fminf(100.0f, em->burnout_score + burnout_gain);

    /* Track daily stats */
    if (em->mental < em->min_mental_today) em->min_mental_today = em->mental;

    /* History snapshot */
    if (em->history_idx < MAX_HISTORY) {
        em->history_mental[em->history_idx]   = em->mental;
        em->history_physical[em->history_idx] = em->physical;
        em->history_idx++;
    }
}

/* ─────────────────────────────────────────────────────────────
 *  em_rest()
 *  Perform a recovery activity.
 *  OS: Enter a CPU C-state to recover power/temperature.
 * ──────────────────────────────────────────────────────────── */
void em_rest(EnergyManager *em, const char *activity, float duration_minutes) {
    if (!em || !activity) return;

    /* Find matching activity */
    const RecoveryActivity *act = NULL;
    for (int i = 0; i < NUM_RECOVERY_ACTIVITIES; i++) {
        if (strncmp(RECOVERY_DB[i].name, activity, strlen(activity)) == 0) {
            act = &RECOVERY_DB[i];
            break;
        }
    }
    if (!act) act = &RECOVERY_DB[5]; /* default: light reading */

    float hours = duration_minutes / 60.0f;
    float eff   = fminf(1.0f, duration_minutes / fmaxf((float)act->min_duration, 1.0f));

    /* Restore energy, capped at capacity */
    em->mental    = fminf(em->mental_cap,    em->mental    + act->mental_rate    * hours * eff);
    em->physical  = fminf(em->physical_cap,  em->physical  + act->physical_rate  * hours * eff);
    em->emotional = fminf(em->emotional_cap, em->emotional + act->emotional_rate * hours * eff);

    /* Recovery reduces burnout */
    em->burnout_score = fmaxf(0.0f, em->burnout_score - (hours * 10.0f));

    /* Reset ultradian cycle */
    if (duration_minutes >= 15.0f) em->ultradian_phase = 0.0f;

    em->total_rest_minutes += duration_minutes;

    printf("[EnergyManager] Rested: '%s' for %.0f min → burnout=%.1f\n",
           act->name, duration_minutes, em->burnout_score);
}

/* ─────────────────────────────────────────────────────────────
 *  em_sleep()
 *  Full overnight sleep — maximum recovery.
 *  OS: Deep CPU power state (C8/C9) — full cache flush, etc.
 * ──────────────────────────────────────────────────────────── */
void em_sleep(EnergyManager *em, float hours) {
    if (!em) return;

    float quality = fminf(1.0f, hours / 8.0f);   /* 8h = 100% quality */

    em->mental    = fminf(em->mental_cap,    em->mental    + em->mental_cap    * 0.9f * quality);
    em->physical  = fminf(em->physical_cap,  em->physical  + em->physical_cap  * 0.85f* quality);
    em->emotional = fminf(em->emotional_cap, em->emotional + em->emotional_cap * 0.8f * quality);

    /* Major burnout reduction from sleep */
    em->burnout_score = fmaxf(0.0f, em->burnout_score - (hours * 8.0f));

    /* Reset daily counters */
    em->ultradian_phase   = 0.0f;
    em->focus_cycles_today = 0;
    em->total_work_minutes = 0;
    em->total_rest_minutes = 0;
    em->peak_mental_today  = em->mental;
    em->min_mental_today   = em->mental;
    em->history_idx        = 0;

    printf("[EnergyManager] Slept %.1fh (quality=%.0f%%) → burnout=%.1f\n",
           hours, quality * 100.0f, em->burnout_score);
}

/* ─────────────────────────────────────────────────────────────
 *  Query functions
 * ──────────────────────────────────────────────────────────── */
float em_get_overall(const EnergyManager *em) {
    if (!em) return 0.0f;
    float m = em->mental_cap    > 0 ? (em->mental    / em->mental_cap)    * 100.0f : 0;
    float p = em->physical_cap  > 0 ? (em->physical  / em->physical_cap)  * 100.0f : 0;
    float e = em->emotional_cap > 0 ? (em->emotional / em->emotional_cap) * 100.0f : 0;
    return m * 0.4f + p * 0.35f + e * 0.25f;
}

float em_get_performance_factor(const EnergyManager *em) {
    if (!em) return 0.0f;
    float overall = em_get_overall(em);
    float burnout_penalty = em->burnout_score / 100.0f * 0.3f;
    float base = (overall >= 70) ? 1.0f : (overall >= 40) ? 0.75f : 0.5f;
    return fmaxf(0.1f, base - burnout_penalty);
}

CircadianPhase em_get_circadian_phase(int hour) {
    if (hour >= 6  && hour < 8)  return CIRCADIAN_WAKE_UP;
    if (hour >= 8  && hour < 12) return CIRCADIAN_MORNING_PEAK;
    if (hour >= 12 && hour < 14) return CIRCADIAN_AFTERNOON_DIP;
    if (hour >= 14 && hour < 17) return CIRCADIAN_AFTERNOON_2;
    if (hour >= 17 && hour < 21) return CIRCADIAN_EVENING;
    return CIRCADIAN_SLEEP;
}

float em_get_circadian_multiplier(int hour) {
    if (hour < 0 || hour > 23) return 0.5f;
    return CIRCADIAN_PROFILE[hour];
}

int em_is_burnout_risk(const EnergyManager *em) {
    return em && em->burnout_score >= em->burnout_threshold;
}

const char* em_circadian_phase_name(CircadianPhase p) {
    switch (p) {
        case CIRCADIAN_WAKE_UP:       return "Wake-up (Rising energy)";
        case CIRCADIAN_MORNING_PEAK:  return "Morning Peak (Best for deep work)";
        case CIRCADIAN_AFTERNOON_DIP: return "Afternoon Dip (Take a break)";
        case CIRCADIAN_AFTERNOON_2:   return "Afternoon Recovery (Meetings OK)";
        case CIRCADIAN_EVENING:       return "Evening (Wind down)";
        case CIRCADIAN_SLEEP:         return "Sleep Phase (Rest!)";
        default:                      return "Unknown";
    }
}

/* ─────────────────────────────────────────────────────────────
 *  Recommendations
 * ──────────────────────────────────────────────────────────── */
void em_recommend_next_action(const EnergyManager *em) {
    if (!em) return;
    float overall = em_get_overall(em);

    printf("\n  💡 Recommendation:\n");

    if (em_is_burnout_risk(em)) {
        printf("    🚨 BURNOUT RISK (score=%.0f)! Take a full day off or long vacation.\n",
               em->burnout_score);
        return;
    }
    if (em->ultradian_phase >= 90.0f) {
        printf("    ⏸  Take a 15-20 min break — ultradian cycle complete (%.0f min)\n",
               em->ultradian_phase);
        return;
    }
    if (overall < 20) {
        printf("    😴 Energy CRITICAL — rest or sleep immediately.\n");
        return;
    }
    if (overall < 40) {
        printf("    🔋 Energy LOW — light tasks only, avoid deep work.\n");
        return;
    }

    CircadianPhase phase = em_get_circadian_phase(em->hour_of_day);
    switch (phase) {
        case CIRCADIAN_MORNING_PEAK:
            printf("    🎯 Peak time! Schedule deep work, complex problems.\n");
            break;
        case CIRCADIAN_AFTERNOON_DIP:
            printf("    ☕ Afternoon dip — take a walk, eat, light meetings.\n");
            break;
        case CIRCADIAN_AFTERNOON_2:
            printf("    📋 Good for reviews, emails, collaborative work.\n");
            break;
        case CIRCADIAN_EVENING:
            printf("    📖 Good for learning, reading, light creative work.\n");
            break;
        default:
            printf("    ✅ Energy OK — proceed with scheduled tasks.\n");
    }
}

void em_suggest_optimal_task_type(const EnergyManager *em) {
    if (!em) return;
    float m_pct = em->mental_cap    > 0 ? em->mental    / em->mental_cap    * 100 : 0;
    float p_pct = em->physical_cap  > 0 ? em->physical  / em->physical_cap  * 100 : 0;
    float e_pct = em->emotional_cap > 0 ? em->emotional / em->emotional_cap * 100 : 0;

    printf("  🎯 Best task type now:\n");
    if (m_pct >= 70)      printf("    Mental work: coding, writing, analysis (%0.f%% mental)\n", m_pct);
    else if (p_pct >= 70) printf("    Physical: exercise, hands-on tasks (%0.f%% physical)\n", p_pct);
    else if (e_pct >= 70) printf("    Social/creative: calls, brainstorm (%0.f%% emotional)\n", e_pct);
    else                  printf("    Low energy — rest, light reading only\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Display
 * ──────────────────────────────────────────────────────────── */
void em_print_status(const EnergyManager *em) {
    if (!em) return;
    CircadianPhase phase = em_get_circadian_phase(em->hour_of_day);
    float circ = em_get_circadian_multiplier(em->hour_of_day);

    printf("\n  ┌─ Energy Manager ─────────────────────────────────\n");
    printf("  │  Overall:    %5.1f%%   Performance: %.0f%%\n",
           em_get_overall(em), em_get_performance_factor(em) * 100.0f);
    printf("  │  Mental:     %5.1f / %.0f\n", em->mental,    em->mental_cap);
    printf("  │  Physical:   %5.1f / %.0f\n", em->physical,  em->physical_cap);
    printf("  │  Emotional:  %5.1f / %.0f\n", em->emotional, em->emotional_cap);
    printf("  │  Circadian:  %s (×%.2f)\n",
           em_circadian_phase_name(phase), circ);
    printf("  │  Ultradian:  %.0f / 90 min  (%.0f min rest cycles)\n",
           em->ultradian_phase, 90.0f - em->ultradian_phase);
    printf("  │  Burnout:    %.1f / 100 %s\n",
           em->burnout_score,
           em_is_burnout_risk(em) ? "⚠ RISK!" : "✓ OK");
    printf("  └──────────────────────────────────────────────────\n");
}

void em_print_circadian_curve(void) {
    printf("\n  Circadian Energy Profile (24-hour energy curve):\n");
    printf("  100%% │");
    for (int h = 0; h < 24; h++) {
        int bars = (int)(CIRCADIAN_PROFILE[h] * 10);
        printf("%c", bars >= 10 ? '█' : bars >= 7 ? '▓' :
                                        bars >= 4 ? '▒' : '░');
    }
    printf("│\n");
    printf("    0%% └");
    for (int h = 0; h < 24; h++) printf("─");
    printf("┘\n");
    printf("        ");
    for (int h = 0; h < 24; h += 4) printf("%02d   ", h);
    printf("\n\n");
}

void em_print_daily_summary(const EnergyManager *em) {
    if (!em) return;
    printf("\n  Daily Summary:\n");
    printf("    Work time:  %.0f min\n", em->total_work_minutes);
    printf("    Rest time:  %.0f min\n", em->total_rest_minutes);
    float ratio = (em->total_work_minutes + em->total_rest_minutes) > 0
                  ? em->total_work_minutes /
                    (em->total_work_minutes + em->total_rest_minutes) * 100.0f
                  : 0;
    printf("    Work/Rest:  %.0f%% / %.0f%%\n", ratio, 100.0f - ratio);
    printf("    Peak mental: %.1f    Min mental: %.1f\n",
           em->peak_mental_today, em->min_mental_today);
}
