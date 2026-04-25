# 🏥 Task D — MLQ Scheduler (ELI5 Edition)

## The Story

Imagine a **hospital emergency room** with 3 waiting areas:

| Waiting Area | Who's Here | Rule | Time |
|---|---|---|---|
| 🚨 **Area 1 (System)** | Heart attacks, strokes | Take turns, 2 min each (Round Robin) | Gets 50% of doctor's time |
| 🤒 **Area 2 (Interactive)** | Broken bones, fevers | Quickest fix goes first (SJF) | Gets 30% of doctor's time |
| 🤕 **Area 3 (Batch)** | Papercuts, bruises | First come, first served (FCFS) | Gets 20% of doctor's time |

The doctor (CPU) cycles through:
1. Spend 5 minutes on Area 1 patients
2. Spend 3 minutes on Area 2 patients
3. Spend 2 minutes on Area 3 patients
4. Repeat!

Nobody starves — even papercut patients eventually get treated.

---

## 🔧 How to Run It

```bash
g++ -o mlq_scheduler mlq_scheduler.cpp
./mlq_scheduler
```

---

## 📖 Code Walkthrough — The Fun Version

### Part 1: Patient Info (Process Struct)

```cpp
struct Process {
    int pid;            // Patient ID
    int arrival;        // When they walked in
    int burst;          // Total treatment time needed
    int remaining;      // How much treatment is left
    int priority;       // 0-10=Area1, 11-20=Area2, 21-30=Area3
    int start_time;     // When they first saw the doctor (-1 = never)
    int completion;     // When treatment finished
    int turnaround;     // Total time in hospital (completion - arrival)
    int waiting;        // Time spent in waiting room (turnaround - burst)
    int response;       // Wait until first doctor visit (start - arrival)
};
```

**Why `start_time = -1`?** We need to know "has this patient EVER seen the doctor?" If `start_time` was 0, we couldn't tell the difference between "saw doctor at time 0" and "hasn't seen doctor yet". So `-1` means "never seen".

**The metrics:**
- **Turnaround** = When you leave - When you arrived = total hospital time
- **Waiting** = Turnaround - actual treatment time = time sitting around doing nothing
- **Response** = First time seeing doctor - arrival = "how long till I was first seen?"

---

### Part 2: Which Waiting Area?

```cpp
int get_queue(int priority) {
    if (priority <= 10) return 1;    // 🚨 Critical
    if (priority <= 20) return 2;    // 🤒 Medium
    return 3;                         // 🤕 Low
}
```

Simple. Priority 0–10 = Area 1. Priority 11–20 = Area 2. Everything else = Area 3.

---

### Part 3: Treating a Patient for 1 Tick

```cpp
void run_one_tick(Process *p, string qname) {
    if (p->start_time == -1) p->start_time = now;  // First time seeing doctor!
    p->remaining--;                                   // 1 minute of treatment done
    now++;                                             // Clock moves forward

    if (p->remaining == 0) {
        // Patient is CURED! 🎉 Calculate all their stats.
        p->completion  = now;
        p->turnaround  = p->completion - p->arrival;
        p->waiting     = p->turnaround - p->burst;
        p->response    = p->start_time - p->arrival;
    }
}
```

---

### Part 4: Area 1 — Round Robin 🔄

```cpp
void run_Q1(int budget) {    // budget = 5 ticks
    // 1. Take the patient at the FRONT of the line
    Process *p = ready.front();
    ready.pop_front();

    // 2. Treat them for at most 2 ticks (the quantum)
    int run_for = min({2, p->remaining, budget - spent});

    // 3. Not cured yet? Go to the BACK of the line
    if (p->remaining > 0)
        ready.push_back(p);
}
```

🔄 **Round Robin = taking turns.** Everyone gets exactly 2 minutes (quantum) with the doctor. If you need more time, you go to the back of the line and wait for your next turn. It's the fairest system — nobody hogs the doctor.

**Why `min({2, remaining, budget})`?** Three constraints at once:
- `2` = quantum (max per turn)
- `remaining` = don't treat longer than needed (patient with 1 min left only needs 1)
- `budget` = Q1 only gets 5 ticks total, don't go over

---

### Part 5: Area 2 — Shortest Job First 📏

```cpp
void run_Q2(int budget) {    // budget = 3 ticks
    // Sort patients by how much treatment they need (shortest first)
    sort(ready.begin(), ready.end(),
         [](Process *a, Process *b) { return a->remaining < b->remaining; });

    Process *p = ready[0];    // Pick the one needing LEAST time
    // Treat them (non-preemptive = once started, don't stop)
}
```

📏 **SJF = "who needs the least time? You go first!"** If Patient A needs 8 min and Patient B needs 2 min, B goes first. This minimizes average waiting time.

**Non-preemptive** means once you start treating someone, you finish (or run out of budget). You don't stop mid-treatment because someone shorter walked in.

---

### Part 6: Area 3 — First Come First Served 🎫

```cpp
void run_Q3(int budget) {    // budget = 2 ticks
    // Find whoever arrived EARLIEST
    Process *earliest = nullptr;
    for (auto &p : procs) {
        if (p.arrival < earliest->arrival)
            earliest = &p;
    }
    // Treat them
}
```

🎫 **FCFS = a regular line at a store.** Whoever showed up first gets treated first. Simple, fair by arrival order, but not efficient (a 1-minute patient behind a 30-minute patient waits a long time).

---

### Part 7: The Main Loop — Cycling Through Areas

```cpp
while (!all_done() && now < 200) {
    run_Q1(5);     // Doctor spends 5 ticks on Area 1
    if (all_done()) break;
    run_Q2(3);     // Then 3 ticks on Area 2
    if (all_done()) break;
    run_Q3(2);     // Then 2 ticks on Area 3
    // Repeat!
}
```

🔁 **The cycle:** Q1 → Q2 → Q3 → Q1 → Q2 → Q3 → ...

**Why time-slicing instead of strict priority?** Without it, if Q1 always has patients, Q2 and Q3 would **starve** (never get treated). Time-slicing guarantees Q3 gets at least 20% of the doctor's time.

**Why `now < 200`?** Safety net. If there's a bug and processes never finish, the program doesn't loop forever.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| MLQ | 3 separate lines, each with its own rules |
| Round Robin | Take turns, everyone gets same amount of time |
| SJF | Shortest job goes first = least average waiting |
| FCFS | Regular queue — first come, first served |
| Time Slicing | Doctor splits time between areas so nobody starves |
| Starvation | When low-priority patients NEVER get treated |
| Quantum | The max time you get per turn in Round Robin |
| Context Switch | When the doctor switches from one patient to another |
