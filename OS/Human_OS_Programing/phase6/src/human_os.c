/*
 * human_os.c  —  Phase 6: Integrated Human OS Kernel Implementation
 *
 * Build:  gcc -Wall -Wextra -o phase6 main.c human_os.c -lm && ./phase6
 *
 * This file is the KERNEL of the Human OS.
 * It integrates all subsystems and implements the intelligence layer:
 *
 *  1. Smart scheduling: chooses tasks based on energy + priority + goals
 *  2. Auto-mode selection: changes system mode based on current state
 *  3. Health monitoring: detects burnout, imbalance, goal drift
 *  4. System optimization: adjusts behavior to maximize output
 *  5. Dashboard: real-time system status display
 *
 * OS Analogies in this file:
 *   hos_boot()          → kernel initialization sequence
 *   hos_new_day()       → daily reset (like `shutdown -r now`)
 *   hos_get_next_task() → scheduler dispatch decision
 *   hos_auto_mode()     → kernel power management (governor)
 *   hos_health_check()  → watchdog daemon
 *   hos_dashboard()     → /proc/status + htop combined
 *   hos_log()           → kernel ring buffer (dmesg)
 */

#include "../include/human_os.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ─────────────────────────────────────────────────────────────
 *  Label helpers
 * ──────────────────────────────────────────────────────────── */
const char* hos_mode_name(SystemMode m) {
    switch (m) {
        case MODE_PRODUCTIVE: return "PRODUCTIVE";
        case MODE_FOCUSED:    return "FOCUSED";
        case MODE_RECOVERY:   return "RECOVERY";
        case MODE_EMERGENCY:  return "EMERGENCY";
        case MODE_LEARNING:   return "LEARNING";
        default:              return "UNKNOWN";
    }
}

const char* hos_status_name(SystemStatus s) {
    switch (s) {
        case STATUS_OPTIMAL:  return "OPTIMAL";
        case STATUS_DEGRADED: return "DEGRADED";
        case STATUS_CRITICAL: return "CRITICAL";
        case STATUS_SHUTDOWN: return "SHUTDOWN";
        default:              return "UNKNOWN";
    }
}

const char* hos_skill_level_name(HSkillLevel l) {
    switch (l) {
        case SKILL_NOVICE:     return "Novice";
        case SKILL_BEGINNER:   return "Beginner";
        case SKILL_COMPETENT:  return "Competent";
        case SKILL_PROFICIENT: return "Proficient";
        case SKILL_EXPERT:     return "Expert";
        default:               return "Unknown";
    }
}

/* ─────────────────────────────────────────────────────────────
 *  Internal helpers
 * ──────────────────────────────────────────────────────────── */
static float clamp(float v, float lo, float hi) {
    return v < lo ? lo : v > hi ? hi : v;
}

static float energy_overall(const HEnergyState *e) {
    float m = e->mental_cap    > 0 ? (e->mental    / e->mental_cap)    * 100.0f : 0;
    float p = e->physical_cap  > 0 ? (e->physical  / e->physical_cap)  * 100.0f : 0;
    float em= e->emotional_cap > 0 ? (e->emotional / e->emotional_cap) * 100.0f : 0;
    return m * 0.4f + p * 0.35f + em * 0.25f;
}

static float energy_efficiency(const HEnergyState *e) {
    float ov = energy_overall(e);
    float bp = e->burnout_score / 100.0f * 0.3f;
    float base = (ov >= 70) ? 1.0f : (ov >= 40) ? 0.75f : 0.5f;
    return clamp(base - bp, 0.1f, 1.0f);
}

/* ─────────────────────────────────────────────────────────────
 *  hos_log() — kernel ring buffer
 *  OS: printk() / dmesg ring buffer in Linux kernel
 * ──────────────────────────────────────────────────────────── */
void hos_log(HumanOS *os, int level, const char *msg) {
    if (!os || !msg) return;

    int idx = os->log_head % MAX_LOG_ENTRIES;
    LogEntry *e = &os->log[idx];
    e->timestamp = time(NULL);
    e->level     = level;
    strncpy(e->message, msg, MAX_LOG_MSG - 1);

    os->log_head++;
    if (os->log_count < MAX_LOG_ENTRIES) os->log_count++;
}

/* ─────────────────────────────────────────────────────────────
 *  hos_boot() — kernel initialization
 *  OS: kernel_start() → init subsystems → mount root fs → run init
 * ──────────────────────────────────────────────────────────── */
void hos_boot(HumanOS *os, float mental_cap, float phys_cap, float emot_cap) {
    if (!os) return;
    memset(os, 0, sizeof(HumanOS));

    /* Initialize all subsystems */
    os->task_mgr.next_id    = 1;
    os->skills.next_id      = 1;
    os->life_mgr.next_id    = 1;

    /* Configure energy system */
    os->energy.mental_cap    = mental_cap;
    os->energy.physical_cap  = phys_cap;
    os->energy.emotional_cap = emot_cap;
    os->energy.mental        = mental_cap;
    os->energy.physical      = phys_cap;
    os->energy.emotional     = emot_cap;

    /* System defaults */
    os->mode         = MODE_PRODUCTIVE;
    os->status       = STATUS_OPTIMAL;
    os->boot_time    = time(NULL);
    os->day_number   = 1;
    os->week_number  = 1;
    os->uptime_hours = 0.0f;

    os->config_work_hours_per_day  = 8;
    os->config_energy_mental_cap   = mental_cap;
    os->config_energy_physical_cap = phys_cap;
    os->config_energy_emotional_cap= emot_cap;

    /* Set hour of day */
    struct tm *lt = localtime(&os->boot_time);
    os->energy.hour_of_day = lt ? lt->tm_hour : 9;

    hos_log(os, 0, "Human OS booted successfully");
    hos_log(os, 0, "All subsystems initialized");

    printf("[HumanOS] Booted (mental=%.0f, physical=%.0f, emotional=%.0f)\n",
           mental_cap, phys_cap, emot_cap);
}

/* ─────────────────────────────────────────────────────────────
 *  Task management
 * ──────────────────────────────────────────────────────────── */
HTask* hos_add_task(HumanOS *os, const char *name, const char *category,
                    HTaskPriority priority, float duration_min,
                    float energy_req, float value) {
    if (!os || os->task_mgr.count >= MAX_TASKS) return NULL;

    HTask *t = &os->task_mgr.tasks[os->task_mgr.count++];
    memset(t, 0, sizeof(HTask));
    t->id             = os->task_mgr.next_id++;
    t->priority       = priority;
    t->status         = TASK_PENDING;
    t->energy_type    = ENERGY_MENTAL;
    t->duration_minutes = duration_min;
    t->energy_required  = energy_req;
    t->value_score      = value;
    t->created_at       = time(NULL);
    strncpy(t->name,     name,     MAX_TASK_NAME - 1);
    strncpy(t->category, category, MAX_TASK_CAT  - 1);

    char log_msg[MAX_LOG_MSG];
    snprintf(log_msg, sizeof(log_msg), "Task added: '%s'", name);
    hos_log(os, 0, log_msg);
    return t;
}

int hos_start_task(HumanOS *os, uint32_t task_id) {
    if (!os) return -1;
    for (int i = 0; i < os->task_mgr.count; i++) {
        HTask *t = &os->task_mgr.tasks[i];
        if (t->id == task_id && t->status == TASK_PENDING) {
            t->status = TASK_RUNNING;
            return 0;
        }
    }
    return -1;
}

int hos_complete_task(HumanOS *os, uint32_t task_id) {
    if (!os) return -1;
    for (int i = 0; i < os->task_mgr.count; i++) {
        HTask *t = &os->task_mgr.tasks[i];
        if (t->id == task_id && (t->status == TASK_RUNNING || t->status == TASK_PENDING)) {
            t->status   = TASK_COMPLETED;
            t->progress = 100.0f;
            char msg[MAX_LOG_MSG];
            snprintf(msg, sizeof(msg), "Completed: '%s'", t->name);
            hos_log(os, 0, msg);
            return 0;
        }
    }
    return -1;
}

/* ─────────────────────────────────────────────────────────────
 *  hos_get_next_task() — the intelligent scheduler
 *
 *  OS: the dispatcher — picks the next process to run on CPU.
 *  This is where all subsystems integrate:
 *    1. Check energy: can we do demanding work?
 *    2. Check mode: what kind of work is appropriate?
 *    3. Check deadline: any overdue tasks?
 *    4. Check priority: highest urgency pending task
 *
 *  Returns the best task to work on RIGHT NOW.
 * ──────────────────────────────────────────────────────────── */
HTask* hos_get_next_task(HumanOS *os) {
    if (!os) return NULL;

    float energy_pct    = energy_overall(&os->energy);
    float efficiency    = energy_efficiency(&os->energy);
    time_t now          = time(NULL);

    HTask *best = NULL;
    float  best_score = -1.0f;

    for (int i = 0; i < os->task_mgr.count; i++) {
        HTask *t = &os->task_mgr.tasks[i];
        if (t->status != TASK_PENDING) continue;

        /* Base urgency score: 100/75/50/25/10 for priority */
        float priority_scores[] = {100.0f, 75.0f, 50.0f, 25.0f, 10.0f};
        float score = priority_scores[(int)t->priority < 5 ? (int)t->priority : 4];

        /* Deadline pressure */
        if (t->deadline > 0) {
            double secs = difftime(t->deadline, now);
            if      (secs <= 0)      score += 50.0f;  /* overdue */
            else if (secs < 3600)    score += 40.0f;  /* < 1h    */
            else if (secs < 86400)   score += 20.0f;  /* < 1 day */
        }

        /* Energy matching: mental tasks need mental energy */
        if (t->energy_type == ENERGY_MENTAL && energy_pct < 40.0f) {
            score *= 0.5f;  /* penalize mental tasks when tired */
        }

        /* Emergency mode: only critical tasks */
        if (os->mode == MODE_EMERGENCY && t->priority != PRIORITY_CRITICAL) {
            score *= 0.1f;
        }

        /* Recovery mode: skip high-energy tasks */
        if (os->mode == MODE_RECOVERY && t->energy_required > 20.0f) {
            score *= 0.2f;
        }

        /* Efficiency modifier */
        score *= efficiency;

        /* Value bonus */
        score += t->value_score * 0.5f;

        if (score > best_score) {
            best_score = score;
            best = t;
        }
    }

    return best;
}

/* ─────────────────────────────────────────────────────────────
 *  Energy management
 * ──────────────────────────────────────────────────────────── */
float hos_get_energy_pct(const HumanOS *os) {
    if (!os) return 0.0f;
    return energy_overall(&os->energy);
}

void hos_work(HumanOS *os, uint32_t task_id, float duration_min) {
    if (!os) return;

    HTask *t = NULL;
    for (int i = 0; i < os->task_mgr.count; i++) {
        if (os->task_mgr.tasks[i].id == task_id) {
            t = &os->task_mgr.tasks[i];
            break;
        }
    }

    float mental   = 0, physical = 0, emotional = 0;
    if (t) {
        float eff = energy_efficiency(&os->energy);
        float cost = t->energy_required * (1.0f / fmaxf(eff, 0.1f)) *
                     (duration_min / fmaxf(t->duration_minutes, 1.0f));
        switch (t->energy_type) {
            case ENERGY_MENTAL:    mental   = cost; break;
            case ENERGY_PHYSICAL:  physical = cost; break;
            case ENERGY_EMOTIONAL: emotional= cost; break;
            case ENERGY_MIXED:     mental=cost*0.4f; physical=cost*0.3f;
                                   emotional=cost*0.3f; break;
        }
    } else {
        mental = duration_min * 0.2f;
    }

    /* Consume energy */
    os->energy.mental    = clamp(os->energy.mental    - mental,    0, os->energy.mental_cap);
    os->energy.physical  = clamp(os->energy.physical  - physical,  0, os->energy.physical_cap);
    os->energy.emotional = clamp(os->energy.emotional - emotional, 0, os->energy.emotional_cap);

    /* Update ultradian cycle */
    os->energy.ultradian_phase += duration_min;
    os->energy.total_work_minutes += duration_min;
    os->uptime_hours += duration_min / 60.0f;

    /* Burnout accumulation: more when tired */
    float fatigue_factor = 1.0f - energy_efficiency(&os->energy);
    os->energy.burnout_score = clamp(
        os->energy.burnout_score + (duration_min / 60.0f) * fatigue_factor * 5.0f,
        0.0f, 100.0f);

    /* History snapshot */
    if (os->energy.history_idx < ENERGY_HISTORY_SIZE) {
        os->energy.history_mental[os->energy.history_idx++] = os->energy.mental;
    }
}

void hos_rest(HumanOS *os, float duration_min, const char *activity) {
    if (!os) return;

    /* Different activities restore different energies */
    float m = 0, p = 0, e = 0;
    float hours = duration_min / 60.0f;

    if (activity && strstr(activity, "exercise"))  { p = 15.0f * hours; m = 3.0f * hours; }
    else if (activity && strstr(activity, "social")){ e = 12.0f * hours; m = 2.0f * hours; }
    else if (activity && strstr(activity, "sleep")) { m = 12.0f * hours; p = 10.0f * hours; e = 10.0f * hours; }
    else if (activity && strstr(activity, "meditat")){ m = 10.0f * hours; e = 8.0f * hours; }
    else { /* generic break */
        m = 5.0f * hours;
        p = 4.0f * hours;
        e = 4.0f * hours;
    }

    os->energy.mental    = clamp(os->energy.mental    + m, 0, os->energy.mental_cap);
    os->energy.physical  = clamp(os->energy.physical  + p, 0, os->energy.physical_cap);
    os->energy.emotional = clamp(os->energy.emotional + e, 0, os->energy.emotional_cap);

    /* Rest reduces burnout */
    os->energy.burnout_score = clamp(
        os->energy.burnout_score - (hours * 8.0f), 0.0f, 100.0f);

    /* Reset ultradian cycle on meaningful break */
    if (duration_min >= 15.0f) os->energy.ultradian_phase = 0.0f;

    char msg[MAX_LOG_MSG];
    snprintf(msg, sizeof(msg), "Rested: %s (%.0f min)", activity ? activity : "break", duration_min);
    hos_log(os, 0, msg);
}

void hos_sleep(HumanOS *os, float hours) {
    if (!os) return;

    float quality = clamp(hours / 8.0f, 0.1f, 1.0f);
    os->energy.mental    = clamp(os->energy.mental    + os->energy.mental_cap    * 0.9f * quality, 0, os->energy.mental_cap);
    os->energy.physical  = clamp(os->energy.physical  + os->energy.physical_cap  * 0.85f * quality, 0, os->energy.physical_cap);
    os->energy.emotional = clamp(os->energy.emotional + os->energy.emotional_cap * 0.8f  * quality, 0, os->energy.emotional_cap);
    os->energy.burnout_score = clamp(os->energy.burnout_score - hours * 8.0f, 0.0f, 100.0f);
    os->energy.ultradian_phase   = 0.0f;
    os->energy.total_work_minutes= 0.0f;
    os->energy.history_idx       = 0;

    hos_log(os, 0, "Sleep complete — energy restored");
}

/* ─────────────────────────────────────────────────────────────
 *  Goal and habit management
 * ──────────────────────────────────────────────────────────── */
HGoal* hos_add_goal(HumanOS *os, const char *name, float target, float importance) {
    if (!os || os->life_mgr.goal_count >= MAX_GOALS) return NULL;
    HGoal *g = &os->life_mgr.goals[os->life_mgr.goal_count++];
    memset(g, 0, sizeof(HGoal));
    g->id         = os->life_mgr.next_id++;
    g->target     = target;
    g->importance = importance;
    g->current    = 0.0f;
    g->active     = 1;
    strncpy(g->name, name, MAX_GOAL_NAME - 1);
    return g;
}

void hos_update_goal(HumanOS *os, uint32_t id, float progress) {
    if (!os) return;
    for (int i = 0; i < os->life_mgr.goal_count; i++) {
        if (os->life_mgr.goals[i].id == id) {
            os->life_mgr.goals[i].current = clamp(
                os->life_mgr.goals[i].current + progress,
                0.0f, os->life_mgr.goals[i].target);
            return;
        }
    }
}

HHabit* hos_add_habit(HumanOS *os, const char *name, int is_positive) {
    if (!os || os->life_mgr.habit_count >= MAX_HABITS) return NULL;
    HHabit *h = &os->life_mgr.habits[os->life_mgr.habit_count++];
    memset(h, 0, sizeof(HHabit));
    h->id          = os->life_mgr.next_id++;
    h->is_positive = is_positive;
    strncpy(h->name, name, MAX_HABIT_NAME - 1);
    return h;
}

void hos_mark_habit_done(HumanOS *os, uint32_t habit_id) {
    if (!os) return;
    for (int i = 0; i < os->life_mgr.habit_count; i++) {
        if (os->life_mgr.habits[i].id == habit_id) {
            os->life_mgr.habits[i].streak++;
            os->life_mgr.habits[i].done_today = 1;
            os->life_mgr.habits[i].completion_rate =
                clamp(os->life_mgr.habits[i].completion_rate + 5.0f, 0, 100);
            return;
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 *  Skill tracking
 * ──────────────────────────────────────────────────────────── */
HSkill* hos_add_skill(HumanOS *os, const char *name) {
    if (!os || os->skills.count >= MAX_SKILLS) return NULL;
    HSkill *s = &os->skills.skills[os->skills.count++];
    memset(s, 0, sizeof(HSkill));
    s->id       = os->skills.next_id++;
    s->level    = SKILL_NOVICE;
    s->xp       = 0.0f;
    s->xp_needed= 100.0f;
    strncpy(s->name, name, MAX_SKILL_NAME - 1);
    return s;
}

void hos_practice_skill(HumanOS *os, uint32_t skill_id, float hours) {
    if (!os) return;
    for (int i = 0; i < os->skills.count; i++) {
        HSkill *s = &os->skills.skills[i];
        if (s->id == skill_id) {
            float level_factor = 1.0f / (1.0f + (float)s->level * 0.2f);
            float xp = hours * 30.0f * level_factor;
            s->xp    += xp;
            s->hours += hours;
            s->streak++;
            os->skills.total_hours += hours;

            /* Level up */
            while (s->xp >= s->xp_needed && s->level < SKILL_EXPERT) {
                s->xp -= s->xp_needed;
                s->xp_needed *= 1.5f;
                s->level = (HSkillLevel)((int)s->level + 1);
                char msg[MAX_LOG_MSG];
                snprintf(msg, sizeof(msg), "LEVEL UP: '%s' → %s",
                         s->name, hos_skill_level_name(s->level));
                hos_log(os, 0, msg);
                printf("  🎉 %s\n", msg);
            }
            return;
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 *  hos_auto_mode() — automatic mode selection (kernel governor)
 *  OS: Linux CPU frequency governor (performance/powersave/ondemand)
 * ──────────────────────────────────────────────────────────── */
void hos_auto_mode(HumanOS *os) {
    if (!os) return;

    float energy_pct = energy_overall(&os->energy);
    float burnout    = os->energy.burnout_score;
    int   hour       = os->energy.hour_of_day;

    SystemMode new_mode = os->mode;

    /* Emergency: burnout critical or very low energy */
    if (burnout >= 85.0f || energy_pct < 15.0f) {
        new_mode = MODE_RECOVERY;
        hos_log(os, 2, "AUTO: Switched to RECOVERY mode (critical state)");
    }
    /* Morning peak (9–12): focused deep work */
    else if (hour >= 9 && hour < 12 && energy_pct >= 70.0f) {
        new_mode = MODE_FOCUSED;
    }
    /* Afternoon dip (12–14): lighter work */
    else if (hour >= 12 && hour < 14) {
        new_mode = MODE_PRODUCTIVE;
    }
    /* Late evening: learning */
    else if (hour >= 19 && hour < 22 && energy_pct >= 40.0f) {
        new_mode = MODE_LEARNING;
    }
    /* Low energy: recovery */
    else if (energy_pct < 30.0f) {
        new_mode = MODE_RECOVERY;
    }
    else {
        new_mode = MODE_PRODUCTIVE;
    }

    if (new_mode != os->mode) {
        char msg[MAX_LOG_MSG];
        snprintf(msg, sizeof(msg), "Mode: %s → %s",
                 hos_mode_name(os->mode), hos_mode_name(new_mode));
        hos_log(os, 0, msg);
        os->mode = new_mode;
    }
}

/* ─────────────────────────────────────────────────────────────
 *  hos_health_check() — watchdog daemon
 *  OS: kernel watchdog that monitors system health
 * ──────────────────────────────────────────────────────────── */
void hos_health_check(HumanOS *os) {
    if (!os) return;

    float energy_pct = energy_overall(&os->energy);
    float burnout    = os->energy.burnout_score;
    SystemStatus new_status = STATUS_OPTIMAL;

    if      (burnout >= 80.0f || energy_pct < 20.0f)  new_status = STATUS_SHUTDOWN;
    else if (burnout >= 60.0f || energy_pct < 40.0f)  new_status = STATUS_CRITICAL;
    else if (burnout >= 40.0f || energy_pct < 60.0f)  new_status = STATUS_DEGRADED;

    if (new_status != os->status) {
        char msg[MAX_LOG_MSG];
        snprintf(msg, sizeof(msg), "Status: %s → %s",
                 hos_status_name(os->status), hos_status_name(new_status));
        hos_log(os, new_status >= STATUS_CRITICAL ? 2 : 1, msg);
        os->status = new_status;
    }
}

/* ─────────────────────────────────────────────────────────────
 *  hos_compute_metrics() — calculate system performance metrics
 * ──────────────────────────────────────────────────────────── */
SystemMetrics hos_compute_metrics(const HumanOS *os) {
    SystemMetrics m = {0};
    if (!os) return m;

    /* Productivity: completed tasks / total tasks */
    int total = 0, completed = 0;
    for (int i = 0; i < os->task_mgr.count; i++) {
        total++;
        if (os->task_mgr.tasks[i].status == TASK_COMPLETED) completed++;
    }
    m.productivity_score = total > 0 ? (float)completed / total * 100.0f : 0;

    /* Energy efficiency */
    m.energy_efficiency = energy_efficiency(&os->energy);

    /* Goal velocity: avg % complete */
    float goal_sum = 0;
    for (int i = 0; i < os->life_mgr.goal_count; i++) {
        const HGoal *g = &os->life_mgr.goals[i];
        if (g->active && g->target > 0)
            goal_sum += g->current / g->target * 100.0f;
    }
    m.goal_velocity = os->life_mgr.goal_count > 0 ?
                      goal_sum / os->life_mgr.goal_count : 0;

    /* Learning rate: total skill hours */
    m.learning_rate = os->skills.total_hours > 0 ?
                      os->skills.total_hours / fmaxf((float)os->day_number, 1.0f) : 0;

    /* Habit adherence: avg completion rate */
    float hab_sum = 0;
    for (int i = 0; i < os->life_mgr.habit_count; i++)
        hab_sum += os->life_mgr.habits[i].completion_rate;
    m.habit_adherence = os->life_mgr.habit_count > 0 ?
                        hab_sum / os->life_mgr.habit_count : 0;

    /* Burnout index */
    m.burnout_index = os->energy.burnout_score;

    /* Work-life balance: focus time ratio */
    float total_minutes = os->energy.total_work_minutes +
                          (float)(os->config_work_hours_per_day * 60 -
                                  (int)os->energy.total_work_minutes);
    m.focus_ratio = total_minutes > 0 ?
                    (os->energy.total_work_minutes / total_minutes) * 100.0f : 0;

    return m;
}

/* ─────────────────────────────────────────────────────────────
 *  hos_new_day() — daily reset
 * ──────────────────────────────────────────────────────────── */
void hos_new_day(HumanOS *os) {
    if (!os) return;
    os->day_number++;
    if (os->day_number % 7 == 0) os->week_number++;

    os->energy.total_work_minutes = 0;
    os->energy.ultradian_phase    = 0;
    os->energy.history_idx        = 0;

    /* Reset habit done_today flags */
    for (int i = 0; i < os->life_mgr.habit_count; i++)
        os->life_mgr.habits[i].done_today = 0;

    char msg[MAX_LOG_MSG];
    snprintf(msg, sizeof(msg), "New day #%d (week %d)",
             os->day_number, os->week_number);
    hos_log(os, 0, msg);
}

void hos_shutdown(HumanOS *os) {
    if (!os) return;
    hos_log(os, 0, "Human OS graceful shutdown initiated");
    printf("[HumanOS] Shutdown complete after %d days, %.1f hours.\n",
           os->day_number, os->uptime_hours);
}

/* ─────────────────────────────────────────────────────────────
 *  Dashboard — the terminal UI
 *  OS: like `htop` + `/proc/status` combined
 * ──────────────────────────────────────────────────────────── */

/* Simple bar chart helper */
static void print_bar(float value, float max_val, int width, const char *fill) {
    int filled = max_val > 0 ? (int)(value / max_val * width) : 0;
    if (filled > width) filled = width;
    printf("[");
    for (int i = 0; i < width; i++)
        printf(i < filled ? (value/max_val > 0.6f ? "█" :
               value/max_val > 0.3f ? "▓" : "░") : " ");
    printf("]");
}

void hos_print_status_bar(const HumanOS *os) {
    if (!os) return;
    float energy = energy_overall(&os->energy);
    const char *status_icons[] = {"✅", "⚠️ ", "🔴", "💀"};
    const char *mode_icons[]   = {"📋", "🎯", "😴", "🚨", "📚"};

    printf("  %s %s  | Day %-3d | Energy: %5.1f%% | Burnout: %5.1f%% | Mode: %s %s\n",
           status_icons[(int)os->status < 4 ? (int)os->status : 0],
           hos_status_name(os->status),
           os->day_number,
           energy,
           os->energy.burnout_score,
           mode_icons[(int)os->mode < 5 ? (int)os->mode : 0],
           hos_mode_name(os->mode));
}

void hos_print_energy_bars(const HumanOS *os) {
    if (!os) return;
    const HEnergyState *e = &os->energy;

    printf("  ┌─ Energy ──────────────────────────────────────────\n");
    printf("  │  Mental    "); print_bar(e->mental,   e->mental_cap,   30, "█");
    printf(" %5.1f/%.0f\n",  e->mental,   e->mental_cap);

    printf("  │  Physical  "); print_bar(e->physical, e->physical_cap, 30, "█");
    printf(" %5.1f/%.0f\n",  e->physical, e->physical_cap);

    printf("  │  Emotional "); print_bar(e->emotional,e->emotional_cap,30, "█");
    printf(" %5.1f/%.0f\n",  e->emotional,e->emotional_cap);

    printf("  │  Overall   "); print_bar(energy_overall(e), 100.0f, 30, "█");
    printf(" %5.1f%%\n", energy_overall(e));

    printf("  │  Burnout   "); print_bar(e->burnout_score, 100.0f, 30, "░");
    printf(" %5.1f%% %s\n",
           e->burnout_score,
           e->burnout_score >= 70 ? "⚠ RISK" : "OK");

    printf("  │  Ultradian "); print_bar(e->ultradian_phase, 90.0f, 30, "▒");
    printf(" %5.0f/90 min\n", e->ultradian_phase);
    printf("  └───────────────────────────────────────────────────\n");
}

void hos_print_task_table(const HumanOS *os) {
    if (!os || os->task_mgr.count == 0) {
        printf("  (no tasks)\n");
        return;
    }

    const char *status_sym[] = {"⬜", "▶️ ", "⏸️ ", "✅", "❌", "⏯️ "};
    const char *pri_sym[]    = {"🔴", "🟠", "🟡", "🟢", "⚪"};

    printf("  ┌─ Tasks (%d) ─────────────────────────────────────\n",
           os->task_mgr.count);
    printf("  │  %-4s %-28s %-8s %5s %5s\n",
           "ID", "Name", "Status", "Dur", "Val");

    for (int i = 0; i < os->task_mgr.count; i++) {
        const HTask *t = &os->task_mgr.tasks[i];
        if (t->status == TASK_COMPLETED) continue;  /* skip completed */
        int p = (int)t->priority < 5 ? (int)t->priority : 4;
        int s = (int)t->status   < 6 ? (int)t->status   : 0;
        printf("  │  %s %-4u %-28s %s  %4.0fm %4.1f\n",
               pri_sym[p], t->id, t->name, status_sym[s],
               t->duration_minutes, t->value_score);
    }
    printf("  └───────────────────────────────────────────────────\n");
}

void hos_print_goals_summary(const HumanOS *os) {
    if (!os || os->life_mgr.goal_count == 0) return;
    printf("  ┌─ Goals ────────────────────────────────────────────\n");
    for (int i = 0; i < os->life_mgr.goal_count; i++) {
        const HGoal *g = &os->life_mgr.goals[i];
        if (!g->active) continue;
        float pct = g->target > 0 ? g->current / g->target * 100.0f : 0;
        printf("  │  %-28s ", g->name);
        print_bar(pct, 100.0f, 15, "█");
        printf(" %5.1f%%\n", pct);
    }
    printf("  └───────────────────────────────────────────────────\n");
}

void hos_print_skill_levels(const HumanOS *os) {
    if (!os || os->skills.count == 0) return;
    printf("  ┌─ Skills ──────────────────────────────────────────\n");
    for (int i = 0; i < os->skills.count; i++) {
        const HSkill *s = &os->skills.skills[i];
        float pct = s->xp_needed > 0 ? s->xp / s->xp_needed * 100.0f : 100.0f;
        printf("  │  %-24s  %-12s ", s->name, hos_skill_level_name(s->level));
        print_bar(pct, 100.0f, 10, "█");
        printf(" %5.1fh  🔥%d\n", s->hours, s->streak);
    }
    printf("  └───────────────────────────────────────────────────\n");
}

void hos_print_metrics(const HumanOS *os) {
    SystemMetrics m = hos_compute_metrics(os);
    printf("  ┌─ System Metrics ───────────────────────────────────\n");
    printf("  │  Productivity:    %5.1f%%\n", m.productivity_score);
    printf("  │  Energy Effic.:   %5.0f%%\n", m.energy_efficiency * 100.0f);
    printf("  │  Goal Progress:   %5.1f%%\n", m.goal_velocity);
    printf("  │  Learning Rate:   %5.2f h/day\n", m.learning_rate);
    printf("  │  Habit Adherence: %5.1f%%\n", m.habit_adherence);
    printf("  │  Burnout Index:   %5.1f / 100\n", m.burnout_index);
    printf("  └───────────────────────────────────────────────────\n");
}

void hos_print_log(const HumanOS *os, int last_n) {
    if (!os || os->log_count == 0) return;
    const char *level_names[] = {"INFO", "WARN", "ERR "};
    const char *level_icons[] = {"  ", "⚠️", "🔴"};

    int start = os->log_head - last_n;
    if (start < 0) start = 0;

    printf("  ┌─ System Log (last %d entries) ─────────────────────\n",
           last_n < os->log_count ? last_n : os->log_count);
    for (int i = start; i < os->log_head && i < start + last_n; i++) {
        const LogEntry *e = &os->log[i % MAX_LOG_ENTRIES];
        char ts[16];
        struct tm *lt = localtime(&e->timestamp);
        strftime(ts, sizeof(ts), "%H:%M:%S", lt);
        int lv = e->level < 3 ? e->level : 0;
        printf("  │  %s %s [%s] %s\n",
               ts, level_icons[lv], level_names[lv], e->message);
    }
    printf("  └───────────────────────────────────────────────────\n");
}

void hos_dashboard(const HumanOS *os) {
    if (!os) return;

    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║              HUMAN OS DASHBOARD                     ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
    hos_print_status_bar(os);
    printf("\n");
    hos_print_energy_bars(os);
    printf("\n");
    hos_print_task_table(os);
    printf("\n");
    hos_print_goals_summary(os);
    printf("\n");
    hos_print_skill_levels(os);
    printf("\n");
    hos_print_metrics(os);
    printf("\n");
    hos_print_log(os, 6);
    printf("\n");
}
