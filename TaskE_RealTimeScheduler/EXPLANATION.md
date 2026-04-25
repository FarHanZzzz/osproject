# ⏱️ Task E — Real-Time Scheduler (ELI5 Edition)

> **Quick Overview:** This program simulates two real-time scheduling algorithms — Rate Monotonic (RM) and Earliest Deadline First (EDF) — on 3 periodic tasks. It runs a 20-tick simulation for each and checks if any deadlines are missed. **Key OS concept: Real-Time Scheduling with RM and EDF.**

---

Imagine you're an **air traffic controller** managing three aircraft:
- ✈️ **Plane 1:** Must check in every 5 minutes, needs 2 minutes of your attention each time
- 🛩️ **Plane 2:** Must check in every 10 minutes, needs 4 minutes of your attention each time
- 🚁 **Helicopter:** Must check in every 20 minutes, needs 1 minute of your attention each time

If you miss even ONE check-in, a plane could crash. These are **hard deadlines** — there's no "I'll get to it later." You need a **strategy** to decide who to talk to and when.

That's what this program does — it tests two different strategies:
- **Rate Monotonic (RM):** Always talk to whoever checks in most frequently first
- **Earliest Deadline First (EDF):** Always talk to whoever's deadline is closest right now

---

## 🎯 The Big Picture

```
We have 3 periodic tasks:
  Task 1: Exec=2, Period=5   (runs every 5 ticks, needs 2 ticks)
  Task 2: Exec=4, Period=10  (runs every 10 ticks, needs 4 ticks)  
  Task 3: Exec=1, Period=20  (runs every 20 ticks, needs 1 tick)

Hyperperiod = LCM(5, 10, 20) = 20 ticks

We simulate 20 ticks using TWO strategies:
  │
  ├── Strategy 1: Rate Monotonic
  │     Fixed priority: Task1 > Task2 > Task3 (shorter period = higher priority)
  │
  └── Strategy 2: Earliest Deadline First
        Dynamic priority: whoever's deadline is soonest gets the CPU
```

---

## 🔧 How to Run It

```bash
g++ -o rt_scheduler rt_scheduler.cpp
./rt_scheduler
```

---

## 📖 Code Walkthrough — The Fun Version

### Part 1: What Makes Real-Time Different?

🚑 **Normal scheduling:** "I'll get to it when I can." It's fine if your Spotify playlist loads in 200ms instead of 100ms. Nobody dies.

⚡ **Real-time scheduling:** "It MUST finish by this exact deadline." An airbag must deploy within 50ms of impact. A robot arm must move within 10ms of the sensor reading. Late = failure.

**Hard vs. Soft Real-Time:**
- **Hard:** Missing a deadline = catastrophic failure (airbag, pacemaker, nuclear reactor)
- **Soft:** Missing a deadline = degraded quality (video dropping frames, audio stuttering)

---

### Part 2: Periodic Tasks

```c
vector<Task> tasks = {
    {1, 2, 5,  0},   // Task 1: needs 2 ticks, repeats every 5 ticks
    {2, 4, 10, 0},   // Task 2: needs 4 ticks, repeats every 10 ticks
    {3, 1, 20, 0},   // Task 3: needs 1 tick, repeats every 20 ticks
};
```

🔄 **A periodic task is like a recurring alarm.** Task 1 goes off at tick 0, 5, 10, 15. Every time it goes off, it demands 2 ticks of CPU time and MUST finish before the next alarm (deadline = end of current period).

**Timeline for Task 1:**
```
Period 1: [0-4]   → Must finish 2 ticks of work by tick 5
Period 2: [5-9]   → Must finish 2 ticks of work by tick 10
Period 3: [10-14] → Must finish 2 ticks of work by tick 15
Period 4: [15-19] → Must finish 2 ticks of work by tick 20
```

---

### Part 3: The Hyperperiod

```c
int hyperperiod = 20;  // LCM(5, 10, 20) = 20
```

🔁 **The Hyperperiod is the "Groundhog Day" of scheduling.** After 20 ticks, ALL three tasks have completed exactly the same number of periods (Task 1: 4 periods, Task 2: 2 periods, Task 3: 1 period), and the entire pattern repeats.

**Why is this useful?** If we can prove that no deadlines are missed in 20 ticks, then no deadlines will EVER be missed. We don't need to simulate for 1000 ticks — just 20 is enough to prove correctness!

**How to calculate it:**
- LCM(5, 10) = 10
- LCM(10, 20) = 20
- So the Hyperperiod = 20

---

### Part 4: Utilization — Can We Even Do This?

```
Utilization = C1/T1 + C2/T2 + C3/T3
            = 2/5 + 4/10 + 1/20
            = 0.4 + 0.4 + 0.05
            = 0.85 (85% CPU usage)
```

📊 **Think of utilization as "how full is the CPU's schedule?"**
- 0.85 = the CPU is busy 85% of the time, free 15%
- If utilization > 1.0 = IMPOSSIBLE to schedule. There literally aren't enough ticks in a period.

**RM Bound:** For 3 tasks, RM is guaranteed to work if U ≤ 3 × (2^(1/3) - 1) ≈ 0.78. Our U = 0.85 > 0.78, so RM is NOT guaranteed... but it might still work for this specific task set!

**EDF Bound:** EDF is guaranteed to work if U ≤ 1.0. Our U = 0.85 ≤ 1.0, so EDF is GUARANTEED to work. ✅

---

### Part 5: Rate Monotonic (RM) — "Shortest Cycle Wins"

```c
// RM: fixed priority chain — Task 1 always beats Task 2 always beats Task 3
if (tasks[0].time_left > 0) {         // Task 1 (period 5) — HIGHEST priority
    tasks[0].time_left--;
}
else if (tasks[1].time_left > 0) {    // Task 2 (period 10) — MEDIUM priority
    tasks[1].time_left--;
}
else if (tasks[2].time_left > 0) {    // Task 3 (period 20) — LOWEST priority
    tasks[2].time_left--;
}
```

📏 **RM Rule: The task with the SHORTEST period gets the HIGHEST fixed priority.** Period 5 < Period 10 < Period 20, so Task 1 > Task 2 > Task 3. This priority NEVER changes.

**Why if-else chain?** Because RM priorities are static! Task 1 is always checked first. If Task 1 needs the CPU, it gets it — no questions asked. Task 3 only runs when both Task 1 AND Task 2 are idle.

**RM at tick 0-4:**
```
Tick 0: Task 1 wakes up (needs 2), Task 2 wakes up (needs 4), Task 3 wakes up (needs 1)
Tick 0: Task 1 runs (highest priority). Task 1 left: 1
Tick 1: Task 1 runs. Task 1 left: 0 ✅ done!
Tick 2: Task 2 runs (next highest). Task 2 left: 3
Tick 3: Task 2 runs. Task 2 left: 2
Tick 4: Task 2 runs. Task 2 left: 1
```

---

### Part 6: Earliest Deadline First (EDF) — "Urgent Deadline Wins"

```c
// Calculate absolute deadlines for each task at this tick
int deadline1 = ((tick / 5) + 1) * 5;     // End of Task 1's current period
int deadline2 = ((tick / 10) + 1) * 10;   // End of Task 2's current period
int deadline3 = ((tick / 20) + 1) * 20;   // End of Task 3's current period

// Pick whoever has the EARLIEST (smallest) deadline
int best = -1;
int earliest = 999999;

if (tasks[0].time_left > 0 && deadline1 < earliest) {
    best = 0; earliest = deadline1;
}
if (tasks[1].time_left > 0 && deadline2 < earliest) {
    best = 1; earliest = deadline2;
}
if (tasks[2].time_left > 0 && deadline3 < earliest) {
    best = 2; earliest = deadline3;
}
```

🏃 **EDF Rule: Whoever's deadline is the CLOSEST gets the CPU.** Unlike RM, priorities in EDF change EVERY tick.

**The deadline formula explained:**
```
At tick 7, Task 1's period is 5:
  Which period is tick 7 in?  7/5 = 1 (integer division)
  When does this period end?  (1 + 1) * 5 = 10
  So Task 1's deadline at tick 7 = 10
  
At tick 7, Task 2's period is 10:
  Which period is tick 7 in?  7/10 = 0
  When does this period end?  (0 + 1) * 10 = 10
  So Task 2's deadline at tick 7 = 10
```

When deadlines are tied, the first task checked wins (Task 1 in our code).

**Why EDF is "optimal":** EDF can schedule ANY task set where U ≤ 1.0. No other algorithm can do better! If EDF fails, NO algorithm would succeed. That's why it's called "optimal."

---

### Part 7: Activating Tasks

```c
if (tick % 5 == 0) {
    if (tasks[0].time_left > 0) {
        cout << "DEADLINE MISS: Task 1" << endl;
    }
    tasks[0].time_left = tasks[0].exec_time;  // Reset to 2
}
```

⏰ **Every time a task's period starts (tick 0, 5, 10, 15 for Task 1), we "wake it up"** by resetting its `time_left` back to its full execution time.

**But first, we check:** If `time_left > 0` when a new period starts, it means the task DIDN'T finish in its previous period. That's a **DEADLINE MISS** — the worst thing that can happen in real-time scheduling!

---

### Part 8: RM vs EDF — Who Wins?

For THIS specific task set (U = 0.85), BOTH RM and EDF produce 0 deadline misses. But this won't always be the case!

**When RM fails but EDF succeeds:** If we increase Task 2's execution time to 5 (making U = 0.95), RM would miss deadlines but EDF would still handle it perfectly, because 0.95 ≤ 1.0.

**When BOTH fail:** If total utilization > 1.0, even EDF can't save you. There simply isn't enough CPU time in the universe.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| Real-Time System | A system where being late = failure. Deadlines are absolute, not suggestions. |
| Periodic Task | A recurring job. Like a heartbeat — it happens every N seconds and needs C seconds of attention each time. |
| Period (Ti) | How often the task repeats. Task with period 5 = runs at tick 0, 5, 10, 15... |
| Execution Time (Ci) | How much CPU time the task needs each period. |
| Deadline | The latest time the task must finish. In our case, deadline = end of current period. |
| Hyperperiod | LCM of all periods. After this many ticks, the entire schedule repeats perfectly. |
| Utilization (U) | Sum of Ci/Ti for all tasks. Tells you how "full" the CPU schedule is. U > 1.0 = impossible. |
| Rate Monotonic (RM) | Shorter period = higher priority. Priority is FIXED forever. Simple but not optimal. |
| EDF | Earliest absolute deadline = highest priority. Priority CHANGES every tick. Optimal! |
| RM Bound | n × (2^(1/n) - 1). If U ≤ this bound, RM is guaranteed to work. If U > bound, RM MIGHT still work. |
| EDF Bound | 1.0. If U ≤ 1.0, EDF is GUARANTEED to work. Period. |
| Deadline Miss | Task didn't finish before its deadline. In hard real-time = catastrophic failure. |
| Preemption | Kicking a running task off the CPU because a higher-priority one just woke up. Both RM and EDF do this. |
| Optimal Scheduler | A scheduler that can handle ANY task set that is theoretically schedulable. EDF is optimal. RM is not. |
