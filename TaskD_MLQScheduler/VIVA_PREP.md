# Task D — MLQ Scheduler: Viva Preparation

---

## 🧠 What Is The Problem?

The assignment asks: *"Simulate a CPU scheduler that has THREE separate queues. Each queue uses a different scheduling algorithm and gets a fixed percentage of CPU time."*

Real operating systems categorize processes. A system process (like the kernel) needs the CPU urgently. An interactive app (like a browser) needs to feel responsive. A background job (like a file backup) can wait. A single queue with one algorithm cannot serve all these fairly.

**The solution: Multiple queues, each with its own rules, each getting a fixed slice of CPU time.**

---

## 💬 How To Explain The Problem (Your Opening Line)

> *"The problem is that different types of processes have different priority needs. We used an ELI5 analogy of a Teacher helping Students. Group 1 (System) is urgent, Group 2 (Interactive) needs quick responses, and Group 3 (Batch) can wait. My MLQ scheduler divides the Teacher's time: Group 1 gets 50% (Round Robin), Group 2 gets 30% (SJF), and Group 3 gets 20% (FCFS). This cycles repeatedly until all students are helped."*

---

## ⚙️ How Your Code Solves It — Step By Step

### Step 1 — Student (Process) Struct
The code represents a Process as a `Student` waiting for the Teacher:
```cpp
struct Student {
    int id;               
    int arrival_time;     
    int total_time_needed;
    int time_left;        
    int priority_score;   // 0-10=Group1, 11-20=Group2, 21-30=Group3
    int first_helped_time;
    int finished_time;    
    bool is_in_line;      
};
```

### Step 2 — Queue Assignment by Priority
- Priority 0–10 → Group 1 (System)
- Priority 11–20 → Group 2 (Interactive)
- Priority 21–30 → Group 3 (Batch)
- Students enter their line only when their `arrival_time` is reached

### Step 3 — The Time Slice Cycle
Every cycle of the teacher's time:
1. **Group 1 gets 5 minutes (50%)** — uses Round Robin with cycle limit=2
2. **Group 2 gets 3 minutes (30%)** — uses Shortest Job First
3. **Group 3 gets 2 minutes (20%)** — uses First Come First Served
4. Repeat from Group 1

### Step 4 — Round Robin (Group 1)
- Take student from front of line
- Help them for `min(2, allowed_time, time_left)` minutes
- If not done → push to BACK of line (the "rotating" part of Round Robin)
- If done → mark finish_time, increment completed students

### Step 5 — Shortest Job First (Group 2)
- Scan all students in Group 2 line
- Pick the one with the SMALLEST `time_left`
- Help them until done or allowed time exhausted
- If not done → keep them in line

### Step 6 — First Come First Served (Group 3)
- Always take the student at the FRONT
- Help until done or allowed time exhausted
- If not done → push back to FRONT (so they continue next cycle)

### Step 7 — Dynamic Arrivals
- `check_for_new_arrivals()` is called after every minute of teaching
- Any student whose `arrival_time <= current_minute` gets added to their line
- This means students can walk in mid-simulation and get picked up correctly

### Step 8 — Metrics and Gantt Chart
- **Waiting Time** = Turnaround − Time Needed = (finish − arrival) − total_time_needed
- **Turnaround Time** = finish_time − arrival_time
- **Response Time** = first_helped_time − arrival_time
- **Timeline**: an array of Student IDs helped at each minute, printed as a timeline

---

## 🔥 Challenges & What You'd Say You Faced

> *"The hardest part was handling the interaction between the time budget and Round Robin. In classic Round Robin, you run until the quantum expires or process finishes. But here, Group 1's total budget is 5 minutes for the whole group — not per student. I had to calculate `minutes_to_talk` based on three limits simultaneously."*

> *"Dynamic arrivals were also tricky. A student arriving at minute 2 should not be in line until minute 2. I solved this with a `check_for_new_arrivals()` helper that's called constantly. It scans the classroom and moves any newly arrived students into their lines."*

> *"For FCFS in Group 3, when a student isn't finished after the budget runs out, they must stay at the FRONT of the line (not the back). If they went to the back, other students that arrived later would jump ahead — violating the "first come" principle. I use `push_front()` on the deque for this."*

---

## 🤔 Choices You Made & Why

| Choice | Why |
|--------|-----|
| `deque<int>` instead of `queue<int>` | Deque allows both `push_front` and `push_back` — needed for FCFS back-insertion |
| Store indices, not Student objects, in lines | Updating `time_left` in the line directly isn't possible; indices let us modify the original `classroom` vector |
| `check_for_new_arrivals()` called every minute | Ensures students are detected as soon as they walk in |
| Teacher/Student Analogy | Makes it trivially easy to explain standard MLQ algorithms to non-experts |
| `first_helped_time = -1` initially | Sentinel value: tells us the student hasn't gotten help yet (needed for Response Time) |

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
| **Waiting Time** | Finish − Arrival − Total Needed | Time spent standing in line, not getting help |
| **Turnaround Time** | Finish − Arrival | Total time from walking in to going home |
| **Response Time** | First Help − Arrival | Delay before the student gets any help at all |

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
> By their priority score. Priority 0–10 goes to Group 1, 11–20 to Group 2, 21–30 to Group 3. The function `get_group_number(priority)` handles this mapping. Once assigned, they stay in that group.

**Q: What happens if a queue runs out of budget before all its processes finish?**
> The unfinished students stay in their line. The Teacher moves on to the next group in the cycle. Those students will get more time when the teacher circles back to their group.

**Q: What if ALL queues are empty but some processes haven't arrived yet?**
> The scheduler runs an IDLE tick, advancing time by 1, then calls `activate_arrivals()` to check if any new processes have arrived. This prevents an infinite loop.
