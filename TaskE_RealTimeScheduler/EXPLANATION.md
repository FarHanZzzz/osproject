# ⏰ Task E — Real-Time Scheduler (ELI5 Edition)

## The Story

Imagine you're a **juggler** 🤹 keeping 3 balls in the air:
- 🔴 **Ball 1**: Must be caught and thrown every **5 seconds** (needs 2 sec to handle)
- 🟡 **Ball 2**: Must be caught every **10 seconds** (needs 4 sec)
- 🟢 **Ball 3**: Must be caught every **20 seconds** (needs 1 sec)

If you drop a ball (miss a deadline), the audience boos. The question is: **can you keep all 3 balls in the air without dropping any?**

We test two juggling strategies:
- **RM (Rate Monotonic)**: "Always catch the ball that comes back fastest" (Ball 1 > Ball 2 > Ball 3)
- **EDF (Earliest Deadline First)**: "Always catch whichever ball is about to hit the ground soonest"

---

## 🔧 How to Run It

```bash
g++ -o rt_scheduler rt_scheduler.cpp
./rt_scheduler
```

---

## 📖 Code Walkthrough — The Fun Version

### Part 1: The Balls (Task Struct)

```cpp
struct Task {
    int    id;                  // Ball 1, 2, or 3
    double exec_time;           // How long to handle it (Ci)
    double period;              // How often it comes back (Ti)
    double deadline;            // Must finish before this (= period)
    double utilization;         // Ci/Ti = how much of your time it uses

    double remaining;           // Time left handling current catch
    double absolute_deadline;   // "This ball hits the ground at tick 15"
    bool   ready;               // "Is this ball in the air right now?"
    int    static_priority;     // For RM: fastest ball = highest priority

    int completed;              // Times successfully caught
    int missed;                 // Times DROPPED 😱
};
```

**The test balls (from the assignment PDF):**

| Ball | Handle Time (Ci) | Comes Every (Ti) | Uses % of you (Ci/Ti) |
|------|:-:|:-:|:-:|
| 🔴 Ball 1 | 2 | 5 | 40% |
| 🟡 Ball 2 | 4 | 10 | 40% |
| 🟢 Ball 3 | 1 | 20 | 5% |
| **Total** | | | **85%** |

You need 85% of your time just to handle all 3 balls. That leaves only 15% idle time. Tight!

---

### Part 2: Can You Even Do This? (Schedulability Check)

```cpp
// RM bound: n * (2^(1/n) - 1)
double rm_limit = 3 * (pow(2.0, 1.0/3.0) - 1.0);    // ≈ 0.779 = 77.9%

// EDF bound: always 1.0 = 100%
double edf_limit = 1.0;
```

🧮 **Before juggling, we do MATH to see if it's even possible:**

**RM check:** "Can RM handle 85% utilization?" The RM bound for 3 balls ≈ 77.9%. Since 85% > 77.9%, the math says **"NOT GUARANTEED"**. But here's the twist — this is a conservative test. RM might still work, just no guarantee. Think of it like a speed limit: the sign says 80 km/h, but some cars can safely go 85.

**EDF check:** "Can EDF handle 85%?" EDF's bound is 100%. Since 85% < 100%, **EDF is guaranteed to work.** EDF is just better at juggling! 🏆

---

### Part 3: What's a Hyperperiod?

```cpp
int compute_hyperperiod(vector<Task> &tasks) {
    // LCM of all periods
    // LCM(5, 10, 20) = 20
}
```

🔄 **The hyperperiod = when everything lines up again.** Ball 1 comes every 5, Ball 2 every 10, Ball 3 every 20. After 20 ticks, ALL three balls arrive at the same time again — exactly like tick 0. So the pattern repeats every 20 ticks. We only need to simulate 20 ticks to see the complete picture!

**LCM (Least Common Multiple):** The smallest number divisible by all periods. LCM(5, 10, 20) = 20.

---

### Part 4: Balls Arriving (Activation)

```cpp
void activate_tasks(vector<Task> &tasks, int tick) {
    for (auto &t : tasks) {
        if (tick % (int)t.period == 0) {       // "Is it time for this ball?"
            t.ready = true;                     // "Ball is in the air!"
            t.remaining = t.exec_time;          // Reset handling time
            t.absolute_deadline = tick + t.period;  // "Must catch by tick X"
        }
    }
}
```

🎯 **How `tick % period == 0` works:**
- Ball 1 (period=5): activates at tick 0, 5, 10, 15, 20 → `tick % 5 == 0`
- Ball 2 (period=10): activates at tick 0, 10, 20 → `tick % 10 == 0`
- Ball 3 (period=20): activates at tick 0, 20 → `tick % 20 == 0`

At tick 0, ALL balls arrive. At tick 5, only Ball 1 comes back. At tick 10, Balls 1 and 2.

---

### Part 5: The Two Strategies

**RM — "Always pick the fastest ball"** 🏃
```cpp
Task* pick_rm(vector<Task> &tasks) {
    // Pick the ready task with the LOWEST static_priority number
    // Lower number = shorter period = higher priority
    if (t.static_priority < best->static_priority)
        best = &t;
}
```
Ball 1 (period 5) always beats Ball 2 (period 10) which always beats Ball 3 (period 20). This NEVER changes. Priorities are set once at the start.

**EDF — "Always pick whichever ball is about to hit the ground"** 🎯
```cpp
Task* pick_edf(vector<Task> &tasks) {
    // Pick the ready task with the SMALLEST absolute deadline
    if (t.absolute_deadline < best->absolute_deadline)
        best = &t;
}
```
Priorities change every tick! At tick 3, Ball 1's deadline is tick 5 (close!) while Ball 2's deadline is tick 10 (far away). Ball 1 wins. But at tick 8, Ball 2's deadline is tick 10 (close!) while Ball 1's next deadline is tick 10 too. It's a tie! EDF adapts to the situation.

**Why EDF is more powerful:** RM is like a general who always sends the same soldier first, no matter what. EDF is like a general who looks at the battlefield and picks the best soldier for RIGHT NOW. Adaptability wins. 🏆

---

### Part 6: The Simulation Loop

```cpp
for (int tick = 0; tick < hyperperiod; tick++) {
    activate_tasks(tasks, tick);         // Any balls arriving?

    // Check for drops (deadline misses)
    for (auto &t : tasks) {
        if (t.ready && t.remaining > 0 && tick >= t.absolute_deadline) {
            // DROPPED! 😱
            t.missed++;
        }
    }

    // Pick a ball (RM or EDF)
    Task *running = pick_rm(tasks);  // or pick_edf(tasks)

    // Handle it for 1 tick
    if (running) {
        running->remaining -= 1.0;
        if (running->remaining <= 0)
            running->completed++;    // Caught successfully! 🎉
    }
}
```

Each loop = 1 tick. We check what arrived, check for drops, pick the best ball, and handle it for 1 tick.

---

### Part 7: The Results

```
RM:  0 misses → All deadlines met! ✅ (lucky — math said it might fail!)
EDF: 0 misses → All deadlines met! ✅ (guaranteed by math)
```

**The twist:** RM works here even though the math said "not guaranteed." The bound (77.9%) is a **conservative lower limit** — like saying "this bridge can hold 10 tons" when it actually holds 12. The math gives a worst-case guarantee, but specific task sets can do better.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| Real-Time | Tasks have DEADLINES. Miss one = failure. |
| RM | "Fastest ball always has priority." Fixed. Simple. |
| EDF | "Closest deadline always has priority." Dynamic. Powerful. |
| Hyperperiod | LCM of all periods. Schedule repeats after this many ticks. |
| Utilization | How much of the CPU each task eats. Total must be ≤ 100%. |
| Schedulability | Math test: "Can we GUARANTEE all deadlines are met?" |
| Deadline Miss | Ball dropped. Task didn't finish in time. System failure. |
| Preemption | Stop working on one ball to catch a more urgent one. |
| Periodic Task | A task that repeats at regular intervals forever. |
