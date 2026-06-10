/*
 * main.c  —  Phase 6: Human OS Complete Integration Example
 *
 * Build:  gcc -Wall -Wextra -o phase6 main.c human_os.c -lm
 * Run:    ./phase6
 *
 * This main.c demonstrates all Phase 6 capabilities:
 *
 *  1. Initialize the Human OS kernel
 *  2. Set up tasks, energy, goals, habits, and skills
 *  3. Run a simulation of daily life with the integrated system
 *  4. Display real-time dashboards and metrics
 *  5. Demonstrate automatic mode selection and optimization
 *
 * The simulation runs through one full "day" with typical activities:
 *   - Morning: preparation and planning
 *   - Midday: deep work and recovery
 *   - Afternoon: learning and skill practice
 *   - Evening: reflection and rest
 *
 * Key Learnings:
 *
 *   • How OS kernel coordinates all subsystems (like Linux kernel)
 *   • Real-time scheduling decisions based on system state
 *   • Adaptive behavior (mode selection, energy management)
 *   • Performance monitoring and optimization
 *   • Integration of all five previous phases
 *
 * OS Analogies Demonstrated:
 *
 *   hos_boot()         ← kernel_start() in Linux
 *   hos_work()         ← process execution (uses CPU/energy)
 *   hos_get_next_task()← scheduler dispatch decision
 *   hos_auto_mode()    ← CPU frequency governor
 *   hos_health_check() ← watchdog daemon
 *   hos_optimize()     ← sysctl tuning
 *   hos_dashboard()    ← htop + /proc/stat visualization
 *   hos_log()          ← kernel ring buffer (dmesg)
 */

#include "../include/human_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ─────────────────────────────────────────────────────────────
 *  Helper: Print a fancy separator
 * ──────────────────────────────────────────────────────────── */
void print_separator(const char *title) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║ %-55s ║\n", title);
    printf("╚═══════════════════════════════════════════════════════╝\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Demo Scenario: A typical workday for a developer/learner
 * ──────────────────────────────────────────────────────────── */
int main(void) {
    print_separator("HUMAN OS PHASE 6 - INTEGRATED SYSTEM DEMO");

    /* ─────────────────────────────────────────────────────────────
     *  STAGE 1: KERNEL BOOT
     * ──────────────────────────────────────────────────────────── */

    HumanOS os;
    print_separator("STAGE 1: KERNEL BOOT SEQUENCE");

    printf("Initializing Human OS kernel...\n");
    printf("Configuration:\n");
    printf("  • Mental capacity:    100.0\n");
    printf("  • Physical capacity:  90.0\n");
    printf("  • Emotional capacity: 80.0\n");
    printf("  • Daily work hours:   8\n\n");

    hos_boot(&os, 100.0f, 90.0f, 80.0f);
    os.energy.hour_of_day = 9;  /* Start at 9 AM */

    printf("✅ Kernel boot complete\n");
    printf("   System uptime: 0h\n");
    printf("   Status: %s\n\n", hos_status_name(os.status));

    /* ─────────────────────────────────────────────────────────────
     *  STAGE 2: SUBSYSTEM INITIALIZATION
     * ──────────────────────────────────────────────────────────── */

    print_separator("STAGE 2: SUBSYSTEM INITIALIZATION");

    /* Initialize tasks */
    printf("Loading tasks...\n");
    hos_add_task(&os, "Review project requirements", "work",
                 PRIORITY_HIGH, 60.0f, 40.0f, 90.0f);
    hos_add_task(&os, "Write core feature code", "work",
                 PRIORITY_CRITICAL, 120.0f, 60.0f, 95.0f);
    hos_add_task(&os, "Exercise", "health",
                 PRIORITY_MEDIUM, 45.0f, 50.0f, 70.0f);
    hos_add_task(&os, "Read documentation", "learning",
                 PRIORITY_MEDIUM, 30.0f, 25.0f, 60.0f);
    hos_add_task(&os, "Team meeting", "communication",
                 PRIORITY_HIGH, 60.0f, 30.0f, 50.0f);
    printf("✅ %d tasks loaded\n\n", os.task_mgr.count);

    /* Initialize goals */
    printf("Loading goals...\n");
    hos_add_goal(&os, "Complete Q4 project", 100.0f, 9.5f);
    hos_add_goal(&os, "Learn Go programming", 50.0f, 8.0f);
    hos_add_goal(&os, "Maintain fitness", 75.0f, 7.5f);
    hos_add_goal(&os, "Read 5 books", 5.0f, 6.0f);
    printf("✅ %d goals loaded\n\n", os.life_mgr.goal_count);

    /* Initialize habits */
    printf("Loading habits...\n");
    hos_add_habit(&os, "Daily meditation", 1);
    hos_add_habit(&os, "Code review practice", 1);
    hos_add_habit(&os, "Evening reflection", 1);
    hos_add_habit(&os, "Procrastination", 0);
    printf("✅ %d habits loaded\n\n", os.life_mgr.habit_count);

    /* Initialize skills */
    printf("Loading skills...\n");
    HSkill *s1 = hos_add_skill(&os, "C programming");
    HSkill *s2 = hos_add_skill(&os, "System design");
    HSkill *s3 = hos_add_skill(&os, "Go programming");
    printf("✅ %d skills initialized\n\n", os.skills.count);

    /* Simulate some skill practice to show progression */
    hos_practice_skill(&os, s1->id, 3.5f);
    hos_practice_skill(&os, s2->id, 2.0f);

    /* ─────────────────────────────────────────────────────────────
     *  STAGE 3: SYSTEM OPERATION (Simulated Day)
     * ──────────────────────────────────────────────────────────── */

    print_separator("STAGE 3: DAILY OPERATION SIMULATION");

    printf("Simulating workday...\n\n");

    /* --- 9:00 AM: Morning standup --- */
    printf("[09:00] Morning standup begins\n");
    os.energy.hour_of_day = 9;
    hos_auto_mode(&os);
    hos_health_check(&os);

    HTask *next = hos_get_next_task(&os);
    if (next) {
        printf("   Scheduler selected: '%s' (priority: %d, energy: %.0f)\n",
               next->name, next->priority, next->energy_required);
        hos_start_task(&os, next->id);
        hos_work(&os, next->id, 60.0f);
        hos_complete_task(&os, next->id);
        printf("   ✓ Task completed (%.0f%% energy remaining)\n\n",
               hos_get_energy_pct(&os));
    }

    /* --- 10:30 AM: Deep work session --- */
    printf("[10:30] Deep work session begins\n");
    os.energy.hour_of_day = 10;
    hos_auto_mode(&os);

    next = hos_get_next_task(&os);
    if (next) {
        printf("   Scheduler selected: '%s' (priority: %d, energy: %.0f)\n",
               next->name, next->priority, next->energy_required);
        hos_start_task(&os, next->id);
        hos_work(&os, next->id, 90.0f);
        /* Simulate progress on long task */
        if (next->duration_minutes > 90.0f) {
            next->progress = 60.0f;
            printf("   ⌛ Task in progress (60%% complete, %.0f%% energy)\n\n",
                   hos_get_energy_pct(&os));
        } else {
            hos_complete_task(&os, next->id);
            printf("   ✓ Task completed (%.0f%% energy remaining)\n\n",
                   hos_get_energy_pct(&os));
        }
    }

    /* --- 12:00 PM: Lunch and recovery --- */
    printf("[12:00] Lunch break and recovery\n");
    os.energy.hour_of_day = 12;
    hos_auto_mode(&os);

    printf("   Taking 60-minute break\n");
    hos_rest(&os, 60.0f, "lunch");
    printf("   ✓ Recovery complete (%.0f%% energy restored)\n\n",
           hos_get_energy_pct(&os));

    /* --- 13:00 PM: Meeting --- */
    printf("[13:00] Team meeting\n");
    os.energy.hour_of_day = 13;

    next = hos_get_next_task(&os);
    if (next && strcmp(next->name, "Team meeting") == 0) {
        hos_start_task(&os, next->id);
        hos_work(&os, next->id, 60.0f);
        hos_complete_task(&os, next->id);
        printf("   ✓ Meeting completed\n\n");
    }

    /* --- 14:00 PM: Afternoon learning --- */
    printf("[14:00] Learning and skill development\n");
    os.energy.hour_of_day = 14;
    os.mode = MODE_LEARNING;

    next = hos_get_next_task(&os);
    if (next) {
        printf("   Scheduler selected: '%s' (learning mode)\n", next->name);
        hos_start_task(&os, next->id);
        hos_work(&os, next->id, 30.0f);
        hos_practice_skill(&os, s3->id, 0.5f);  /* Skill practice */
        hos_complete_task(&os, next->id);
        printf("   ✓ Learning session complete\n\n");
    }

    /* --- 15:30 PM: Exercise and physical recovery --- */
    printf("[15:30] Exercise for physical recovery\n");
    os.energy.hour_of_day = 15;

    next = hos_get_next_task(&os);
    if (next && strcmp(next->name, "Exercise") == 0) {
        hos_start_task(&os, next->id);
        hos_work(&os, next->id, 45.0f);
        hos_rest(&os, 45.0f, "exercise");
        hos_complete_task(&os, next->id);
        printf("   ✓ Exercise complete, physical energy recovered\n\n");
    }

    /* --- 17:00 PM: Optimization pass --- */
    printf("[17:00] System optimization pass\n");
    hos_optimize(&os);
    printf("\n");

    /* --- 18:00 PM: Evening reflection --- */
    printf("[18:00] Evening reflection and planning\n");
    os.energy.hour_of_day = 18;
    hos_mark_habit_done(&os, 3);  /* Mark meditation habit */
    hos_update_goal(&os, 1, 10.0f);
    printf("   ✓ Habits updated, goals tracked\n\n");

    /* --- 22:00 PM: Sleep --- */
    printf("[22:00] Sleep and recovery\n");
    os.energy.hour_of_day = 22;
    hos_sleep(&os, 8.0f);
    printf("   ✓ Sleep complete, full energy reset\n\n");

    /* ─────────────────────────────────────────────────────────────
     *  STAGE 4: SYSTEM DIAGNOSTICS
     * ──────────────────────────────────────────────────────────── */

    print_separator("STAGE 4: SYSTEM DIAGNOSTICS");

    /* Final health check */
    hos_health_check(&os);
    printf("Final system status: %s\n\n", hos_status_name(os.status));

    /* Final optimization assessment */
    printf("Running final optimization...\n");
    hos_optimize(&os);
    printf("\n");

    /* ─────────────────────────────────────────────────────────────
     *  STAGE 5: DASHBOARD DISPLAY
     * ──────────────────────────────────────────────────────────── */

    print_separator("STAGE 5: REAL-TIME DASHBOARD");

    hos_dashboard(&os);

    /* ─────────────────────────────────────────────────────────────
     *  STAGE 6: PERFORMANCE ANALYSIS
     * ──────────────────────────────────────────────────────────── */

    print_separator("STAGE 6: PERFORMANCE ANALYSIS");

    SystemMetrics metrics = hos_compute_metrics(&os);

    printf("Daily Performance Report:\n");
    printf("┌─────────────────────────────────────────────────────┐\n");
    printf("│ Metric                      │ Value               │\n");
    printf("├─────────────────────────────────────────────────────┤\n");
    printf("│ Tasks Completed             │ %3.0f%%              │\n",
           metrics.productivity_score);
    printf("│ Energy Efficiency           │ %3.0f%%              │\n",
           metrics.energy_efficiency * 100.0f);
    printf("│ Goal Progress               │ %3.1f%%              │\n",
           metrics.goal_velocity);
    printf("│ Learning Hours (today)      │ %3.2f h              │\n",
           metrics.learning_rate);
    printf("│ Habit Adherence             │ %3.1f%%              │\n",
           metrics.habit_adherence);
    printf("│ Work-Life Balance (ratio)   │ %3.1f%%              │\n",
           metrics.focus_ratio);
    printf("│ Burnout Risk Index          │ %3.1f / 100         │\n",
           metrics.burnout_index);
    printf("└─────────────────────────────────────────────────────┘\n\n");

    /* ─────────────────────────────────────────────────────────────
     *  STAGE 7: RECOMMENDATIONS
     * ──────────────────────────────────────────────────────────── */

    print_separator("STAGE 7: SYSTEM RECOMMENDATIONS");

    printf("AI-Generated Recommendations for Tomorrow:\n\n");

    if (metrics.productivity_score < 60.0f) {
        printf("⚠️  Low productivity today. Consider:\n");
        printf("   • Fewer, higher-priority tasks tomorrow\n");
        printf("   • Longer focus blocks\n");
        printf("   • More recovery time\n\n");
    }

    if (metrics.burnout_index > 50.0f) {
        printf("⚠️  Burnout risk detected. Recommendations:\n");
        printf("   • Take a recovery day\n");
        printf("   • Prioritize sleep\n");
        printf("   • Reduce workload\n\n");
    }

    if (metrics.energy_efficiency < 0.5f) {
        printf("⚠️  Low energy efficiency. Try:\n");
        printf("   • Match tasks to peak energy times\n");
        printf("   • Take more breaks\n");
        printf("   • Optimize task order\n\n");
    }

    if (metrics.goal_velocity > 70.0f) {
        printf("✅ Excellent goal progress! Keep momentum!\n\n");
    }

    if (metrics.habit_adherence > 80.0f) {
        printf("✅ Strong habit adherence! Your streaks are impressive!\n\n");
    }

    /* ─────────────────────────────────────────────────────────────
     *  STAGE 8: LEARNING OUTCOMES
     * ──────────────────────────────────────────────────────────── */

    print_separator("LEARNING OUTCOMES - KEY CONCEPTS DEMONSTRATED");

    printf("\n1. KERNEL ARCHITECTURE (Phase 1)\n");
    printf("   ✓ CPU analogy: Task scheduler makes dispatch decisions\n");
    printf("   ✓ Memory analogy: Energy tracking like RAM management\n");
    printf("   ✓ State machine: Tasks transition through states\n\n");

    printf("2. TASK MANAGEMENT (Phase 2)\n");
    printf("   ✓ Priority scheduling: Tasks sorted by urgency\n");
    printf("   ✓ Deadline management: Critical tasks prioritized\n");
    printf("   ✓ Smart dispatch: Multiple factors weigh scheduling\n\n");

    printf("3. ENERGY MANAGEMENT (Phase 3)\n");
    printf("   ✓ Multi-type energy: Mental, physical, emotional\n");
    printf("   ✓ Ultradian rhythms: 90-minute cycles tracked\n");
    printf("   ✓ Burnout detection: System monitors fatigue\n\n");

    printf("4. DECISION MAKING (Phase 4)\n");
    printf("   ✓ Weighted scoring: Multiple criteria evaluated\n");
    printf("   ✓ Mode selection: System adapts to conditions\n");
    printf("   ✓ Auto-mode: Kernel governor patterns\n\n");

    printf("5. LEARNING SYSTEM (Phase 5)\n");
    printf("   ✓ Skill tracking: XP-based progression\n");
    printf("   ✓ Level system: Mastery progression\n");
    printf("   ✓ Goal tracking: Progress toward objectives\n\n");

    printf("6. SYSTEM INTEGRATION (Phase 6)\n");
    printf("   ✓ Dashboard: Real-time monitoring like htop\n");
    printf("   ✓ Health checks: Watchdog daemon patterns\n");
    printf("   ✓ Optimization: Kernel tuning parameters\n");
    printf("   ✓ Logging: Ring buffer kernel logs\n");
    printf("   ✓ Metrics: Performance counters\n\n");

    /* ─────────────────────────────────────────────────────────────
     *  STAGE 9: SHUTDOWN
     * ──────────────────────────────────────────────────────────── */

    print_separator("STAGE 9: GRACEFUL SHUTDOWN");

    hos_shutdown(&os);

    printf("✅ All systems shutdown cleanly\n");
    printf("   System uptime: %.1f hours\n", os.uptime_hours);
    printf("   Days active: %d\n", os.day_number);
    printf("   Total work sessions: %d\n\n", os.task_mgr.count);

    print_separator("PHASE 6 COMPLETE - HUMAN OS READY FOR PRODUCTION");

    return 0;
}
