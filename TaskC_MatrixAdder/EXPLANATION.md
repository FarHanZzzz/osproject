# 🧮 Task C — Parallel Matrix Adder (ELI5 Edition)

> **Quick Overview:** This program adds two large 1000×1000 matrices (A + B = C) using multiple threads running in parallel. The user can specify the number of threads via the command line. The last thread handles any remainder rows. **Key OS concept: Multithreading with pthreads.**

---

## 🎯 The Big Picture

```
Main Thread (the boss)
  │
  ├── Reads thread count from command line (default: 4)
  ├── Fills matrices A and B with numbers
  │
  ├── pthread_create(thread1)  →  Worker 1: rows   0-332  (333 rows)
  ├── pthread_create(thread2)  →  Worker 2: rows 333-665  (333 rows)
  ├── pthread_create(thread3)  →  Worker 3: rows 666-999  (334 rows ← remainder!)
  │
  ├── pthread_join(all)  ←  Wait for everyone to finish
  └── Verify and print results! 🎉
```

---

## 🔧 How to Run It

```bash
gcc -o matrix_add matrix_adder.c -lpthread

# Default (4 threads):
./matrix_add

# Custom thread count:
./matrix_add 8

# Performance testing:
time ./matrix_add 1    # Single-threaded baseline
time ./matrix_add 4    # 4 threads
time ./matrix_add 8    # 8 threads
```

---

## 📖 Code Walkthrough

### Part 1: Dynamic Threading (Enhancement)

```c
int num_threads = 4;  // default

if (argc > 1) {
    num_threads = atoi(argv[1]);  // Read from command line
    if (num_threads < 1) num_threads = 1;
    if (num_threads > MAX_THREADS) num_threads = MAX_THREADS;
}
```

🎛️ **The user controls the thread count** via command-line argument. `./matrix_add 8` creates 8 threads. This allows easy performance testing — compare 1 thread vs 4 vs 8.

---

### Part 2: Remainder Row Handling (Enhancement)

```c
int rows_per_thread = ROWS / num_threads;   // 1000 / 3 = 333
int remainder = ROWS % num_threads;         // 1000 % 3 = 1

for (int i = 0; i < num_threads; i++) {
    args[i].start_row = current_row;
    if (i == num_threads - 1)
        args[i].end_row = ROWS;      // Last thread gets remainder
    else
        args[i].end_row = current_row + rows_per_thread;
    current_row = args[i].end_row;
}
```

📐 **What if rows aren't evenly divisible?** 1000 ÷ 3 = 333 remainder 1. The last thread handles 334 rows (333 + 1 leftover). This ensures ALL rows are covered, no matter the thread count.

---

### Part 3: Thread Safety Without Locks

Each thread works on **completely different rows**:
- Thread 1: rows 0–332
- Thread 2: rows 333–665
- Thread 3: rows 666–999

No two threads ever write to the same memory address. This is called **disjoint memory access** — it means we need **zero locks/mutexes**!

---

### Part 4: Full Verification

```c
int correct = 1;
for (int i = 0; i < ROWS && correct; i++)
    for (int j = 0; j < COLS && correct; j++)
        if (C[i][j] != 3) correct = 0;
printf("Full verification: %s\n", correct ? "ALL CORRECT" : "ERRORS FOUND");
```

We verify every single element, not just a few spot checks. Since A=1 and B=2, every C[i][j] should be 3.

---

### Part 5: Performance Testing (Enhancement)

```bash
$ time ./matrix_add 1
real    0m0.015s    # Single-threaded

$ time ./matrix_add 4
real    0m0.008s    # 4 threads — nearly 2x faster!

$ time ./matrix_add 8
real    0m0.006s    # 8 threads — diminishing returns
```

Use the `time` command to compare. More threads help up to the number of CPU cores; beyond that, gains diminish due to scheduling overhead.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| Thread | A lightweight worker that shares memory with the main program. |
| Process | A heavyweight worker with its own separate memory. |
| `pthread_create()` | Hire a new worker and give them a job and instructions. |
| `pthread_join()` | Boss waits for the worker to finish. |
| Race Condition | Two threads writing to the same spot. Result = chaos. |
| Disjoint Access | Each thread writes to different rows. Zero conflicts = zero locks. |
| Remainder Handling | Last thread handles leftover rows when ROWS % threads ≠ 0. |
| Dynamic Threading | Thread count set at runtime via command-line argument. |
| `-lpthread` | Compiler flag to link the pthreads library. Required! |
| Data Parallelism | Splitting data (rows) across workers. Same operation, different data. |
