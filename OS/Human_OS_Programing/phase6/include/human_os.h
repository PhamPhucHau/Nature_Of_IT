/*
 * human_os.h  —  Phase 6: Integrated Human Operating System
 *
 * This is the master header that unifies all phases into one system.
 *
 * OS Analogy: This is the KERNEL — the core that coordinates all
 * subsystems: scheduler, memory manager, I/O system, file system.
 *
 * Human OS Architecture:
 *
 *   ┌──────────────────────────────────────────────────────────┐
 *   │                    HUMAN OS KERNEL                       │
 *   ├────────────┬─────────────┬──────────────┬───────────────┤
 *   │  Task Mgr  │ Energy Mgr  │ Decision Eng │ Learning Sys  │
 *   │ (Phase 1-2)│  (Phase 3)  │  (Phase 4)   │  (Phase 5)    │
 *   ├────────────┴─────────────┴──────────────┴───────────────┤
 *   │                    DASHBOARD / SHELL                     │
 *   │              (Real-time metrics display)                 │
 *   └──────────────────────────────────────────────────────────┘
 *
 * Key OS concepts demonstrated:
 *   - Kernel coordination of subsystems (scheduler + resource manager)
 *   - System calls (human_os_run_tick = one kernel tick)
 *   - Inter-process communication (subsystems share state)
 *   - System health monitoring (dashboard = /proc filesystem)
 *   - Performance optimization (tune subsystem parameters)
 */

#ifndef HUMAN_OS_H
#define HUMAN_OS_H

#include <time.h>
#include <stdint.h>

/* ─────────────────────────────────────────────────────────────
 *  EMBEDDED SUBSYSTEM DEFINITIONS
 *  (Each subsystem carries its state inline in the kernel struct)
 * ──────────────────────────────────────────────────────────── */

/* --- Minimal task definitions (from Phase 1/2) --- */
#define MAX_TASKS        50
#define MAX_TASK_NAME    64
#define MAX_TASK_DESC   128
#define MAX_TASK_CAT     32

typedef enum { TASK_PENDING=0, TASK_RUNNING, TASK_BLOCKED,
               TASK_COMPLETED, TASK_FAILED, TASK_PAUSED } HTaskStatus;
typedef enum { PRIORITY_CRITICAL=0, PRIORITY_HIGH, PRIORITY_MEDIUM,
               PRIORITY_LOW, PRIORITY_OPTIONAL } HTaskPriority;
typedef enum { ENERGY_MENTAL=0, ENERGY_PHYSICAL,
               ENERGY_EMOTIONAL, ENERGY_MIXED } HEnergyType;

typedef struct {
    uint32_t       id;
    char           name[MAX_TASK_NAME];
    char           category[MAX_TASK_CAT];
    HTaskPriority  priority;
    HTaskStatus    status;
    HEnergyType    energy_type;
    float          duration_minutes;
    float          energy_required;
    float          progress;
    float          value_score;
    time_t         created_at;
    time_t         deadline;
} HTask;

typedef struct {
    HTask    tasks[MAX_TASKS];
    int      count;
    uint32_t next_id;
} HTaskManager;

/* --- Minimal energy definitions (from Phase 3) --- */
#define ENERGY_HISTORY_SIZE 48  /* 48 × 30-min = 24h history */

typedef struct {
    float   mental;
    float   physical;
    float   emotional;
    float   mental_cap;
    float   physical_cap;
    float   emotional_cap;
    float   burnout_score;
    float   ultradian_phase;     /* minutes into current 90-min cycle */
    float   total_work_minutes;
    float   history_mental[ENERGY_HISTORY_SIZE];
    int     history_idx;
    int     hour_of_day;
} HEnergyState;

/* --- Minimal goal definitions (from Phase 4) --- */
#define MAX_GOALS     10
#define MAX_HABITS    15
#define MAX_GOAL_NAME 64
#define MAX_HABIT_NAME 64

typedef struct {
    uint32_t id;
    char     name[MAX_GOAL_NAME];
    float    current;
    float    target;
    float    importance;
    int      active;
} HGoal;

typedef struct {
    uint32_t id;
    char     name[MAX_HABIT_NAME];
    int      streak;
    float    completion_rate;
    int      is_positive;
    int      done_today;
} HHabit;

typedef struct {
    HGoal  goals[MAX_GOALS];
    int    goal_count;
    HHabit habits[MAX_HABITS];
    int    habit_count;
    uint32_t next_id;
} HLifeManager;

/* --- Minimal skill definitions (from Phase 5) --- */
#define MAX_SKILLS     20
#define MAX_SKILL_NAME 64

typedef enum {
    SKILL_NOVICE=1, SKILL_BEGINNER, SKILL_COMPETENT,
    SKILL_PROFICIENT, SKILL_EXPERT
} HSkillLevel;

typedef struct {
    uint32_t    id;
    char        name[MAX_SKILL_NAME];
    HSkillLevel level;
    float       xp;
    float       xp_needed;
    float       hours;
    int         streak;
} HSkill;

typedef struct {
    HSkill skills[MAX_SKILLS];
    int    count;
    float  total_hours;
    uint32_t next_id;
} HSkillTracker;

/* ─────────────────────────────────────────────────────────────
 *  HUMAN OS KERNEL — the master system struct
 *  OS: struct representing the entire kernel state
 * ──────────────────────────────────────────────────────────── */

/* System run mode */
typedef enum {
    MODE_PRODUCTIVE  = 0,   /* normal full-capacity work mode         */
    MODE_FOCUSED     = 1,   /* deep work: block distractions          */
    MODE_RECOVERY    = 2,   /* rest: minimal tasks, prioritize energy */
    MODE_EMERGENCY   = 3,   /* crisis: only critical tasks            */
    MODE_LEARNING    = 4,   /* study mode: learning tasks prioritized */
} SystemMode;

/* System health status */
typedef enum {
    STATUS_OPTIMAL  = 0,   /* everything running well                 */
    STATUS_DEGRADED = 1,   /* some issues, performance impacted       */
    STATUS_CRITICAL = 2,   /* serious issues, intervention needed     */
    STATUS_SHUTDOWN = 3,   /* burnout / system needs full reset       */
} SystemStatus;

/* Performance metrics — like /proc/stat in Linux */
typedef struct {
    float productivity_score;   /* 0–100: tasks completed vs planned     */
    float energy_efficiency;    /* 0–1.0: output per energy unit         */
    float goal_velocity;        /* avg goal progress per day             */
    float learning_rate;        /* skill XP gained per day               */
    float habit_adherence;      /* % of habits completed as scheduled    */
    float work_life_balance;    /* ratio: personal vs work time          */
    float focus_ratio;          /* % of time in deep work vs shallow     */
    float burnout_index;        /* 0–100: risk level                     */
} SystemMetrics;

/* Daily log entry */
#define MAX_LOG_ENTRIES  30
#define MAX_LOG_MSG     128

typedef struct {
    time_t timestamp;
    char   message[MAX_LOG_MSG];
    int    level;   /* 0=INFO, 1=WARN, 2=ERROR */
} LogEntry;

/* The main kernel struct */
typedef struct {
    /* Subsystems */
    HTaskManager  task_mgr;
    HEnergyState  energy;
    HLifeManager  life_mgr;
    HSkillTracker skills;

    /* System state */
    SystemMode    mode;
    SystemStatus  status;
    SystemMetrics metrics;

    /* Uptime tracking */
    time_t        boot_time;
    int           day_number;
    int           week_number;
    float         uptime_hours;

    /* System log (ring buffer) */
    LogEntry      log[MAX_LOG_ENTRIES];
    int           log_head;
    int           log_count;

    /* Configuration */
    int           config_work_hours_per_day;
    float         config_energy_mental_cap;
    float         config_energy_physical_cap;
    float         config_energy_emotional_cap;
} HumanOS;

/* ─────────────────────────────────────────────────────────────
 *  FUNCTION DECLARATIONS
 * ──────────────────────────────────────────────────────────── */

/* Kernel lifecycle */
void  hos_boot(HumanOS *os, float mental_cap, float phys_cap, float emot_cap);
void  hos_new_day(HumanOS *os);
void  hos_shutdown(HumanOS *os);

/* Task management */
HTask* hos_add_task(HumanOS *os, const char *name, const char *category,
                    HTaskPriority priority, float duration_min,
                    float energy_req, float value);
int    hos_start_task(HumanOS *os, uint32_t task_id);
int    hos_complete_task(HumanOS *os, uint32_t task_id);
HTask* hos_get_next_task(HumanOS *os);  /* smart scheduler */

/* Energy management */
void  hos_work(HumanOS *os, uint32_t task_id, float duration_min);
void  hos_rest(HumanOS *os, float duration_min, const char *activity);
void  hos_sleep(HumanOS *os, float hours);
float hos_get_energy_pct(const HumanOS *os);

/* Goal and habit management */
HGoal*  hos_add_goal(HumanOS *os, const char *name, float target, float importance);
void    hos_update_goal(HumanOS *os, uint32_t id, float progress);
HHabit* hos_add_habit(HumanOS *os, const char *name, int is_positive);
void    hos_mark_habit_done(HumanOS *os, uint32_t habit_id);

/* Skill tracking */
HSkill* hos_add_skill(HumanOS *os, const char *name);
void    hos_practice_skill(HumanOS *os, uint32_t skill_id, float hours);

/* System intelligence */
void    hos_auto_mode(HumanOS *os);          /* select best mode automatically */
void    hos_optimize(HumanOS *os);           /* run optimization pass */
void    hos_health_check(HumanOS *os);       /* assess system health */

/* Metrics */
SystemMetrics hos_compute_metrics(const HumanOS *os);
void          hos_log(HumanOS *os, int level, const char *msg);

/* Dashboard (terminal UI) */
void  hos_dashboard(const HumanOS *os);
void  hos_print_status_bar(const HumanOS *os);
void  hos_print_task_table(const HumanOS *os);
void  hos_print_energy_bars(const HumanOS *os);
void  hos_print_goals_summary(const HumanOS *os);
void  hos_print_skill_levels(const HumanOS *os);
void  hos_print_metrics(const HumanOS *os);
void  hos_print_log(const HumanOS *os, int last_n);

/* Labels */
const char* hos_mode_name(SystemMode m);
const char* hos_status_name(SystemStatus s);
const char* hos_skill_level_name(HSkillLevel l);

#endif /* HUMAN_OS_H */
