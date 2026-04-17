# Task E — Real-Time Scheduler
## How the Code Works — Complete Explanation

> **Source:** `rt_scheduler.cpp` | **Language:** C++ | **OS Concepts:** Real-time scheduling, RM, EDF, Schedulability, Hyperperiod

---

## What This Program Does

This program simulates Real-Time CPU scheduling for periodic tasks (tasks that must run repeatedly, like reading a sensor every 10ms). It features two scheduling algorithms:
1. **Rate Monotonic (RM):** Static priority, where shorter periods = higher priority.
2. **Earliest Deadline First (EDF):** Dynamic priority, where closest absolute deadline = higher priority.

It also mathematically proves whether a task set is schedulable *before* running the simulation, and accurately simulates the tasks up to their hyperperiod to verify for missed deadlines.

```
Task Set: τ1(C=2,T=5), τ2(C=4,T=10), τ3(C=1,T=20)
RM Schedulable: NO  (Total U=0.85 > Bound=0.7798)
EDF Schedulable: YES
```

---

## How It Works — Step by Step

### 1. Mathematical Pre-Check (Lines 110–152)

```cpp
double U = 0.0;
for (auto &t : tasks) U += t.utilization;

double rm_bound  = rm_utilisation_bound(n); // n*(2^(1/n)-1)
double edf_bound = 1.0;
```

Before simulating, we calculate CPU Utilization ($U = \sum \frac{C_i}{T_i}$). 
- For EDF, the CPU just needs to be $\le 100\%$ ($1.0$). 
- For RM, Liu and Layland proved a specific mathematical bound. For 3 tasks, this bound is $\approx 0.7798$. 

Our test case has a utilization of $0.85$, so we mathematically warn the user that RM *might* fail.

### 2. The Hyperperiod Calculation (Lines 90–96)

```cpp
static int compute_hyper_period(const std::vector<Task> &tasks) {
    long long hp = 1;
    for (auto &t : tasks) hp = lcm_ll(hp, (long long)t.period);
    return (int)hp;
}
```

We don't need to simulate to infinity. Because periodic tasks follow a repeating pattern, the entire system state resets perfectly after the Least Common Multiple (LCM) of all periods. For $T = {5, 10, 20}$, the LCM is $20$. We only need to simulate 20 ticks.

### 3. Task Activation (Lines 158–167)

```cpp
if (tick % (int)t.period == 0) {
    t.ready             = true;
    t.remaining_time    = t.exec_time;
    t.absolute_deadline = tick + t.period;
}
```

At every tick, we check if `tick % period == 0`. If so, a new instance of the task has "arrived". We reset its remaining execution time, mark it ready, and set its absolute deadline (e.g., if it arrives at tick 10 and has a period of 5, its absolute deadline is 15).

### 4. Selection Algorithms (Lines 172–195)

```cpp
// For RM (Static Priority)
if (!best || t.static_priority < best->static_priority)

// For EDF (Dynamic Priority)
if (!best || t.absolute_deadline < best->absolute_deadline)
```

This highlights the fundamental difference between the two algorithms. 
- **RM** only looks at a fixed number (`static_priority`) assigned at startup.
- **EDF** looks at the `absolute_deadline` variable, which changes dynamically every period. 

### 5. Execution and Deadline Misses (Lines 217–252)

```cpp
if ((double)tick >= t.absolute_deadline) {
    // DEADLINE MISSED
    t.deadline_misses++;
    t.ready = false; 
    t.remaining_time = 0;
}
```

Before picking a task to run, the simulation checks if any ready task has breached its deadline. If it has, we log the failure, abort the current instance, and reset it. 

---

## Key OS Concepts

| Concept | Where in Code | What It Does |
|---------|---------------|--------------|
| **Utilization Bound** | `rm_utilisation_bound` | The theoretical threshold of CPU load an algorithm can guarantee scheduling for. |
| **Hyperperiod** | `compute_hyper_period` | The LCM of all periods; the exact length needed to prove schedulability via simulation. |
| **Static Priority** | `select_rm` | Priorities set once (based on period length) and never changed. |
| **Dynamic Priority** | `select_edf` | Priorities calculated on the fly based on which deadline is approaching fastest. |

---

## Test Results (Verified)

| Metric Test (Manual Math vs Output) | Expected | Actual Output | Status |
|-------------------------------------|----------|---------------|--------|
| Total Utilization | $0.8500$ | $0.8500$ | ✅ PASS |
| RM Bound (n=3) | $0.7798$ | $0.7798$ | ✅ PASS |
| LCM (5, 10, 20) | $20$ | $20$ | ✅ PASS |
| Deadlines missed (EDF) | 0 | 0 | ✅ PASS |

---

## Viva Questions & Answers

**Q: The output shows RM failed the schedulability check (0.85 > 0.779), but in the simulation, RM met all deadlines. How is this possible?**
A: The Liu & Layland bound for RM ($n(2^{1/n}-1)$) is a **sufficient, but not necessary** condition. If the utilization is below the bound, it is mathematically guaranteed to work. If it is above the bound, it is not guaranteed, but it *might* still work depending on the exact alignment of task periods (especially if the periods are harmonious/multiples of each other, as 5, 10, and 20 are).

**Q: Why does EDF always have a utilization bound of 1.0 (100%), whereas RM is much lower?**
A: EDF uses dynamic priority. It can adapt on the fly and prioritize whichever task is in the most danger of failing its deadline. RM uses static priority. Because RM forces tasks with longer periods to always wait for tasks with shorter periods—even if the longer-period task is about to miss its deadline—the CPU cannot be fully utilized under worst-case phasing conditions.

**Q: What is a Hyperperiod and why do we simulate exactly up to that point?**
A: The hyperperiod is the Least Common Multiple (LCM) of all task periods. It represents the point where all tasks restart their cycles perfectly synchronized, exactly like they did at tick 0. If a schedule succeeds up to the hyperperiod without missing a deadline, it will succeed infinitely. Simulating further is mathematically redundant.

**Q: In the code, what is the difference between a task's relative deadline and absolute deadline?**
A: The relative deadline ($D_i$) is fixed. E.g., "This task must finish within 10 ticks of arriving." The absolute deadline is dynamic. If that same task arrives at tick 30, its absolute deadline is tick 40. EDF schedules using the absolute deadline.
