# Task D — MLQ Scheduler
## How the Code Works — Complete Explanation

> **Source:** `mlq_scheduler.cpp` | **Language:** C++ | **OS Concepts:** Multilevel Queue, Time Slicing, Round Robin, SJF, FCFS

---

## What This Program Does

This program simulates a Multilevel Queue (MLQ) CPU scheduler. Instead of a single queue for all processes, it categorizes processes into three strict queues based on priority:
1. **System (Priority 0-10):** Runs Round Robin (quantum=2). Gets 50% CPU time (5 ticks).
2. **Interactive (Priority 11-20):** Runs Shortest Job First (non-preemptive). Gets 30% CPU time (3 ticks).
3. **Batch (Priority 21-30):** Runs First Come First Served. Gets 20% CPU time (2 ticks).

To prevent lower queues from starving, it utilizes an overarching time-slicing mechanism that cycles through the queues: Q1(5 ticks) → Q2(3 ticks) → Q3(2 ticks) → Repeat.

---

## How It Works — Step by Step

### 1. Process Classification (Lines 73–78)

```cpp
static int classify_queue(Process &p) {
    if (p.priority <= 10) { p.queue_name = "Q1-System";      return 1; }
    if (p.priority <= 20) { p.queue_name = "Q2-Interactive"; return 2; }
                            p.queue_name = "Q3-Batch";        return 3;
}
```

Whenever we need to interact with a process, we check its priority and assign it to its static queue. In a true MLQ (unlike MLFQ), processes cannot change queues. 

### 2. The Time-Sliced Simulation Loop (Lines 362–368)

```cpp
while (!all_done() && current_time < MAX_TICKS) {
    run_Q1_RR  (BUDGET_Q1); // 5 ticks
    if (all_done()) break;
    run_Q2_SJF (BUDGET_Q2); // 3 ticks
    if (all_done()) break;
    run_Q3_FCFS(BUDGET_Q3); // 2 ticks
}
```

This is the core execution cycle. Instead of letting Q1 run until it is completely empty (which would starve Q2 and Q3), the system gives a strict time budget to each queue. We check `all_done()` between calls so we don't log idle ticks if the entire system finishes early.

### 3. Queue 1: Round Robin Mechanics (Lines 124–154)

```cpp
int run_for = std::min({QUANTUM, p->remaining_time, time_budget - spent});
for (int i = 0; i < run_for; i++) {
    run_tick(p, "Q1");
    spent++;
    collect_arrived(ready, 1); // Get new arrivals
}
if (p->remaining_time > 0)
    ready.push_back(p);
```

For Round Robin, we pull a process from the `front` of the ready deque. 
We determine how long it can run using a 3-way minimum:
1. It can't exceed the RR `QUANTUM` (2 ticks).
2. It can't run longer than its `remaining_time`.
3. It can't exceed the queue's remaining `time_budget`.

If the process isn't finished after its turn, it gets placed at the `back` of the deque, waiting for its next turn. New arrivals are checked every tick so they interleave properly.

### 4. Queue 2: Shortest Job First Mechanics (Lines 159–193)

```cpp
std::sort(ready.begin(), ready.end(),
    [](Process *a, Process *b) {
        return a->remaining_time < b->remaining_time;
    });

Process *p = ready[0];
int run_for = std::min(p->remaining_time, time_budget - spent);
```

To implement SJF, we collect all arrived Q2 processes into a vector and sort them based on `remaining_time`. The process at index 0 is the shortest. Since it's **non-preemptive**, it runs until it finishes or until the queue's time budget runs out. We re-sort every time we pick a new process because a shorter job might have arrived.

### 5. Queue 3: First Come First Served (Lines 198–225)

```cpp
for (auto &proc : all_processes) {
    if (proc.arrival_time <= current_time && proc.remaining_time > 0 && classify_queue(proc) == 3) {
        if (!p || proc.arrival_time < p->arrival_time)
            p = &proc;
    }
}
```

FCFS is simple. We scan all processes and find the one that belongs to Q3, has arrived, and has the absolute lowest `arrival_time`.

### 6. Process Metric Calculations (Lines 100–113)

```cpp
if (p->start_time == -1) p->start_time = current_time;
p->remaining_time--;

if (p->remaining_time == 0) {
    p->completion_time  = current_time;
    p->turnaround_time  = p->completion_time - p->arrival_time;
    p->waiting_time     = p->turnaround_time - p->burst_time;
    p->response_time    = p->start_time      - p->arrival_time;
}
```

Metrics are gathered lazily inside `run_tick`. The moment a process finishes (`remaining_time == 0`), we calculate the exact metrics based on OS formulas.

---

## Key OS Concepts

| Concept | Where in Code | What It Does |
|---------|---------------|--------------|
| **Multilevel Queue** | `classify_queue` | Segregating processes by static priority into distinct execution buckets. |
| **Time Slicing** | Main `while` loop | Allocating distinct execution windows (5,3,2 ticks) to prevent starvation. |
| **Round Robin** | `run_Q1_RR` | Preempting processes after a time quantum, ensuring fair response times. |
| **SJF** | `run_Q2_SJF` | Sorting by remaining time to minimize average waiting time. |
| **Metrics** | `run_tick` | Turnaround, Waiting, and Response times evaluated mathematically. |

---

## Test Results (Verified)

| Metric Test (Manual Math vs Output) | Status |
|-------------------------------------|--------|
| P1: TAT = 12, WT = 6, RT = 0 | ✅ PASS |
| P2: TAT = 20, WT = 16, RT = 3 | ✅ PASS |
| Average TAT: 35.11 | ✅ PASS |
| Average WT: 30.11 | ✅ PASS |

---

## Viva Questions & Answers

**Q: What is the difference between MLQ and MLFQ (Multilevel Feedback Queue)?**
A: In MLQ (which this implements), processes are assigned to a queue based on a static property (like priority 0-10) and they *never leave that queue*. In MLFQ, processes dynamically move between queues based on their behavior (e.g., getting demoted if they use too much CPU time). 

**Q: How does this implementation solve the "Starvation" problem common in MLQ?**
A: If Q1 always had absolute priority, it could run continuously and completely starve Q2 and Q3. This implementation uses time-slicing across the queues. Q1 is guaranteed only 50% of the CPU cycles (5 ticks out of 10). Q2 is guaranteed 30%, and Q3 is guaranteed 20%. This ensures even the lowest priority batch jobs eventually make progress.

**Q: In Q2 (SJF), why do you sort the processes every single time a job finishes?**
A: Because new jobs can arrive while the current job is executing. If we only sorted at the beginning, we might miss a newly arrived process that has a burst time of 1, which should jump to the front of the queue.

**Q: Why does the RR algorithm use `push_back` instead of `push_front` on the deque?**
A: Round Robin requires that when a process finishes its time quantum, it goes to the end of the line so that the next process gets a fair turn. If we used `push_front`, the same process would immediately run again, defeating the purpose of fair sharing.
