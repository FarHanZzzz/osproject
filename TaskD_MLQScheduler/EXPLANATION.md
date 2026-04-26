# 📋 Task D — Time-Sliced MLQ Scheduler (ELI5 Edition)

> **Quick Overview:** This program simulates a CPU scheduler with 3 priority queues using time-sliced allocation. Q1 uses Round Robin (quantum=2, 50% CPU), Q2 uses Shortest Job First (30% CPU), Q3 uses FCFS (20% CPU). It outputs a Gantt chart and calculates Waiting Time, Turnaround Time, and Response Time. **Key OS concept: Multilevel Queue CPU Scheduling with Performance Metrics.**

---

## 🎯 The Big Picture

```
Processes are assigned to queues by priority:
  Priority 0–10  → Q1 (System)      — Round Robin, quantum=2
  Priority 11–20 → Q2 (Interactive)  — Shortest Job First
  Priority 21–30 → Q3 (Batch)        — First Come First Served

Each cycle (10 ticks total):
  ├── Q1 gets 5 ticks  (50% CPU)
  ├── Q2 gets 3 ticks  (30% CPU)
  └── Q3 gets 2 ticks  (20% CPU)

Repeat Q1 → Q2 → Q3 → Q1 → Q2 → Q3 ... until all done
```

---

## 🔧 How to Run It

```bash
g++ -o mlq_scheduler mlq_scheduler.cpp
./mlq_scheduler
```

---

## 📖 Code Walkthrough

### Part 1: Process Struct (All Required Fields)

```cpp
struct Process {
    int pid;
    int arrival_time;     // When the process enters the system
    int burst_time;       // Total CPU time needed
    int remaining_time;   // CPU time still left
    int priority;         // 0-10=Q1, 11-20=Q2, 21-30=Q3
    int start_time;       // First time process gets CPU (for response time)
    int finish_time;      // When process completes
    bool activated;       // Has been added to its queue?
};
```

📋 The struct tracks everything needed for the performance metrics:
- `start_time` → used for **Response Time** = start_time - arrival_time
- `finish_time` → used for **Turnaround Time** = finish_time - arrival_time
- `priority` → determines which queue (via `get_queue()` helper)

---

### Part 2: Separate Queue Data Structures

```cpp
deque<int> q1, q2, q3;  // Three separate deques
```

🗄️ We use `std::deque` (double-ended queue) for each queue instead of a single vector. This allows:
- **Q1 (RR):** `pop_front()` → run → `push_back()` (circular rotation)
- **Q2 (SJF):** Find shortest → `erase()` → `push_back()` if not done
- **Q3 (FCFS):** `pop_front()` → run → `push_front()` if not done (resume same process)

---

### Part 3: Round Robin with Quantum=2

```cpp
int budget = 5;
while (budget > 0 && !q1.empty()) {
    int idx = q1.front();
    q1.pop_front();
    int ticks = min({2, budget, procs[idx].remaining_time});  // quantum=2
    for (int i = 0; i < ticks; i++) {
        run_one_tick(idx);
        budget--;
    }
    if (procs[idx].remaining_time == 0) { /* finished */ }
    else q1.push_back(idx);  // Back of queue
}
```

🔄 Each process gets at most **2 ticks** (the quantum), then goes to the back of the queue. The budget (5 ticks) limits total Q1 time per cycle.

---

### Part 4: Dynamic Arrivals

```cpp
auto activate_arrivals = [&]() {
    for (int i = 0; i < n; i++) {
        if (!procs[i].activated && procs[i].arrival_time <= current_time) {
            procs[i].activated = true;
            int ql = get_queue(procs[i].priority);
            if (ql == 1) q1.push_back(i);
            else if (ql == 2) q2.push_back(i);
            else q3.push_back(i);
        }
    }
};
```

⏰ At every tick, we check if any new processes have arrived and add them to the correct queue based on their priority.

---

### Part 5: Gantt Chart Output

```
=== Gantt Chart ===
  | P1 | P1 | P2 | P2 | P1 | P4 | P4 | P3 | P5 | P5 | P2 | ...
     0    1    2    3    4    5    6    7    8    9   10  ...
```

The program records which process ran at each tick and displays a visual timeline.

---

### Part 6: Performance Metrics

```
=== Performance Metrics ===
  PID  Queue  Arrival  Burst  Finish  Waiting  Turnaround  Response
   1     Q1       0       5      13        8          13         0
   2     Q1       1       3      11        7          10         1
   ...

  Avg Waiting Time:    10.50
  Avg Turnaround Time: 14.33
  Avg Response Time:   6.00
```

| Metric | Formula |
|--------|---------|
| **Waiting Time** | Finish - Arrival - Burst |
| **Turnaround Time** | Finish - Arrival |
| **Response Time** | First CPU Time - Arrival |

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| MLQ (Multilevel Queue) | Multiple separate lines, each with its own rules. |
| Round Robin | Kids sharing a controller. Each gets 2 turns, then pass. |
| Shortest Job First | Express checkout. Fewest items goes first. |
| FCFS | DMV line. First to arrive, first served. |
| Quantum | How long each process gets before being forced to switch (2 ticks). |
| Time Slice | Fixed CPU time per queue per cycle (5:3:2 = 50%:30%:20%). |
| Waiting Time | Time spent NOT running. |
| Turnaround Time | Total time from arrival to completion. |
| Response Time | Time from arrival to first getting the CPU. |
| Gantt Chart | Visual timeline showing which process ran at each tick. |
| Priority-based Queue Assignment | Priority 0-10 → Q1, 11-20 → Q2, 21-30 → Q3. |
