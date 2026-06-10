# Phase 6: Quick Start Guide

## 🚀 Get Up and Running in 2 Minutes

### Step 1: Build
```bash
cd phase6
make
```

### Step 2: Run
```bash
make run
```

### Step 3: Explore the Output
The program will display:
- ✅ Kernel boot sequence
- 📋 Task scheduling and execution
- ⚡ Energy management simulation
- 📊 Real-time dashboard
- 📈 Performance metrics
- 💡 AI recommendations

## 🎯 What You'll See

**Real-time Dashboard Example:**
```
✅ OPTIMAL  | Day 1  | Energy: 96.0% | Burnout: 0.0% | Mode: 📚 LEARNING

Energy Bars:
  Mental    [████████████████]  90.0/100
  Physical  [██████████████]    90.0/90
  Emotional [██████████████]    80.0/80
  Overall   [████████████]      96.0%

Tasks, Goals, Skills, Metrics, and Logs...
```

## 📚 Key Components

### 1. Scheduler (`hos_get_next_task`)
- Picks the best task based on:
  - Priority
  - Energy availability
  - Current mode
  - Historical success

### 2. Mode Selection (`hos_auto_mode`)
- **PRODUCTIVE**: Normal work
- **FOCUSED**: Deep work
- **RECOVERY**: Rest
- **EMERGENCY**: Crisis
- **LEARNING**: Study

### 3. Energy System
Tracks three energy types:
- **Mental**: Cognition, focus
- **Physical**: Body, movement
- **Emotional**: Social, feelings

### 4. Dashboard (`hos_dashboard`)
Real-time system monitor showing:
- Status and energy levels
- Task queue
- Goal progress
- Skill levels
- Performance metrics
- Activity log

## 🔧 How to Customize

### Add More Tasks
Edit `main.c`, find "Initialize tasks" section:
```c
hos_add_task(&os, "Task name", "category",
             PRIORITY_HIGH, 120.0f,  // duration minutes
             60.0f,                   // energy required
             95.0f);                  // value score
```

### Change Energy Capacities
In `main.c` boot line:
```c
hos_boot(&os, 100.0f,  // mental
              90.0f,   // physical
              80.0f);  // emotional
```

### Adjust System Modes
Edit `hos_auto_mode()` in `src/human_os.c` to change mode selection logic.

### Modify Optimization Strategies
Edit `hos_optimize()` in `src/human_os.c` to add new strategies.

## 🧪 Test Different Scenarios

### Scenario 1: High Burnout
Modify `main.c` to do more work with less rest:
```c
hos_work(&os, task_id, 240.0f);  // Work 4 hours straight
```

### Scenario 2: Perfect Recovery
Balance work and recovery:
```c
hos_work(&os, task_id, 60.0f);      // 1h work
hos_rest(&os, 15.0f, "break");      // 15m break
```

### Scenario 3: Skill Development
Practice skills to see progression:
```c
hos_practice_skill(&os, skill_id, 2.0f);  // 2 hours practice
```

## 📊 Understanding Metrics

| Metric | Meaning | Ideal Range |
|--------|---------|-------------|
| **Productivity** | Tasks completed | > 80% |
| **Energy Efficiency** | Output per energy | > 0.8 |
| **Goal Progress** | Goal completion | > 50% |
| **Learning Rate** | Hours learned/day | > 1.5 |
| **Habit Adherence** | Habit completion | > 80% |
| **Burnout Index** | Fatigue risk | < 30 |

## 🎓 Learning Path

1. **Understand**: Read comments in `human_os.h`
2. **Explore**: Study `hos_get_next_task()` function
3. **Trace**: Follow the main loop in `main.c`
4. **Modify**: Change parameters and observe effects
5. **Extend**: Add new functions and features

## 💡 Common Questions

**Q: Why is energy decreasing?**
A: Working on tasks consumes energy. Rest and sleep restore it.

**Q: What's "burnout_score"?**
A: Accumulated fatigue. High values trigger RECOVERY mode.

**Q: How does scheduling work?**
A: Multiple factors (priority, deadline, energy, mode) create a score. Highest score wins.

**Q: Can I add new system modes?**
A: Yes! Edit `SystemMode` enum and add logic to `hos_auto_mode()`.

## 🚀 Next Steps

1. **Run the demo**: `make run`
2. **Read the output**: Understand each stage
3. **Study the code**: Focus on `hos_get_next_task()`
4. **Modify parameters**: Try different values
5. **Add features**: Create new optimization strategies

## 📖 Documentation

- **README.md**: Complete system guide
- **human_os.h**: API and data structures
- **human_os.c**: Implementation with detailed comments
- **main.c**: Demonstration and testing
- **Makefile**: Build configuration

## 🆘 Troubleshooting

### Build fails
```bash
make clean  # Remove old builds
make        # Rebuild from scratch
```

### Executable not found
```bash
ls -la build/phase6  # Check if built
make rebuild         # Force rebuild
```

### Compilation warnings
These are normal. The `-Wall -Wextra` flags catch minor issues. Code still works fine.

## 🎉 Success!

If you see the dashboard with:
- ✅ Status indicators
- 📊 Energy bars
- 📋 Task lists
- 📈 Metrics

**Congratulations! Phase 6 is complete!**

---

**Next**: Explore the code, understand the concepts, and build your own integrated systems!
