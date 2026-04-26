# ⏱️ Task E — Real-Time Scheduler (ELI5 Edition)

> **Quick Overview:** This program simulates two real-time scheduling algorithms — Rate Monotonic (RM) and Earliest Deadline First (EDF) — on 3 periodic tasks. It performs **schedulability analysis** BEFORE simulation, computes the hyperperiod via LCM, runs a 20-tick simulation for each algorithm, and checks if any deadlines are missed. **Key OS concept: Real-Time Scheduling with RM, EDF, and Schedulability Testing.**

---

## 🎯 The Big Picture

```
Step 1: Define periodic tasks (exec time, period)
Step 2: Compute Hyperperiod = LCM(5, 10, 20) = 20
Step 3: SCHEDULABILITY ANALYSIS (before simulating!)
  ├── RM Bound: n*(2^(1/n)-1) ≈ 0.779 → U=0.85 > 0.779 → INCONCLUSIVE
  └── EDF Bound: 1.0            → U=0.85 ≤ 1.0   → GUARANTEED ✅
Step 4: RM Simulation (20 ticks, fixed priorities)
Step 5: EDF Simulation (20 ticks, dynamic priorities)
```

---

## 🔧 How to Run It

```bash
g++ -o rt_scheduler rt_scheduler.cpp
./rt_scheduler
```

---

## 📖 Code Walkthrough

### Part 1: Schedulability Testing (Required by Assignment)

```cpp
// RM Bound: n * (2^(1/n) - 1)
double rm_bound = n * (pow(2.0, 1.0 / n) - 1);
// For n=3: 3 * (2^(1/3) - 1) ≈ 0.779

if (total_util <= rm_bound)
    cout << "GUARANTEED schedulable by RM";
else if (total_util <= 1.0)
    cout << "INCONCLUSIVE (must simulate to verify)";
else
    cout << "NOT schedulable by ANY algorithm";
```

📊 **We check schedulability BEFORE simulating:**
- **RM Bound:** U ≤ n×(2^(1/n)-1). For n=3, bound ≈ 0.779. Our U=0.85 > 0.779, so RM is **inconclusive** — it might work, might not. We must simulate to find out.
- **EDF Bound:** U ≤ 1.0. Our U=0.85 ≤ 1.0, so EDF is **guaranteed** to work.

---

### Part 2: Hyperperiod via LCM

```cpp
int gcd(int a, int b) { while (b) { int t=b; b=a%b; a=t; } return a; }
int lcm(int a, int b) { return a / gcd(a,b) * b; }

int hyperperiod = tasks[0].period;
for (int i = 1; i < n; i++)
    hyperperiod = lcm(hyperperiod, tasks[i].period);
// LCM(5, 10, 20) = 20
```

🔁 **The hyperperiod is computed dynamically** using GCD/LCM — not hardcoded. After one hyperperiod, the entire schedule repeats. Proving correctness in 20 ticks proves it for all time.

---

### Part 3: Rate Monotonic — Fixed Priority

```cpp
// Tasks sorted by period: Task1(5) > Task2(10) > Task3(20)
// Priority NEVER changes
for (int i = 0; i < n; i++) {
    if (tasks[i].time_left > 0) {
        chosen = i;
        break;  // First ready = highest priority
    }
}
```

📏 RM assigns priorities ONCE: shorter period = higher priority. Task 1 (period 5) always beats Task 2 (period 10) always beats Task 3 (period 20).

---

### Part 4: EDF — Dynamic Priority with Absolute Deadlines

```cpp
// Track absolute deadlines
vector<int> abs_deadline(n, 0);

// At each period start:
abs_deadline[i] = tick + tasks[i].period;

// Pick whoever has the EARLIEST deadline
for (int i = 0; i < n; i++) {
    if (tasks[i].time_left > 0 && abs_deadline[i] < earliest) {
        best = i;
        earliest = abs_deadline[i];
    }
}
```

🏃 EDF uses **absolute deadlines** (arrival + period) that change every period. The task whose deadline is soonest gets the CPU — priorities shift dynamically every tick.

---

### Part 5: Deadline Miss Detection

```cpp
if (tick % tasks[i].period == 0) {
    if (tasks[i].time_left > 0) {
        cout << "DEADLINE MISS: Task " << tasks[i].id;
        misses++;
    }
    tasks[i].time_left = tasks[i].exec_time;  // New instance
}
```

⚠️ At each period boundary, if a task still has remaining work, it **missed its deadline** — a critical failure in real-time systems.

---

### Expected Results (from Assignment)

| Algorithm | U=0.85 | Result |
|-----------|--------|--------|
| **RM** | U > 0.779 (bound) | Inconclusive, but simulation shows **0 misses** ✅ |
| **EDF** | U ≤ 1.0 (bound) | Guaranteed, simulation confirms **0 misses** ✅ |

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| Real-Time System | Being late = failure. Deadlines are absolute. |
| Periodic Task | A recurring job every N ticks needing C ticks of work each time. |
| Hyperperiod | LCM of all periods. After this, the schedule repeats forever. |
| Utilization (U) | Sum of Ci/Ti. How "full" is the CPU? U > 1.0 = impossible. |
| Rate Monotonic | Shorter period = higher fixed priority. Simple but not optimal. |
| EDF | Earliest absolute deadline = highest priority. Dynamic and optimal. |
| RM Bound | n×(2^(1/n)-1). If U ≤ bound, RM guaranteed. If U > bound, inconclusive. |
| EDF Bound | 1.0. If U ≤ 1.0, EDF is guaranteed. Period. |
| Deadline Miss | Task didn't finish before its deadline. Catastrophic in hard real-time. |
| Schedulability Test | Check if scheduling is possible BEFORE running the simulation. |
