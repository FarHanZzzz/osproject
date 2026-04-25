# 🧮 Task C — Parallel Matrix Adder (ELI5 Edition)

> **Quick Overview:** This program adds two large 1000×1000 matrices (A + B = C) using 4 threads running in parallel. Each thread handles 250 rows. Because they work on different rows, no locks are needed. **Key OS concept: Multithreading with pthreads.**

---

Imagine you have a MASSIVE spreadsheet — 1000 rows and 1000 columns. You need to add two of these spreadsheets together, cell by cell (A + B = C). That's **1,000,000 additions!**

Doing it alone would take forever. So you hire **4 workers** and divide the spreadsheet into 4 equal strips:
- Worker 1 handles rows 0–249
- Worker 2 handles rows 250–499
- Worker 3 handles rows 500–749
- Worker 4 handles rows 750–999

All 4 workers start at the same time and work **in parallel**. When they're all done, the result matrix C is complete!

That's exactly what this program does:
- **Workers** = threads (created with `pthread_create()`)
- **The spreadsheet** = a global 2D array that all workers share
- **Waiting for everyone to finish** = `pthread_join()`

---

## 🎯 The Big Picture

```
Main Thread (the boss)
  │
  ├── Fills matrices A and B with numbers
  │
  ├── pthread_create(thread1)  →  Worker 1: rows   0-249  →  C[i][j] = A[i][j] + B[i][j]
  ├── pthread_create(thread2)  →  Worker 2: rows 250-499  →  C[i][j] = A[i][j] + B[i][j]
  ├── pthread_create(thread3)  →  Worker 3: rows 500-749  →  C[i][j] = A[i][j] + B[i][j]
  ├── pthread_create(thread4)  →  Worker 4: rows 750-999  →  C[i][j] = A[i][j] + B[i][j]
  │
  ├── pthread_join(all 4)  ←  Wait for everyone to finish
  │
  └── Print results! 🎉
```

All 4 workers are running at the **exact same time** on different CPU cores. This is **true parallelism**.

---

## 🔧 How to Run It

```bash
gcc -o matrix_add matrix_adder.c -lpthread
./matrix_add
```

**Note:** The `-lpthread` flag is REQUIRED. Without it, the linker can't find `pthread_create()` and the build fails.

---

## 📖 Code Walkthrough — The Fun Version

### Part 1: Threads vs. Processes

🏠 **Processes (fork) = Building a new house.** Each process gets its own separate memory. To communicate, you need pipes, shared memory, or other IPC mechanisms. Heavy and slow to create.

🛏️ **Threads (pthread) = Adding a roommate.** All threads share the SAME memory (same global variables, same heap). No pipes needed — they can read/write the same variables directly. Lightweight and fast to create.

**Why use threads here?** Because matrices A, B, and C are huge global arrays. If we used `fork()`, each child would get its own COPY of these arrays (4 million integers × 4 copies = 64MB of wasted RAM). With threads, all 4 workers share the same arrays — zero copies needed!

---

### Part 2: Global Matrices

```c
int A[ROWS][COLS];  // 1000x1000 = 1,000,000 integers = ~4MB
int B[ROWS][COLS];
int C[ROWS][COLS];  // Result matrix
```

📦 **Why global and not local?** Two reasons:

1. **Size limit:** Local variables live on the **stack**, which is typically only 8MB. Three 4MB arrays = 12MB = stack overflow crash! Global variables live in the **data segment**, which can be much larger.

2. **Thread sharing:** Global variables are automatically visible to all threads. If these were local to `main()`, we'd have to pass pointers to each thread (still doable, but messier).

---

### Part 3: The Sticky Note (ThreadArgs)

```c
typedef struct {
    int start_row;
    int end_row;
} ThreadArgs;

ThreadArgs args1 = {0,   250};   // "Worker 1, do rows 0 to 249"
ThreadArgs args2 = {250, 500};   // "Worker 2, do rows 250 to 499"
ThreadArgs args3 = {500, 750};   // "Worker 3, do rows 500 to 749"
ThreadArgs args4 = {750, 1000};  // "Worker 4, do rows 750 to 999"
```

📝 **Each `ThreadArgs` is a sticky note** that the boss hands to a worker: "Your job is rows X through Y. Don't touch anyone else's rows." Because each worker works on completely DIFFERENT rows, they never interfere with each other. This means we need **zero locks/mutexes** — the simplest possible parallel program!

**What's `end_row` — inclusive or exclusive?** It's exclusive. `{0, 250}` means rows 0, 1, 2, ..., 249 (250 rows total). The loop uses `i < end_row`.

---

### Part 4: Launching the Workers

```c
pthread_create(&thread1, NULL, add_rows, &args1);
```

🚀 **Breaking down the arguments:**
- `&thread1` — Where to store the thread's handle (ID), so we can `join` it later
- `NULL` — Use default thread settings (stack size, etc.)
- `add_rows` — The function this thread will execute
- `&args1` — The argument to pass to that function (our sticky note)

**What happens inside the OS?** The kernel creates a new execution context (registers, stack pointer) that shares the same address space as the main thread. The new thread starts executing `add_rows()` immediately — potentially on a different CPU core!

---

### Part 5: The Work Function

```c
void *add_rows(void *arg) {
    ThreadArgs *t = (ThreadArgs *)arg;

    for (int i = t->start_row; i < t->end_row; i++) {
        for (int j = 0; j < COLS; j++) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }

    return NULL;
}
```

🔨 **This is the "job" each worker performs.** All 4 workers are running this function at the same time, but each has a different `ThreadArgs` telling them which rows to handle.

**Why `void *arg` instead of `ThreadArgs *arg`?** `pthread_create()` requires the function to accept `void *` (a generic pointer). This is C's way of saying "I'll accept any type of data." Inside the function, we cast it back to `ThreadArgs *` because we know what it actually is.

**Why `return NULL`?** The function signature requires returning `void *`. Since we don't need to return any data (the result is already in the global matrix C), we just return NULL.

---

### Part 6: Waiting for Everyone

```c
pthread_join(thread1, NULL);
pthread_join(thread2, NULL);
pthread_join(thread3, NULL);
pthread_join(thread4, NULL);
```

⏳ **`pthread_join` = "Boss waits at the door until this worker walks out."** The main thread pauses here until the specified thread finishes its work. We call it 4 times because we have 4 workers.

**Why is this necessary?** Without `join`, the main thread might reach `return 0` and exit the entire program before the workers finish their math! The result matrix C would be incomplete.

**What's the second argument (NULL)?** It's where we could receive the thread's return value. Since our threads return NULL, we pass NULL here too — we don't care about the return value.

---

### Part 7: Why No Locks?

🔒 **The #1 danger of threads is a "Race Condition."** Imagine two workers both trying to write to `C[500][500]` at the same time. Who wins? Nobody knows! The result is unpredictable garbage.

**But we don't have this problem!** Because:
- Worker 1 ONLY touches rows 0–249
- Worker 2 ONLY touches rows 250–499
- Worker 3 ONLY touches rows 500–749
- Worker 4 ONLY touches rows 750–999

No two workers ever touch the same cell. Zero conflicts = zero need for locks = maximum speed!

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| Thread | A lightweight worker that shares memory with the main program. Like a roommate — same house, different tasks. |
| Process | A heavyweight worker with its own separate memory. Like a neighbor — different house entirely. |
| `pthread_create()` | Hire a new worker and give them a job (function) and instructions (args). |
| `pthread_join()` | Boss waits at the door for the worker to finish. Prevents premature exit. |
| Race Condition | Two threads writing to the same spot at the same time. Result = chaos. |
| Mutex / Lock | A "bathroom occupied" sign. Only one thread can enter the critical section at a time. (We don't need one here!) |
| `-lpthread` | The compiler flag that links the pthreads library. Without it, the program won't compile. |
| Global variables | Variables outside any function. All threads can see them. Perfect for shared data like matrices. |
| Stack overflow | Local arrays that are too big for the stack (default ~8MB). Solution: make them global. |
| Data parallelism | Splitting data (rows) across workers. Each worker does the same operation on different data. |
