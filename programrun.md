# How to Run Each Program
## CSC413/CSE315 — OS Programming Assignment, Spring 2026

> **Prerequisites:** A Linux terminal, `gcc`, `g++`, and `make` installed.
> Check with: `gcc --version && g++ --version && make --version`

---

## 🔧 First-Time Setup (Run Once)

```bash
# Navigate to the project root
cd ~/Downloads/Osproject

# Verify the folder structure
ls -1
# Expected output:
# TaskA_WordCounter/
# TaskB_ElusiveCursor/
# TaskC_MatrixAdder/
# TaskD_MLQScheduler/
# TaskE_RealTimeScheduler/
# OptionalA_KernelModule/
# OPTIONAL_SECTION_PLAN.md
```

---

## Task A — Word Counter

### Step 1: Build
```bash
cd ~/Downloads/Osproject/TaskA_WordCounter
make
```
Expected output:
```
gcc -Wall -Wextra -std=c11 -g -o word_counter word_counter.c
```

### Step 2: Run — Count words in a single file
```bash
./word_counter word_counter.c
```
Expected output:
```
=== Word Counter — fork+pipe IPC ===
Counting words in 1 file(s)...

  word_counter.c                               816 word(s)

----------------------------------------
Grand Total: 816 word(s)
========================================
```

### Step 3: Run — Count words in multiple files
```bash
./word_counter word_counter.c Makefile
```

### Step 4: Run — Test error handling (missing file)
```bash
./word_counter word_counter.c ghost.txt Makefile
```
Expected: ghost.txt shows `ERROR (file not found or unreadable)`, other files counted normally.

### Step 5: Run built-in test suite
```bash
make test
```

### Step 6: Clean build artifacts
```bash
make clean
```

### Quick Verification Against `wc -w`
```bash
wc -w word_counter.c Makefile
# Your program's totals should match exactly
```

---

## Task B — Elusive Cursor (Windows Only)

> ⚠️ **This task requires Windows + MinGW or MSVC. Cannot run on Linux.**

```cmd
# On Windows, open a terminal in TaskB_ElusiveCursor/
gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32
elusive_cursor.exe

# To kill: Press Ctrl+Shift+Q (the registered hotkey)
```

---

## Task C — Multi-threaded Matrix Adder

### Step 1: Build
```bash
cd ~/Downloads/Osproject/TaskC_MatrixAdder
make
```
Expected output:
```
gcc -Wall -Wextra -std=c11 -O2 -D_POSIX_C_SOURCE=199309L -o matrix_add matrix_adder.c -lpthread
```

### Step 2: Run — 1 thread (baseline)
```bash
./matrix_add 1
```
Expected output:
```
=== Parallel Matrix Adder (1000x1000) ===
Threads requested: 1

  [Thread  0] Processing rows    0 –  999

Verifying result... CORRECT ✓

Sample output C[0][0..4]:     0     1     2     3     4
...
  Wall time    : ~16 ms
```

### Step 3: Run — 4 threads (parallel)
```bash
./matrix_add 4
```

### Step 4: Run — Remainder row test (1000 / 3)
```bash
./matrix_add 3
# Last thread gets 1 extra row (334 instead of 333)
```

### Step 5: Run — Error case (too many threads)
```bash
./matrix_add 1001
# Expected: ERROR: num_threads must be between 1 and 1000.
```

### Step 6: Performance benchmark
```bash
make perf
# Runs 1, 2, 4, 8 threads and shows timing for each
```
Or manually:
```bash
time ./matrix_add 1
time ./matrix_add 4
```

### Step 7: Clean
```bash
make clean
```

---

## Task D — MLQ Scheduler

### Step 1: Build
```bash
cd ~/Downloads/Osproject/TaskD_MLQScheduler
make
```
Expected output:
```
g++ -Wall -Wextra -std=c++17 -O2 -o mlq_scheduler mlq_scheduler.cpp
```

### Step 2: Run
```bash
./mlq_scheduler
```
The program runs immediately with 9 built-in processes and prints:
- Input process table (PID, Arrival, Burst, Priority, Queue)
- Full Gantt chart (per tick)
- Process metrics table (Completion, Turnaround, Waiting, Response Times)
- Average statistics

No arguments needed — everything is hardcoded in `main()`.

### Step 3: Clean
```bash
make clean
```

### Sample Expected Output (first few lines)
```
=== Time-Sliced MLQ Scheduler ===
  Q1 (System,      priority  0-10): Round Robin (quantum=2), 50% CPU
  Q2 (Interactive, priority 11-20): SJF (non-preemptive),    30% CPU
  Q3 (Batch,       priority 21-30): FCFS,                    20% CPU

Input Processes:
  PID  Arrival  Burst  Priority              Queue
-------------------------------------------------
    1        0      6         5          Q1-System
  ...
```

---

## Task E — Real-Time Scheduler

### Step 1: Build
```bash
cd ~/Downloads/Osproject/TaskE_RealTimeScheduler
make
```
Expected output:
```
g++ -Wall -Wextra -std=c++17 -O2 -o rt_scheduler rt_scheduler.cpp
```

### Step 2: Run — Full simulation (RM + EDF)
```bash
./rt_scheduler
```
The program will:
1. Print the schedulability analysis table
2. Run RM simulation and print compact Gantt chart
3. Run EDF simulation and print compact Gantt chart
4. Ask: `Run again with custom mode? (RM/EDF/no):`

### Step 3: Run RM mode only
```bash
echo "no" | ./rt_scheduler
# Pipes "no" to skip the interactive prompt
```
Or interactively:
```bash
./rt_scheduler
# At prompt, type: RM   or   EDF   or   no
```

### Step 4: Run EDF mode with custom prompt
```bash
echo "EDF" | ./rt_scheduler
```

### Step 5: Clean
```bash
make clean
```

### Key things to observe in output
```
RM  schedulable : NO  ✗  (U exceeds bound)   ← U=0.85 > 0.7798
EDF schedulable : YES ✓                        ← U=0.85 < 1.0
✓ All deadlines met!                           ← EDF meets all deadlines
```

---

## Build Everything at Once

```bash
cd ~/Downloads/Osproject

# Build all tasks in one go
for task in TaskA_WordCounter TaskC_MatrixAdder TaskD_MLQScheduler TaskE_RealTimeScheduler; do
  echo "=== Building $task ==="
  (cd $task && make)
  echo ""
done
```

## Clean Everything at Once

```bash
cd ~/Downloads/Osproject
for task in TaskA_WordCounter TaskC_MatrixAdder TaskD_MLQScheduler TaskE_RealTimeScheduler; do
  (cd $task && make clean)
done
```

## Create the Submission ZIP

```bash
cd ~/Downloads

# Replace with your actual section, ID, and name
zip -r "3_XXXXXXX_CSC413_YourName.zip" Osproject/ \
  --exclude "Osproject/.git/*"

ls -lh 3_XXXXXXX_CSC413_YourName.zip
```
