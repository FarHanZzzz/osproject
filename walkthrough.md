# Code Walkthrough & Commentary
## CSC413/CSE315 — OS Programming Assignment, Spring 2026

> This document walks through every source file in detail — what each block of code does,
> **why** it was written that way, and what OS concept it demonstrates.
> Think of this as the "annotated" version of your codebase.

---

## Task A — `word_counter.c`

### Overall Strategy

The program solves a classic **parallel IPC problem**: how do you get multiple child processes to
report results back to one parent, without them stepping on each other?

The answer is **one pipe per child**. Each child gets its own private two-way "tube":
- Child writes its result into the tube's write-end
- Parent reads from the tube's read-end
- No child ever touches another child's tube → zero interference

```
                  pipe[0][0] ←──── child 0 (counts file1) ←── pipe[0][1]
Parent ────────── pipe[1][0] ←──── child 1 (counts file2) ←── pipe[1][1]
                  pipe[2][0] ←──── child 2 (counts file3) ←── pipe[2][1]
```

---

### Section-by-Section Commentary

#### Includes and Defines (Lines 28–37)

```c
#include <unistd.h>    // fork(), pipe(), read(), write(), close()
#include <sys/wait.h>  // waitpid(), WIFEXITED(), WEXITSTATUS()
#include <errno.h>     // errno, strerror()
#define MAX_FILES 64
```

**Why these headers?**
- `unistd.h` is the POSIX "swiss army knife" — all the core OS syscalls live here.
- `sys/wait.h` gives us `waitpid()`, which is the proper way to reap children.
- `errno.h` lets us turn system error codes (like `ENOENT`) into human-readable strings.
- `MAX_FILES 64` is a practical ceiling. The OS allows thousands of open file descriptors,
  but 64 files is more than enough for a word counter demo.

---

#### `count_words_in_file()` (Lines 44–63)

```c
while (fscanf(fp, "%4095s", word) == 1) {
    count++;
}
```

**What's happening:**
`fscanf` with `%s` automatically skips all whitespace (spaces, tabs, newlines) and reads
one "word" (any non-whitespace sequence) at a time. When it returns 1, it found a word.
When it returns `EOF` (-1), the file is done.

**Why `%4095s` instead of `%s`?**
A buffer overflow protection. The `word` array is 4096 bytes. Without the width limit,
a file containing a 10,000-character "word" (no spaces) would overflow the stack.
The `4095` leaves room for the null terminator.

**Why return -1 on error?**
Because `-1` is a signed int that can travel through the pipe. The parent receives it
and knows to treat it as an error signal rather than a word count.

---

#### Pipe Array Allocation (Lines 87–88)

```c
int   pipes[MAX_FILES][2];
pid_t pids[MAX_FILES];
```

**Why two-dimensional?**
Each pipe needs two file descriptors: `[0]` = read end, `[1]` = write end.
So `pipes[i][0]` is where the parent reads child i's result,
and `pipes[i][1]` is where child i writes its result.

---

#### The Fork Loop (Lines 93–146) — The Heart of the Program

```c
pipe(pipes[i]);   // Create pipe BEFORE fork
pids[i] = fork(); // Both parent and child inherit both pipe ends
```

**Critical sequence:**
1. `pipe()` must come BEFORE `fork()` — this is how both processes get the same pipe.
2. After `fork()`, the child gets a full copy of the parent's memory, including the pipe fds.
3. Now both parent and child have BOTH ends of the pipe open — we must close the unused ends.

**Why closing unused ends is NOT optional:**

```c
// In child: close the read end (child only writes)
close(pipes[i][0]);

// In parent: close the write end immediately after fork
close(pipes[i][1]);
```

If the parent forgets to close its write end of a pipe, then when the child writes its result
and exits, the parent's `read()` call will **block forever** — because the OS sees the pipe
still has a writer (the parent itself) and waits for more data that never comes.
This is the #1 beginner mistake with pipes.

**Why the child closes OTHER pipes too:**

```c
for (int j = 0; j < num_files; j++) {
    if (j != i) {
        close(pipes[j][0]);
        close(pipes[j][1]);
    }
}
```

If child 0 leaves pipe[1] and pipe[2] open, then even after child 1 and child 2 exit,
the parent's `read(pipes[1][0])` will block — because child 0 still has the write end open.
Children must close every pipe they don't own.

---

#### Collecting Results (Lines 152–173)

```c
// Read BEFORE waitpid
ssize_t bytes_read = read(pipes[i][0], &result, sizeof(result));
```

**Why read before waitpid?**
If the child's result is large (or if there are many children), the pipe's kernel buffer (typically
64KB) could fill up. A full pipe causes the child's `write()` to block. If the parent is
also blocked in `waitpid()`, both are waiting on each other → **deadlock**.

Reading first drains the pipe, unblocking the child, so it can exit normally.

---

#### The `waitpid()` Loop (Lines 180–189)

```c
pid_t finished = waitpid(pids[i], &status, 0);
```

**Why waitpid instead of wait?**
`wait()` waits for ANY child. `waitpid(pids[i], ...)` waits for a SPECIFIC child.
This lets us correlate exit status with the right child, which is important for diagnostics.

**What happens without waitpid?**
The child becomes a **zombie process** — its exit code sits in the kernel's process table,
consuming a PID slot, until the parent reads it. With `waitpid`, we reap it properly.

---

## Task C — `matrix_adder.c`

### Overall Strategy

The matrix add operation `C[i][j] = A[i][j] + B[i][j]` is **embarrassingly parallel** —
every element can be computed independently. We exploit this by giving each thread a
horizontal "stripe" of rows, so no two threads ever touch the same memory address.

```
Thread 0 → rows   0–249
Thread 1 → rows 250–499
Thread 2 → rows 500–749
Thread 3 → rows 750–999  (+ any remainder)
```

No locks. No atomics. Pure parallel speed.

---

### Section-by-Section Commentary

#### Feature-Test Macro (Line 1–2)

```c
#define _POSIX_C_SOURCE 199309L
```

**Why this line?**
`clock_gettime()` and `CLOCK_MONOTONIC` are POSIX extensions not included in the C11 standard.
This macro tells the compiler/glibc to expose them. Without it, compilation fails with
"undeclared identifier 'CLOCK_MONOTONIC'".

---

#### Static Matrix Allocation (Lines 47–50)

```c
static int A[ROWS][COLS];   // 1000×1000 = 1,000,000 ints = ~4 MB
static int B[ROWS][COLS];
static int C[ROWS][COLS];
static int C_ref[ROWS][COLS];
```

**Why `static` at file scope, not inside `main()`?**
Each matrix is ~4 MB. Four matrices = ~16 MB total. Stack size is typically 8 MB on Linux.
Declaring them inside a function would overflow the stack immediately.
`static` at file scope puts them in the BSS/data segment, which has no such limit.

---

#### `ThreadArgs` Struct (Lines 58–62)

```c
typedef struct {
    int thread_id;
    int start_row;   // inclusive
    int end_row;     // exclusive
} ThreadArgs;
```

**Why a struct?**
`pthread_create()` only accepts a single `void *` argument. To pass multiple values,
we pack them into a struct and pass a pointer to it. Each thread gets its own unique
`ThreadArgs` with different `start_row`/`end_row` values.

**Why `end_row` is exclusive?**
Standard C convention: `[start, end)` means the loop runs `for (i = start; i < end; i++)`.
Exclusive upper bounds make the "last thread handles remainder" math clean.

---

#### `thread_add_rows()` — The Worker Function (Lines 70–84)

```c
static void *thread_add_rows(void *arg) {
    ThreadArgs *ta = (ThreadArgs *)arg;
    for (int i = ta->start_row; i < ta->end_row; i++)
        for (int j = 0; j < COLS; j++)
            C[i][j] = A[i][j] + B[i][j];
    pthread_exit(NULL);
}
```

**Why no mutex here?**
Because `Thread 0` writes rows 0–249, `Thread 1` writes rows 250–499, etc.
No two threads share a write address. Read accesses to A and B are fine in parallel
because reading shared memory never causes data races — only concurrent writes do.

**Why `pthread_exit(NULL)` instead of `return NULL`?**
Both work. `pthread_exit` is the explicit, canonical way to terminate a thread.
It also supports deferred cancellation cleanup — good practice even when not strictly needed.

---

#### Chunk Size Calculation (Lines 156–168)

```c
int chunk     = ROWS / num_threads;   // base rows per thread
int remainder = ROWS % num_threads;   // leftover rows
```

**Example: 1000 rows, 3 threads**
- `chunk = 1000 / 3 = 333`
- `remainder = 1000 % 3 = 1`
- Thread 0: rows 0–332 (333 rows)
- Thread 1: rows 333–665 (333 rows)
- Thread 2: rows 666–999 (334 rows — absorbs the extra 1)

```c
args[t].end_row = current_row + chunk + (t == num_threads - 1 ? remainder : 0);
```

Only the **last thread** gets the extra rows. This ensures total coverage with no gaps or overlaps.

---

#### Timing with `clock_gettime` (Lines 179–212)

```c
clock_gettime(CLOCK_MONOTONIC, &t_start);
// ... spawn and join threads ...
clock_gettime(CLOCK_MONOTONIC, &t_end);
double ms = elapsed_ms(&t_start, &t_end);
```

**Why `CLOCK_MONOTONIC` instead of `time()`?**
`time()` has 1-second resolution. `CLOCK_MONOTONIC` has nanosecond resolution and is
guaranteed to never go backwards (unlike `CLOCK_REALTIME` which can jump due to NTP).
Perfect for measuring short durations accurately.

---

#### Correctness Verification (Lines 106–120)

```c
static int verify_result(void) {
    // Build single-threaded reference in C_ref
    // Compare C (multi-threaded) against C_ref
}
```

**Why bother verifying?**
A parallel program can produce silently wrong results if there's a bug in the partitioning.
This verification catches any overlap or gap in thread assignments immediately.
In our results: both always match ✓.

---

## Task D — `mlq_scheduler.cpp`

### Overall Strategy

MLQ scheduling works like a corporate hierarchy: System processes (Q1) get priority,
then Interactive (Q2), then Batch (Q3). But instead of starving the lower queues,
we use **time slicing** — Q1 gets 5 ticks, then Q2 gets 3, then Q3 gets 2, then repeat.

Within each queue, a different algorithm runs:
- Q1: Round Robin (fair sharing between system tasks)
- Q2: SJF (shortest interactive task finishes first)
- Q3: FCFS (batch jobs run in order)

---

### Section-by-Section Commentary

#### `Process` Struct (Lines 37–52)

```cpp
struct Process {
    int pid, arrival_time, burst_time, remaining_time, priority;
    int start_time = -1;      // -1 means "hasn't run yet"
    int completion_time = 0;
    int waiting_time = 0;
    int turnaround_time = 0;
    int response_time = 0;
    std::string queue_name;
};
```

**Why `start_time = -1` as sentinel?**
We need to distinguish "has this process ever gotten the CPU?" from "got CPU at time 0".
If `start_time` were 0 by default, we couldn't tell the difference.
The check `if (p->start_time == -1) p->start_time = current_time` sets it exactly once.

**How are the metrics calculated?**
- **Turnaround Time** = Completion − Arrival (total time in system)
- **Waiting Time** = Turnaround − Burst (time spent NOT running)
- **Response Time** = First CPU time − Arrival (time to first response)

---

#### `classify_queue()` (Lines 73–78)

```cpp
static int classify_queue(Process &p) {
    if (p.priority <= 10) { p.queue_name = "Q1-System";      return 1; }
    if (p.priority <= 20) { p.queue_name = "Q2-Interactive"; return 2; }
                            p.queue_name = "Q3-Batch";        return 3;
}
```

**Why classify dynamically instead of storing the queue number?**
It keeps the code DRY. Every function that needs to know a process's queue calls this.
It also sets `queue_name` as a side effect, so display code doesn't need to duplicate the logic.

---

#### `collect_arrived()` — The Arrival Gatekeeper (Lines 83–95)

```cpp
static void collect_arrived(std::deque<Process *> &q, int queue_id) {
    for (auto &p : all_processes) {
        if (p.arrival_time <= current_time &&
            p.remaining_time > 0 &&
            classify_queue(p) == queue_id) {
            // Add to queue only if not already present
        }
    }
}
```

**Why scan all_processes every tick?**
Processes arrive at different times. If we built the queue once at the start, we'd include
processes that haven't arrived yet. By checking `arrival_time <= current_time` each tick,
we only add processes that are actually available for scheduling.

**Why the "not already in deque" check?**
Without it, a process could be added multiple times if `collect_arrived` is called while
it's already waiting. The linear scan is fine for the small process counts used here.

---

#### `run_Q1_RR()` — Round Robin (Lines 124–154)

```cpp
int run_for = std::min({QUANTUM, p->remaining_time, time_budget - spent});
// ... execute run_for ticks ...
if (p->remaining_time > 0)
    ready.push_back(p);  // Goes to back of queue → Round Robin
```

**Three-way minimum — why?**
1. `QUANTUM (2)` — RR time slice limit
2. `p->remaining_time` — don't run longer than the process needs
3. `time_budget - spent` — don't overrun this queue's allocated window

All three constraints must be respected simultaneously.

**Why `push_back` instead of `push_front`?**
Round Robin requires the just-preempted process to go to the **back** of the queue,
giving other processes a turn. `push_front` would cause the same process to run forever.

---

#### `run_Q2_SJF()` — Shortest Job First (Lines 159–193)

```cpp
std::sort(ready.begin(), ready.end(),
    [](Process *a, Process *b) {
        return a->remaining_time < b->remaining_time;
    });
Process *p = ready[0];
int run_for = std::min(p->remaining_time, time_budget - spent);
```

**Why sort on every iteration?**
New processes may arrive between ticks. A process with burst_time=1 arriving at tick 5
should jump ahead of one with burst_time=8 that arrived at tick 3.
Re-sorting each iteration ensures we always pick the current shortest job.

**Why non-preemptive?**
The assignment specifies non-preemptive SJF. Once Q2 starts a job, it runs to completion
(or until the budget runs out). Preemptive SJF (SRTF) would be more complex.

---

#### `run_Q3_FCFS()` — First Come First Served (Lines 198–225)

```cpp
for (auto &proc : all_processes) {
    if (proc.arrival_time <= current_time &&
        proc.remaining_time > 0 &&
        classify_queue(proc) == 3) {
        if (!p || proc.arrival_time < p->arrival_time)
            p = &proc;
    }
}
```

**Why linear scan?**
FCFS just picks the earliest-arrived process. We scan all Q3 processes and track the minimum
arrival time. No sorting needed — O(n) is fine for the demo size.

---

#### Main Simulation Loop (Lines 362–368)

```cpp
while (!all_done() && current_time < MAX_TICKS) {
    run_Q1_RR  (BUDGET_Q1);  // Q1 runs for 5 ticks
    if (all_done()) break;
    run_Q2_SJF (BUDGET_Q2);  // Q2 runs for 3 ticks
    if (all_done()) break;
    run_Q3_FCFS(BUDGET_Q3);  // Q3 runs for 2 ticks
}   // Then repeat from Q1
```

**Why `all_done()` checks between queues?**
If all processes finish during Q1's slot, there's no point giving Q2 or Q3 idle cycles.
This prevents the Gantt chart from being padded with unnecessary idle entries.

**Why `MAX_TICKS = 200` safety cap?**
Prevents infinite loops if a bug causes a process to never complete.
With our 9-process set, completion takes ~70 ticks, well under 200.

---

## Task E — `rt_scheduler.cpp`

### Overall Strategy

Real-time scheduling is fundamentally different from general-purpose scheduling:
- Processes have **deadlines** — they must finish by a specific time or the system fails
- Processes are **periodic** — they repeat at fixed intervals (every 5ms, 10ms, etc.)
- The goal is **schedulability** — can we mathematically prove ALL deadlines will be met?

We implement two algorithms and let the math prove their difference.

---

### Section-by-Section Commentary

#### `Task` Struct (Lines 46–62)

```cpp
struct Task {
    int    id;
    double exec_time;         // Ci — how long it takes each period
    double period;            // Ti — how often it arrives
    double deadline;          // Di — must finish by arrival + deadline
    double utilization;       // Ci/Ti — fraction of CPU needed
    double remaining_time;    // tracks progress within current instance
    double absolute_deadline; // current_time + period (recomputed each period)
    bool   ready;             // true when this period's instance is active
    int    static_priority;   // RM only: lower number = higher priority
    int    instances_completed;
    int    deadline_misses;
};
```

**Relative vs Absolute Deadline:**
- **Relative deadline** (Di): "This task must finish within 5ms of arriving"
- **Absolute deadline**: "This task, which arrived at t=10, must finish by t=15"

Every time a task becomes ready (new period), we compute:
`absolute_deadline = current_tick + period`

---

#### `gcd_ll()` and `lcm_ll()` (Lines 76–84)

```cpp
static long long gcd_ll(long long a, long long b) {
    while (b) { a %= b; std::swap(a, b); }
    return a;
}
static long long lcm_ll(long long a, long long b) {
    return (a / gcd_ll(a, b)) * b;  // avoids overflow vs a*b/gcd
}
```

**Why custom GCD/LCM?**
The periods are 5, 10, 20. LCM(5,10,20) = 20 = the hyperperiod.
We write `a / gcd * b` instead of `a * b / gcd` to avoid integer overflow for large periods.

**What is the hyperperiod?**
The LCM of all task periods. After exactly one hyperperiod, the schedule repeats identically.
So simulating one hyperperiod is sufficient to characterize the entire infinite schedule.

---

#### `rm_utilisation_bound()` (Lines 102–105)

```cpp
static double rm_utilisation_bound(int n) {
    return (double)n * (std::pow(2.0, 1.0 / (double)n) - 1.0);
}
```

**The Liu & Layland bound (1973):**
For n tasks, RM can guarantee schedulability if: U ≤ n(2^(1/n) - 1)

| n  | Bound   |
|----|---------|
| 1  | 1.000   |
| 2  | 0.828   |
| 3  | 0.779   |
| ∞  | → ln(2) ≈ 0.693 |

For our 3-task set: U = 0.85 > 0.779 → RM **cannot** guarantee schedulability.
EDF's bound is always 1.0 (100% utilisation).

---

#### `schedulability_check()` (Lines 110–152)

This function runs BEFORE any simulation. It answers the question:
*"Is it even mathematically possible for this algorithm to meet all deadlines?"*

```cpp
bool rm_ok  = (U <= rm_bound);   // 0.85 <= 0.779? NO
bool edf_ok = (U <= edf_bound);  // 0.85 <= 1.0?  YES
```

**This is a critical viva point**: RM says NO but the simulation shows RM actually
meets all deadlines on this task set. Why? Because the Liu & Layland bound is
**sufficient but not necessary** — it's conservative. RM *can* schedule some task sets
with U > 0.779, but it's not *guaranteed* to.

---

#### `activate_tasks()` (Lines 158–167)

```cpp
if (tick % (int)t.period == 0) {
    t.ready          = true;
    t.remaining_time = t.exec_time;
    t.absolute_deadline = tick + t.period;
}
```

**Why modulo?**
At tick 0, all tasks activate. At tick 5, τ1 (period=5) activates again.
At tick 10, τ1 and τ2 (period=10) both activate. At tick 20, all three activate simultaneously.
`tick % period == 0` is the clean mathematical way to express "every N ticks".

---

#### `select_rm()` vs `select_edf()` (Lines 172–195)

```cpp
// RM: pick task with lowest static_priority number (= shortest period = highest priority)
if (!best || t.static_priority < best->static_priority)

// EDF: pick task with smallest absolute_deadline
if (!best || t.absolute_deadline < best->absolute_deadline)
```

**The fundamental difference:**
- **RM** looks at a fixed property (period) set once at startup → static priority
- **EDF** looks at a dynamic property (absolute deadline) that changes every period → dynamic priority

This is why EDF is more powerful: it adapts to the current state of the system, while RM
is locked in to priorities that were optimal on average but may not be optimal right now.

---

#### Simulation Loop (Lines 217–252)

```cpp
for (int tick = 0; tick < hyper_period; tick++) {
    activate_tasks(tasks, tick);         // Step 1: new period arrivals
    // Check deadline misses BEFORE scheduling
    Task *running = select_edf(tasks);   // Step 2: pick winner
    running->remaining_time -= 1.0;      // Step 3: execute 1 tick
    if (running->remaining_time <= 0)
        running->instances_completed++;
}
```

**Tick-by-tick simulation:**
Each iteration of the loop = 1 unit of time. This is a **discrete event simulation** —
we model continuous CPU time as discrete integer ticks. Fine for demo purposes.

**Why check deadline misses BEFORE scheduling?**
If a task's deadline has already passed and it still has remaining work, there's no point
scheduling it — it's already failed. We count the miss, reset the task, and move on.

---

## How All Tasks Connect — OS Concepts Map

```
Task A  ─── IPC (pipe)   ─── PROCESS MANAGEMENT
Task C  ─── pthreads     ─── THREAD MANAGEMENT / CONCURRENCY
Task D  ─── MLQ          ─── CPU SCHEDULING (general purpose)
Task E  ─── RM / EDF     ─── CPU SCHEDULING (real-time)
```

Together, these four tasks cover the three major pillars of an OS course:
1. **Process & IPC** (Task A) — how processes are created and communicate
2. **Threading** (Task C) — parallelism within a process
3. **Scheduling** (Tasks D, E) — how the OS decides who runs next
