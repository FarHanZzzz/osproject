# Task D — MLQ Scheduler: Viva Preparation

---

## 🧠 What Is The Problem?

The assignment asks: *"Simulate a CPU scheduler that has THREE separate queues. Each queue uses a different scheduling algorithm and gets a fixed percentage of CPU time."*

Real operating systems categorize processes. A system process (like the kernel) needs the CPU urgently. An interactive app (like a browser) needs to feel responsive. A background job (like a file backup) can wait. A single queue with one algorithm cannot serve all these fairly.

**The solution: Multiple queues, each with its own rules, each getting a fixed slice of CPU time.**

---

## 💬 How To Explain The Problem (Your Opening Line)

> *"The problem is that different types of processes have different priority needs. System processes are urgent, interactive processes need quick responses, and batch jobs can wait. One queue with one algorithm doesn't work for all of them. My MLQ scheduler divides CPU time: Q1 gets 50% and uses Round Robin, Q2 gets 30% and uses SJF, Q3 gets 20% and uses FCFS. This cycles repeatedly until all processes finish."*

---

## ⚙️ How Your Code Solves It — Step By Step

### Step 1 — Process Struct (From PDF)
The PDF specifies exactly this struct:
```cpp
struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;  // 0-10=Q1, 11-20=Q2, 21-30=Q3
};
```
I added `start_time`, `finish_time`, and `activated` to track metrics and arrivals.

### Step 2 — Queue Assignment by Priority
- Priority 0–10 → Q1 (System)
- Priority 11–20 → Q2 (Interactive)
- Priority 21–30 → Q3 (Batch)
- Processes enter their queue only when their `arrival_time` is reached

### Step 3 — The Time Slice Cycle
Every cycle of the scheduler:
1. **Q1 gets 5 ticks (50%)** — uses Round Robin with quantum=2
2. **Q2 gets 3 ticks (30%)** — uses Shortest Job First
3. **Q3 gets 2 ticks (20%)** — uses First Come First Served
4. Repeat from Q1

### Step 4 — Round Robin (Q1)
- Take process from front of queue
- Run it for `min(2, budget, remaining_time)` ticks
- If not done → push to BACK of queue (the "rotating" part of Round Robin)
- If done → mark finish_time, increment completed

### Step 5 — Shortest Job First (Q2)
- Scan all processes in Q2
- Pick the one with the SMALLEST `remaining_time`
- Run it until done or budget exhausted
- If not done → push back to Q2

### Step 6 — First Come First Served (Q3)
- Always take the process at the FRONT
- Run until done or budget exhausted
- If not done → push back to FRONT (so it continues next cycle)

### Step 7 — Dynamic Arrivals
- `activate_arrivals()` is called after every tick
- Any process whose `arrival_time <= current_time` gets added to its queue
- This means processes can arrive mid-simulation and get picked up correctly

### Step 8 — Metrics and Gantt Chart
- **Waiting Time** = Turnaround − Burst = (finish − arrival) − burst
- **Turnaround Time** = finish_time − arrival_time
- **Response Time** = start_time − arrival_time
- **Gantt Chart**: array of PIDs run at each tick, printed as a bar chart

---

## 🔥 Challenges & What You'd Say You Faced

> *"The hardest part was handling the interaction between the budget system and Round Robin. In classic Round Robin, you run until quantum expires or process finishes. But here, Q1's total budget is 5 ticks for the whole queue — not per process. I had to use `min(quantum, budget, remaining_time)` to respect all three limits simultaneously."*

> *"Dynamic arrivals were also tricky. A process with `arrival_time=2` should not enter Q1 until tick 2. I solved this with an `activate_arrivals()` helper that's called after every single tick. It scans all processes and moves any newly arrived ones into their queues."*

> *"For FCFS in Q3, when a process isn't finished after the budget runs out, it must go back to the FRONT of the queue (not the back). If it went to the back, other processes that arrived later would jump ahead — violating the "first come" principle. I use `push_front()` on the deque for this."*

---

## 🤔 Choices You Made & Why

| Choice | Why |
|--------|-----|
| `deque<int>` instead of `queue<int>` | Deque allows both `push_front` and `push_back` — needed for FCFS back-insertion |
| Store indices, not Process objects, in queues | Updating `remaining_time` in the queue directly isn't possible; indices let us modify the original `procs` vector |
| `activate_arrivals()` called every tick | Ensures processes are detected as soon as they arrive, even mid-cycle |
| `min({2, budget, remaining_time})` | Three-way min with initializer list — cleaner than nested `if` |
| `start_time = -1` initially | Sentinel value: tells us the process hasn't gotten CPU yet (needed for Response Time) |

---

## 📚 Concepts You Need To Know

### Round Robin (RR)
- Each process gets a fixed time slice called the **quantum** (ours: 2 ticks)
- After its quantum expires, it goes to the BACK of the queue
- The next process in line gets its turn — fair rotation
- **Good for**: interactive systems where everyone needs a turn
- **Trade-off**: more context switches than FCFS

### Shortest Job First (SJF)
- The process with the LEAST remaining work goes first
- Minimizes average waiting time (mathematically provable)
- **Good for**: batch systems where short jobs pile up
- **Trade-off**: long processes may wait forever ("starvation") if short ones keep arriving

### First Come First Served (FCFS)
- Processes run in the order they arrived — no reordering
- Simplest algorithm: just a queue
- **Good for**: batch jobs where order of completion matters
- **Trade-off**: a long job at the front blocks all jobs behind it ("convoy effect")

### Multilevel Queue (MLQ) Scheduler
- Multiple queues, each for a different CLASS of process
- Each queue has its own algorithm suited to its process type
- Processes are PERMANENTLY assigned to one queue (by priority)
- CPU time is divided among queues in fixed proportions

### Performance Metrics (Required by PDF)

| Metric | Formula | Meaning |
|--------|---------|---------|
| **Waiting Time** | Finish − Arrival − Burst | Time spent in queue, not running |
| **Turnaround Time** | Finish − Arrival | Total time from arrival to completion |
| **Response Time** | First CPU − Arrival | Delay before the process first gets the CPU |

### Gantt Chart
- A timeline showing which process was running at each tick
- Looks like: `| P1 | P1 | P2 | P4 | P4 | P3 | ...`
- Useful for visually verifying the scheduling order

### Context Switch
- When the CPU switches from running one process to another
- The OS saves the state of the current process (registers, program counter)
- Restores the state of the next process
- Has a small overhead cost — too many context switches reduce efficiency

---

## ✅ Quick Viva Q&A

**Q: Why does Q1 use Round Robin and not FCFS?**
> System processes are typically short and must feel responsive. Round Robin ensures no single system process monopolizes the CPU — each gets a fair time slice and then yields to the next. FCFS would let a slow system process block urgent ones.

**Q: Why does Q2 use Shortest Job First?**
> Interactive processes (user applications) benefit from SJF because short tasks complete quickly, making the system feel fast and responsive. SJF also minimizes average waiting time mathematically.

**Q: What is the convoy effect in FCFS?**
> If a very long process arrives first, it runs to completion before shorter processes behind it can start. The short processes "convoy" behind the long one. This hurts average waiting time significantly.

**Q: What's the difference between Waiting Time and Response Time?**
> Response time is how long until the process FIRST gets the CPU — important for interactive apps. Waiting time is the total time spent NOT running. A process can have a short response time but a long waiting time if it runs briefly early but then waits a long time before finishing.

**Q: How do processes get assigned to queues?**
> By their priority number. Priority 0–10 goes to Q1, 11–20 to Q2, 21–30 to Q3. The function `get_queue(priority)` handles this mapping. Once assigned, a process stays in its queue forever — it cannot "upgrade" or "downgrade" between queues.

**Q: What happens if a queue runs out of budget before all its processes finish?**
> The unfinished processes stay in the queue. The scheduler moves on to the next queue in the cycle. Those processes will get more CPU time when the cycle comes back to their queue.

**Q: What if ALL queues are empty but some processes haven't arrived yet?**
> The scheduler runs an IDLE tick, advancing time by 1, then calls `activate_arrivals()` to check if any new processes have arrived. This prevents an infinite loop.
