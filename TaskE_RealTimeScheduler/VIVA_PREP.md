# Task E — Real-Time Scheduler: Viva Preparation

---

## 🧠 What Is The Problem?

The assignment asks: *"Simulate two real-time scheduling algorithms — Rate Monotonic (RM) and Earliest Deadline First (EDF) — on a set of periodic tasks. Before running, mathematically check if the task set is even schedulable."*

**Real-time systems are different from normal systems:**
- A normal system just tries to be fast on average
- A real-time system has DEADLINES — being late is a failure, even if the task eventually runs
- Examples: medical devices (insulin pump must inject exactly on time), flight autopilots, anti-lock brakes

This tests your understanding of:
- What periodic tasks are and how they behave
- Rate Monotonic: static priorities based on period
- Earliest Deadline First: dynamic priorities based on deadline
- Schedulability testing: math before simulation

---

## 💬 How To Explain The Problem (Your Opening Line)

> *"The problem is to simulate a real-time CPU scheduler that handles periodic tasks — tasks that repeat at fixed intervals and must finish by a deadline. I implemented two algorithms: Rate Monotonic, where tasks with shorter periods always have higher fixed priority, and Earliest Deadline First, where priority changes dynamically every tick based on which task's deadline is soonest. Before either simulation runs, I mathematically check if the task set is schedulable using the utilization bounds from the PDF."*

---

## ⚙️ How Your Code Solves It — Step By Step

### Step 1 — Define the Task Set (From PDF Table)
```
Task 1: Exec=2, Period=5  → runs 2 ticks out of every 5  → Utilization=0.40
Task 2: Exec=4, Period=10 → runs 4 ticks out of every 10 → Utilization=0.40
Task 3: Exec=1, Period=20 → runs 1 tick  out of every 20 → Utilization=0.05
Total Utilization = 0.85 (85% of CPU is needed)
```

### Step 2 — Compute the Hyperperiod
- Hyperperiod = LCM of all task periods
- LCM(5, 10, 20) = 20
- This is how long one full "repeating pattern" of the schedule lasts
- We simulate exactly one hyperperiod — after that it repeats identically

### Step 3 — Schedulability Analysis (BEFORE simulation)
**RM Bound test:**
- Formula: `n × (2^(1/n) - 1)`
- For n=3: `3 × (2^(1/3) - 1) ≈ 0.779`
- Our U = 0.85 > 0.779 → **INCONCLUSIVE** — must simulate to check
- (If U ≤ bound → guaranteed. If U > 1.0 → impossible. In between → unknown.)

**EDF Bound test:**
- Formula: U ≤ 1.0
- Our U = 0.85 < 1.0 → **GUARANTEED** schedulable by EDF

### Step 4 — Rate Monotonic Simulation
- Priority is FIXED from the start: Task 1 (period=5) always beats Task 2 (period=10), which always beats Task 3 (period=20)
- At each tick: activate tasks whose period starts now, then pick the highest-priority ready task
- "Ready" means the task has remaining execution time in its current period
- Detect deadline miss: when a new period starts and the old period's work isn't done (`time_left > 0`)

### Step 5 — Earliest Deadline First Simulation
- Priority is DYNAMIC: re-evaluated every single tick
- Each task's **absolute deadline** = time it was activated + its period
- At each tick: pick the ready task with the SMALLEST (nearest) absolute deadline
- New task arrivals can preempt the current task if their deadline is earlier

---

## 🔥 Challenges & What You'd Say You Faced

> *"The hardest conceptual part was understanding the difference between relative and absolute deadlines. The 'relative deadline' is just the period length (e.g., 5 ticks). The 'absolute deadline' is the actual clock tick by which the task must finish (e.g., if Task 1 activates at tick 10, its absolute deadline is tick 15). For EDF, I need to track and compare these absolute deadlines constantly."*

> *"For Rate Monotonic, I had to be careful about HOW to detect a deadline miss. A miss happens when a NEW period starts for a task but its `time_left` is still greater than 0 — meaning the previous period's work never finished. I detect this at the moment I reset `time_left` to `exec_time` for the new period."*

> *"The RM schedulability test result of 'INCONCLUSIVE' confused me at first. The math says: if U is above the RM bound (0.779) but below 1.0, RM MIGHT still work — you can't know from the formula alone. You have to actually simulate it. In our case, RM succeeds (0 misses) even though the test was inconclusive — this is one of those cases where RM works even past its guaranteed bound."*

---

## 🤔 Choices You Made & Why

| Choice | Why |
|--------|-----|
| Tasks stored in a vector sorted by period | RM priority = shortest period first; iterating index 0 first = highest priority automatically |
| `time_left` per task (not absolute clock) | Tracks remaining work in the current period; resets each period |
| `abs_deadline[]` array for EDF | Each task's deadline is different per period; must store and update them |
| Hyperperiod as simulation length | After one hyperperiod, the schedule repeats perfectly — no need to run longer |
| Check `time_left > 0` at period start | This is the exact moment to detect a deadline miss |
| `int earliest = 999999` for EDF | Ensures any real deadline will be smaller and replace this sentinel |

---

## 📚 Concepts You Need To Know

### Periodic Task Properties

| Term | Symbol | Meaning | Example |
|------|--------|---------|---------|
| Period | Ti | How often the task repeats | Every 5 ticks |
| Execution Time | Ci | CPU needed per period | 2 ticks of CPU |
| Deadline | Di | Must finish within Di ticks of activation | Di = Ti (= Period) |
| Utilization | Ui = Ci/Ti | Fraction of CPU this task needs | 2/5 = 0.40 = 40% |

### Total CPU Utilization
- U = sum of all Ui
- If U > 1.0: the CPU is overloaded — mathematically impossible to schedule
- If U ≤ 1.0: it MIGHT be possible depending on the algorithm
- Think of it like water in a pipe — if you need more than 100% flow, it's physically impossible

### Rate Monotonic (RM) — Static Priorities
- **Rule**: Shorter period = permanently higher priority
- Task 1 (period=5) ALWAYS beats Task 2 (period=10) — no matter what
- Priority is assigned ONCE and never changes during the simulation
- **RM Schedulability Bound**: U ≤ n × (2^(1/n) - 1)
  - For n=3: ≈ 0.779 (77.9%)
  - As n → ∞, this bound approaches ln(2) ≈ 0.693 (69.3%)
- **If U ≤ bound**: RM is GUARANTEED to meet all deadlines
- **If bound < U ≤ 1.0**: INCONCLUSIVE — need to simulate
- **Simple analogy**: RM is like a strict hierarchy. The boss (Task 1) always goes first.

### Earliest Deadline First (EDF) — Dynamic Priorities
- **Rule**: The task whose DEADLINE is soonest gets the CPU right now
- Priority is re-evaluated at EVERY clock tick — it's not fixed
- A new task arrival can immediately preempt a running task if its deadline is closer
- **EDF Schedulability Bound**: U ≤ 1.0
- **If U ≤ 1.0**: EDF is GUARANTEED to meet all deadlines (it is provably OPTIMAL)
- **Simple analogy**: EDF is like an emergency room triage. Whoever is most urgent right now gets treated first, even if a "higher rank" patient is already being treated.

### Hyperperiod
- Hyperperiod = LCM of all task periods
- After one hyperperiod, every task has completed exactly `hyperperiod / period` instances
- The schedule is periodic — it repeats identically after every hyperperiod
- LCM(5, 10, 20) = 20: Task 1 runs 4 times, Task 2 runs 2 times, Task 3 runs 1 time

### Deadline Miss
- Occurs when a task does not complete its execution before its deadline
- In our simulation: detected when a task's new period begins but `time_left > 0`
- In RM: can happen when utilization exceeds the guarantee bound
- In EDF: CANNOT happen as long as U ≤ 1.0

### GCD and LCM (for Hyperperiod)
- GCD(a, b) = largest number that divides both a and b
- LCM(a, b) = smallest number that is a multiple of both a and b
- LCM(a, b) = (a × b) / GCD(a, b)
- Example: LCM(5, 10) = 50/5 = 10. LCM(10, 20) = 200/10 = 20

---

## ✅ Quick Viva Q&A

**Q: What is a periodic task?**
> A task that repeats at a fixed interval called its period. For example, a sensor that reads data every 5 milliseconds and must process it within 5ms is a periodic task with period=5 and deadline=5.

**Q: What is the difference between RM and EDF?**
> RM uses fixed priorities based on period — shorter period means permanently higher priority, decided once at the start. EDF uses dynamic priorities — at every clock tick, it picks whichever ready task has the nearest (earliest) absolute deadline. EDF is more flexible and provably optimal; RM is simpler to implement in hardware.

**Q: Why is RM's schedulability test result for our task set "INCONCLUSIVE"?**
> Our total utilization is 0.85, which exceeds the RM bound of 0.779. This means RM is not GUARANTEED to work — but it doesn't mean it FAILS either. It's a gray zone. The only way to know is to actually simulate it. In our case, RM happens to succeed with 0 deadline misses because of the specific timing of our task set.

**Q: Why is EDF guaranteed while RM is inconclusive for the same task set?**
> EDF's bound is simply U ≤ 1.0, and our U = 0.85 < 1.0, so EDF is mathematically guaranteed. EDF is the optimal real-time scheduling algorithm — no other algorithm can meet deadlines that EDF fails to meet (when U ≤ 1.0). RM's tighter bound (0.779) exists because fixed priorities are less flexible than dynamic ones.

**Q: What is the hyperperiod and why do we simulate exactly that long?**
> The hyperperiod is the LCM of all task periods — in our case LCM(5, 10, 20) = 20 ticks. After 20 ticks, every task's periods all align back to their starting point simultaneously, and the schedule repeats identically. Simulating beyond the hyperperiod would just show the same pattern again.

**Q: How do you detect a deadline miss in your code?**
> At the start of every new period (when `tick % task.period == 0`), I check if `task.time_left > 0`. If it is, the previous period's work wasn't completed — that's a deadline miss. The deadline was the end of the previous period.

**Q: In EDF, what is the absolute deadline?**
> When a task's period starts at tick T, its absolute deadline = T + period. For example, if Task 1 (period=5) activates at tick 10, its absolute deadline = 10 + 5 = 15. It must finish before tick 15. EDF picks the task with the smallest absolute deadline value — i.e., the one that will run out of time soonest.
