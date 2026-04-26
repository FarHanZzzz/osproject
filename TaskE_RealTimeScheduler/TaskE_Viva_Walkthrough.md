# Task E: Real-Time Scheduler (RM vs EDF) — The Viva Defense Walkthrough

Imagine you're walking into the viva. The examiner asks: *"How did you approach Task E, and why did you write the code this way?"*

This document is your exact thought process, from reading the question to writing the final line of code.

---

## Step 1: Reading the Question & Finding the Prerequisites
The prompt asked to:
1. Simulate two **real-time scheduling algorithms**: Rate Monotonic (RM) and Earliest Deadline First (EDF).
2. Use **periodic tasks** with known execution times and periods.
3. Show the **execution timeline** tick-by-tick.
4. Check if any **deadlines are missed**.

**My immediate thought process:**
"This is the hardest scheduling task. Real-time systems must finish tasks BEFORE their deadlines. Missing a deadline = system failure (think: autopilot, ABS brakes). I need to simulate two different strategies for deciding who runs and compare them."

**Prerequisites I had to learn before coding:**
1.  **Periodic Tasks:** A task that repeats every T ticks. At the start of each period, a new "instance" arrives and must finish before the period ends.
2.  **Rate Monotonic (RM):** FIXED priority. The task with the SHORTEST period always wins. Priority never changes. Simple to implement.
3.  **Earliest Deadline First (EDF):** DYNAMIC priority. Whoever's absolute deadline is soonest gets the CPU. Priority changes every tick.
4.  **Hyperperiod:** The LCM of all periods. After this many ticks, the entire pattern repeats. We only simulate one hyperperiod.
5.  **CPU Utilization:** `U = Σ(Ci/Ti)`. If U ≤ 1.0, EDF can always schedule it. RM has a stricter bound.

---

## Step 2: The Mental Blueprint (ELI5)
*   **The Scenario:** I'm an air traffic controller (the CPU). Three planes (tasks) keep circling back regularly:
    *   Plane 1 circles every 5 minutes and needs 2 minutes of my attention.
    *   Plane 2 circles every 10 minutes and needs 4 minutes.
    *   Plane 3 circles every 20 minutes and needs 1 minute.
*   **RM Strategy:** "I always help the plane that comes back MOST OFTEN first" (shortest period = highest priority). Plane 1 always gets priority, then Plane 2, then Plane 3.
*   **EDF Strategy:** "I always help the plane whose landing deadline is SOONEST." This changes dynamically — sometimes Plane 2 is more urgent than Plane 1!

---

## Step 3: Writing the Code (Line-by-Line Rationale)

### 1. The Task Struct
```cpp
struct Task {
    int id;         // Task name (1, 2, 3)
    int exec_time;  // Ci: how many ticks needed per period
    int period;     // Ti: how often it repeats (also its deadline)
    int time_left;  // Remaining execution in current period
};
```

### 2. The Task Set
```cpp
vector<Task> tasks = {
    {1, 2, 5,  0},   // Task 1: needs 2 ticks every 5 ticks
    {2, 4, 10, 0},   // Task 2: needs 4 ticks every 10 ticks
    {3, 1, 20, 0},   // Task 3: needs 1 tick every 20 ticks
};
```
*Utilization:* `2/5 + 4/10 + 1/20 = 0.4 + 0.4 + 0.05 = 0.85` (85% CPU busy).
*Hyperperiod:* `LCM(5, 10, 20) = 20`. We simulate 20 ticks.

### 3. The RM Simulation
**The core logic — task activation at period start:**
```cpp
for (int tick = 0; tick < hyperperiod; tick++) {
    if (tick % 5 == 0) {
        if (tasks[0].time_left > 0) rm_misses++;  // Previous instance didn't finish!
        tasks[0].time_left = tasks[0].exec_time;   // New instance arrives
    }
    // Same for tick % 10 (Task 2) and tick % 20 (Task 3)
```
*Viva Key:* At the start of each period, we check if the PREVIOUS instance finished. If `time_left > 0`, the old instance missed its deadline. Then we reset `time_left` for the new instance.

**The RM priority — just an if-else chain:**
```cpp
    if (tasks[0].time_left > 0)      { tasks[0].time_left--; /* Run Task 1 */ }
    else if (tasks[1].time_left > 0) { tasks[1].time_left--; /* Run Task 2 */ }
    else if (tasks[2].time_left > 0) { tasks[2].time_left--; /* Run Task 3 */ }
    else                              { /* IDLE */ }
}
```
*Why so simple?* Because RM priority is FIXED. Task 1 (period 5) always beats Task 2 (period 10) always beats Task 3 (period 20). We just check in order.

### 4. The EDF Simulation
**Same activation logic.** The difference is in priority selection:
```cpp
    int deadline1 = ((tick / 5) + 1) * 5;    // Task 1's current deadline
    int deadline2 = ((tick / 10) + 1) * 10;  // Task 2's current deadline
    int deadline3 = ((tick / 20) + 1) * 20;  // Task 3's current deadline

    int best = -1, earliest = 999999;
    if (tasks[0].time_left > 0 && deadline1 < earliest) { best = 0; earliest = deadline1; }
    if (tasks[1].time_left > 0 && deadline2 < earliest) { best = 1; earliest = deadline2; }
    if (tasks[2].time_left > 0 && deadline3 < earliest) { best = 2; earliest = deadline3; }
```
*The deadline formula:* `((tick / period) + 1) * period`
*   At tick 3, Task 1's period is [0,5), so deadline = `((3/5)+1)*5 = (0+1)*5 = 5`.
*   At tick 7, Task 1's period is [5,10), so deadline = `((7/5)+1)*5 = (1+1)*5 = 10`.

This dynamically computes who has the CLOSEST deadline right now.

---

## Step 4: Problems Faced & "What Ifs" (Viva Goldmine)

### 1. "What's the Difference Between RM and EDF?"

| Feature | Rate Monotonic | Earliest Deadline First |
|---|---|---|
| Priority | FIXED (shorter period = higher) | DYNAMIC (closest deadline = higher) |
| Optimality | Optimal among FIXED-priority | Optimal among ALL algorithms |
| Utilization bound | ~69% (for many tasks) | 100% |
| Complexity | Very simple (if-else chain) | Requires deadline calculation |
| Predictability | Very predictable | Less predictable |

### 2. "Why Does EDF Work Better?"
**Answer:** "RM has a utilization bound of about 69% for many tasks (the Liu & Layland bound). Our task set uses 85% CPU — it EXCEEDS this bound. RM might still work (and it does here), but it's not guaranteed. EDF works for ANY utilization ≤ 100%. EDF is 'optimal' — if ANY algorithm can schedule a task set, EDF can too."

### 3. "What Is the Hyperperiod and Why Is It Important?"
**Answer:** "The hyperperiod is the LCM of all task periods: LCM(5,10,20) = 20. After 20 ticks, every task has completed exactly the same number of instances, and the pattern repeats. So simulating one hyperperiod is sufficient to prove the schedule works forever."

### 4. "What Happens When a Deadline Is Missed?"
**Answer:** "In our simulation, we detect it: when a new period starts but `time_left > 0` from the previous period, the old instance didn't finish in time. In a REAL real-time system (like an airplane), a missed deadline could mean a catastrophic failure. That's why real-time scheduling is so critical."

### 5. "Why Not Use Actual Threads/Timers?"
**Answer:** "This is a simulation of the CPU scheduler's DECISION-MAKING logic. The real scheduler runs inside the OS kernel as a single-threaded algorithm that decides 'who gets the CPU next.' We're modeling that decision process, not building an actual OS."
