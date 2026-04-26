# Task D: MLQ Scheduler — The Viva Defense Walkthrough

Imagine you're walking into the viva. The examiner asks: *"How did you approach Task D, and why did you write the code this way?"*

This document is your exact thought process, from reading the question to writing the final line of code.

---

## Step 1: Reading the Question & Finding the Prerequisites
The prompt asked to:
1. Simulate a **Multilevel Queue (MLQ) CPU Scheduler** with 3 queues.
2. Q1 uses **Round Robin** (gets 50% CPU = 5 ticks/cycle).
3. Q2 uses **Shortest Job First** (gets 30% CPU = 3 ticks/cycle).
4. Q3 uses **First Come First Served** (gets 20% CPU = 2 ticks/cycle).
5. The scheduler cycles Q1 → Q2 → Q3 → Q1 → Q2 → Q3 → ... until done.

**My immediate thought process:**
"This isn't about actually running processes. I need to SIMULATE what a CPU scheduler does — decide who gets the CPU and for how long, using different algorithms for different queues."

**Prerequisites I had to learn before coding:**
1.  **Round Robin (RR):** Each process gets a fixed time slice (1 tick). If it's not done, go to the next process. Like kids sharing a game controller — everyone gets one turn, then pass it.
2.  **Shortest Job First (SJF):** Always pick the process with the LEAST remaining time. The idea: finish short jobs quickly to reduce average wait time.
3.  **First Come First Served (FCFS):** Run the first process in line until it finishes (or budget runs out), then move to the next. Like a queue at a bank — first in, first served.
4.  **Time-Sliced MLQ:** The CPU is split into budgets. Q1 gets 5 ticks, then Q2 gets 3 ticks, then Q3 gets 2 ticks. Repeat. Higher-priority queues get more CPU.

---

## Step 2: The Mental Blueprint (ELI5)
*   **Me (The CPU):** I have 3 waiting rooms (queues). Room 1 is VIP, Room 2 is normal, Room 3 is economy.
*   **The Rule:** I spend 5 minutes in Room 1 (Round Robin), then 3 minutes in Room 2 (Shortest Job First), then 2 minutes in Room 3 (First Come First Served). Then I loop back.
*   **Room 1 (RR):** I serve each person for exactly 1 minute, then rotate. Everyone gets fair turns.
*   **Room 2 (SJF):** I look at everyone's paperwork and serve whoever has the quickest task first.
*   **Room 3 (FCFS):** I just serve whoever's been waiting the longest, no cutting.

---

## Step 3: Writing the Code (Line-by-Line Rationale)

### 1. The Process Struct
```cpp
struct Process {
    int id;           // Process name tag (P1, P2, etc.)
    int queue_level;  // Which queue: 1, 2, or 3
    int time_needed;  // Total burst time
    int time_left;    // How much time is STILL remaining
};
```
*Why `time_left` AND `time_needed`?* `time_needed` is the original burst (never changes). `time_left` tracks remaining work and decreases every tick. When `time_left` hits 0, the process is done.

### 2. Hardcoded Processes
```cpp
vector<Process> procs = {
    {1, 1, 5, 5},  // P1 in Q1, needs 5 ticks
    {2, 1, 3, 3},  // P2 in Q1, needs 3 ticks
    {3, 2, 6, 6},  // P3 in Q2, needs 6 ticks
    {4, 2, 2, 2},  // P4 in Q2, needs 2 ticks
    {5, 3, 4, 4},  // P5 in Q3, needs 4 ticks
    {6, 3, 3, 3},  // P6 in Q3, needs 3 ticks
};
```

### 3. The Big Scheduling Loop
```cpp
while (!all_done) {
    // Check if any process still has work
    for (auto &p : procs)
        if (p.time_left > 0) { all_done = false; break; }

    // Q1: Round Robin (5 ticks) → Q2: SJF (3 ticks) → Q3: FCFS (2 ticks)
}
```
Each cycle: give Q1 its 5-tick budget, Q2 its 3-tick budget, Q3 its 2-tick budget.

### 4. Queue 1 — Round Robin
```cpp
int q1_budget = 5;
while (q1_budget > 0 && q1_has_work) {
    q1_has_work = false;
    for (int i = 0; i < procs.size(); i++) {
        if (procs[i].queue_level == 1 && procs[i].time_left > 0 && q1_budget > 0) {
            procs[i].time_left--;  // Run for 1 tick
            q1_budget--;
            clock_tick++;
        }
    }
}
```
*Key insight:* The inner `for` loop cycles through ALL Q1 processes, giving each exactly 1 tick. The outer `while` loop repeats until the budget is spent. This IS Round Robin — fair, rotating turns.

### 5. Queue 2 — Shortest Job First
```cpp
int q2_budget = 3;
while (q2_budget > 0) {
    int shortest_idx = -1, shortest_time = 999999;
    for (int i = 0; i < procs.size(); i++) {
        if (procs[i].queue_level == 2 && procs[i].time_left > 0)
            if (procs[i].time_left < shortest_time) {
                shortest_time = procs[i].time_left;
                shortest_idx = i;
            }
    }
    if (shortest_idx == -1) break;  // No Q2 work left
    procs[shortest_idx].time_left--;
    q2_budget--;
    clock_tick++;
}
```
*Key insight:* Every tick, we scan ALL Q2 processes and find the one with the smallest `time_left`. This is preemptive SJF — if a shorter job appears, it gets priority.

### 6. Queue 3 — First Come First Served
```cpp
int q3_budget = 2;
for (int i = 0; i < procs.size() && q3_budget > 0; i++) {
    while (procs[i].queue_level == 3 && procs[i].time_left > 0 && q3_budget > 0) {
        procs[i].time_left--;
        q3_budget--;
        clock_tick++;
    }
}
```
*Key insight:* We find the FIRST Q3 process and keep running it until budget is gone or it finishes. Only then move to the next. No preemption, no fairness — just order.

---

## Step 4: Problems Faced & "What Ifs" (Viva Goldmine)

### 1. "Why Time-Sliced Instead of Strict Priority?"
**Answer:** In strict priority, Q1 would STARVE Q2 and Q3 entirely — they'd never run until Q1 is empty. Time-slicing guarantees every queue gets SOME CPU time (50/30/20 split), preventing starvation.

### 2. "Why Not Use Actual OS Threads for Each Process?"
**Answer:** "This is a SIMULATION. We're modeling what the OS scheduler does internally. The OS scheduler itself runs as a single piece of code deciding who gets the CPU next. Using actual threads would be like building a real airplane to test aerodynamics when a wind tunnel model works perfectly."

### 3. "What's the Difference Between RR, SJF, and FCFS?"

| Algorithm | Fairness | Starvation Risk | Best For |
|---|---|---|---|
| RR | Very fair (everyone gets equal turns) | None | Interactive systems |
| SJF | Unfair (short jobs win) | Long jobs can starve | Batch systems |
| FCFS | Fair (first-come order) | None, but slow | Simple systems |

### 4. "Could Processes Move Between Queues?"
**Answer:** "That would be a Multilevel FEEDBACK Queue (MLFQ), not MLQ. In MLFQ, a process that uses too much CPU gets demoted to a lower queue. Our MLQ is simpler — processes stay in their assigned queue forever."

### 5. "What Happens If Q1 Has No Work During Its Budget?"
**Answer:** "The unused ticks are simply lost. We don't transfer leftover budget to Q2 or Q3. The scheduler just moves on to the next queue."
