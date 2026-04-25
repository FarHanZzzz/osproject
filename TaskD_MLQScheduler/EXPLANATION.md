# 📋 Task D — MLQ Scheduler (ELI5 Edition)

> **Quick Overview:** This program simulates a CPU scheduler with 3 priority queues. Queue 1 uses Round Robin (5 ticks), Queue 2 uses Shortest Job First (3 ticks), Queue 3 uses First Come First Served (2 ticks). It cycles through all queues until every process finishes. **Key OS concept: Multilevel Queue CPU Scheduling.**

---

Imagine you run a **hospital emergency room**. Patients walk in with different urgency levels:
- 🚨 **Code Red (Queue 1):** Heart attack, gunshot wound — these get treated FIRST
- 🟡 **Code Yellow (Queue 2):** Broken arm, deep cut — important but can wait a bit
- 🟢 **Code Green (Queue 3):** Common cold, headache — you'll get treated eventually

But here's the twist: you can't ONLY treat Code Red patients all day. The Code Green patients would wait forever and die of boredom. So the hospital director says:

> "Every 10-minute cycle, spend **5 minutes on Code Red**, **3 minutes on Code Yellow**, and **2 minutes on Code Green**."

That's a **Multilevel Queue (MLQ) Scheduler**!

---

## 🎯 The Big Picture

```
Each cycle repeats until all processes are done:
  │
  ├── Q1 (Round Robin) gets 5 ticks
  │     Give each Q1 process 1 tick, then move to the next.
  │     Circle back around if budget remains.
  │
  ├── Q2 (Shortest Job First) gets 3 ticks
  │     Find the process with the LEAST remaining time.
  │     Run it for 1 tick. Repeat.
  │
  └── Q3 (First Come First Served) gets 2 ticks
        Run the FIRST process in line until budget runs out.
```

The scheduler cycles Q1 → Q2 → Q3 → Q1 → Q2 → Q3 → ... until every process has `time_left == 0`.

---

## 🔧 How to Run It

```bash
g++ -o mlq_scheduler mlq_scheduler.cpp
./mlq_scheduler
```

---

## 📖 Code Walkthrough — The Fun Version

### Part 1: The Three Scheduling Algorithms

#### 🔄 Round Robin (Q1) — "Fair Share"

Imagine **5 kids sharing one Xbox controller.** Each kid gets to play for exactly 1 minute, then passes the controller to the next kid. When the last kid finishes their turn, the controller goes back to the first kid. Nobody gets special treatment. Everyone gets equal time.

```c
// The for-loop naturally creates the "circle" effect:
for (int i = 0; i < procs.size(); i++) {
    if (procs[i].queue_level == 1 && procs[i].time_left > 0) {
        procs[i].time_left--;   // Play for 1 tick
        q1_budget--;             // Budget decreases
    }
}
// When the loop finishes, the while-loop starts it again from i=0 (back to Kid 1!)
```

**Why Round Robin for high-priority?** System processes (like your mouse driver, keyboard driver) are often short-lived and need quick, fair access. RR guarantees nobody starves.

**What's the "quantum"?** The quantum is how long each process runs before being forced to give up the CPU. In our code, the quantum is 1 tick (each process runs for 1 tick before we move to the next). In real OS schedulers, it's typically 10-100 milliseconds.

---

#### ⚡ Shortest Job First (Q2) — "Express Lane"

Imagine the **express checkout at a grocery store.** You look at everyone in line and let the person with the FEWEST items go first. Why? Because they'll be done quickly, reducing the average wait time for everyone.

```c
// Scan ALL Q2 processes, find the one with smallest time_left
int shortest_idx = -1;
int shortest_time = 999999;

for (int i = 0; i < procs.size(); i++) {
    if (procs[i].queue_level == 2 && procs[i].time_left > 0) {
        if (procs[i].time_left < shortest_time) {
            shortest_time = procs[i].time_left;
            shortest_idx = i;
        }
    }
}
// Run that process for 1 tick
procs[shortest_idx].time_left--;
```

**Why SJF for medium priority?** Interactive processes (like a text editor responding to your keystrokes) tend to have short bursts. SJF minimizes average waiting time, making the system feel snappy.

**The downside of SJF:** **Starvation.** If short jobs keep arriving, a long job might NEVER get to run. That's why SJF is used for the middle queue, not the top one.

---

#### 📏 First Come First Served (Q3) — "Take a Number"

The simplest algorithm possible. **It's a line at the DMV.** Whoever arrived first gets served first. No cutting, no exceptions. You wait your turn.

```c
for (int i = 0; i < procs.size(); i++) {
    while (procs[i].queue_level == 3 && procs[i].time_left > 0 && q3_budget > 0) {
        procs[i].time_left--;   // Keep running THIS process
        q3_budget--;
    }
}
```

**Why a `while` loop inside the `for` loop?** FCFS says "stick with the current process until it finishes (or the budget runs out)." Unlike RR where we give 1 tick and move on, FCFS runs the same process continuously.

**Why FCFS for low priority?** Batch jobs (like backups, log processing) don't care about response time. They just need to eventually complete. FCFS is the simplest and fairest for non-urgent work.

---

### Part 2: Time Slicing (The Budget System)

```c
int q1_budget = 5;   // Q1 gets 50% of CPU (5 out of 10 ticks)
int q2_budget = 3;   // Q2 gets 30% of CPU (3 out of 10 ticks)
int q3_budget = 2;   // Q3 gets 20% of CPU (2 out of 10 ticks)
```

⏱️ **Why budgets?** Without budgets, Q1 would run until ALL Q1 processes finish, and Q2/Q3 would starve. The budget forces the scheduler to switch queues after a fixed number of ticks.

**The ratio 5:3:2 = 50%:30%:20%.** This means:
- High-priority processes get half the CPU → fast response
- Medium-priority processes get 30% → decent speed
- Low-priority processes get 20% → they'll finish... eventually

---

### Part 3: The Big Loop

```c
while (!all_done) {
    all_done = true;
    for (auto &p : procs)
        if (p.time_left > 0) { all_done = false; break; }

    // Run Q1 for 5 ticks
    // Run Q2 for 3 ticks
    // Run Q3 for 2 ticks
}
```

🔁 **This outer loop is the "cycle" that repeats forever.** Each iteration:
1. Check: is everyone done? If yes, stop.
2. Give Q1 its 5 ticks of glory
3. Give Q2 its 3 ticks
4. Give Q3 its 2 ticks
5. Go back to step 1

**When does it end?** When every single process has `time_left == 0`. In our example with 6 processes totaling 23 ticks of work, the loop runs about 3 full cycles.

---

### Part 4: What "No Preemption Between Queues" Means

In our simple implementation, Q1 runs for exactly 5 ticks, THEN Q2 runs for exactly 3 ticks, etc. Even if a new Q1 process "arrives" while Q2 is running, Q2 keeps its full 3-tick budget. This is called **time-sliced MLQ**.

In a real OS, a more aggressive version called **preemptive MLQ** would immediately interrupt Q2 if a higher-priority Q1 process arrives. Our version is simpler and more predictable.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| MLQ (Multilevel Queue) | Multiple separate lines, each with its own rules. VIP line, regular line, economy line. |
| Round Robin | Kids sharing a game controller. 1 turn each, then pass it. Nobody gets special treatment. |
| Shortest Job First | Express checkout lane. Person with fewest items goes first. |
| FCFS | DMV line. First person to arrive gets served first. No cutting. |
| Quantum | How long each process gets the CPU before being forced to switch. Like the "turn length" in a board game. |
| Time Slice | A fixed chunk of CPU time assigned to each queue per cycle (5 ticks, 3 ticks, 2 ticks). |
| Starvation | A process that waits forever because other processes keep cutting in front. |
| Preemption | Kicking a running process off the CPU because a higher-priority one showed up. |
| Budget | The number of ticks a queue gets before the scheduler moves to the next queue. |
| Burst Time | The total amount of CPU time a process needs to complete its work. |
| Turnaround Time | Total time from when a process arrives until it finishes. (Completion - Arrival). |
| Waiting Time | Time spent NOT running. (Turnaround - Burst). |
