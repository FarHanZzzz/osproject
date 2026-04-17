# Task C — Multi-threaded Matrix Adder
## How the Code Works — Complete Explanation

> **Source:** `matrix_adder.c` | **Language:** C | **OS Concepts:** pthreads, concurrency, data partitioning, race conditions

---

## What This Program Does

This program adds two large 1000x1000 matrices ($A + B = C$) in parallel. It splits the workload across multiple threads, where each thread is responsible for computing a specific chunk of rows. 

```
$ ./matrix_add 4
=== Parallel Matrix Adder (1000x1000) ===
Threads requested: 4

  [Thread  0] Processing rows    0 –  249
  [Thread  1] Processing rows  250 –  499
  [Thread  2] Processing rows  500 –  749
  [Thread  3] Processing rows  750 –  999

Verifying result... CORRECT ✓
```

---

## How It Works — Step by Step

### 1. Global Matrix Allocation (Lines 47–50)

```c
static int A[ROWS][COLS];
static int B[ROWS][COLS];
static int C[ROWS][COLS];
static int C_ref[ROWS][COLS];
```

The matrices are 1000x1000 integers. That's $1,000,000 \times 4$ bytes $\approx 4$ MB per matrix.
We declare them as `static` globally to allocate them in the data segment (BSS) rather than on the stack. If they were local variables in `main()`, they would immediately cause a **stack overflow** since the default Linux stack size is usually 8 MB.

### 2. The Worker Function (Lines 70–84)

```c
static void *thread_add_rows(void *arg) {
    ThreadArgs *ta = (ThreadArgs *)arg;
    for (int i = ta->start_row; i < ta->end_row; i++) {
        for (int j = 0; j < COLS; j++) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
    pthread_exit(NULL);
}
```

This is the function every thread runs. It receives a `ThreadArgs` pointer that tells it exactly which rows to process. It iterates over its assigned rows (`start_row` to `end_row - 1`) and adds the elements. 

### 3. Calculating the Work Chunks (Lines 156–168)

```c
int chunk      = ROWS / num_threads;
int remainder  = ROWS % num_threads;
```

We divide the 1000 rows by the number of threads. 
If we ask for 3 threads: `1000 / 3 = 333` rows per thread, with a remainder of `1`.
Thread 0 gets rows 0-332. Thread 1 gets rows 333-665. The **last thread** gets the base chunk *plus* the remainder (rows 666-999). 

### 4. Creating Threads (Lines 183–199)

```c
for (int t = 0; t < num_threads; t++) {
    args[t].thread_id = t;
    args[t].start_row = current_row;
    args[t].end_row   = current_row + chunk + (t == num_threads - 1 ? remainder : 0);
    current_row       = args[t].end_row;

    pthread_create(&threads[t], NULL, thread_add_rows, &args[t]);
}
```

We fill out a `ThreadArgs` struct for each thread so they know their boundaries. Then we call `pthread_create()`, passing the `thread_add_rows` function and the specific argument struct. 

### 5. Waiting for Completion (Lines 201–208)

```c
for (int t = 0; t < num_threads; t++) {
    pthread_join(threads[t], NULL);
}
```

`pthread_join()` pauses the main thread until the specified worker thread finishes. We loop through all threads and join them. Without this, the main program might exit before the threads finish their work, killing them prematurely.

### 6. Verification (Lines 106–120)

```c
static int verify_result(void) {
    // Computes A+B single-threaded into C_ref, then compares C_ref with C
}
```

Parallel programming is tricky. A simple off-by-one error in the row boundaries might cause some rows to be skipped or overwritten. We compute the exact answer on a single thread and compare the entire $1000 \times 1000$ matrix to ensure our threaded logic was perfect.

---

## Key OS Concepts

| Concept | Where in Code | What It Does |
|---------|---------------|--------------|
| `pthread_create()` | Line 191 | Spawns a new OS-level thread within the same process. |
| `pthread_join()` | Line 203 | Synchronises the main thread with worker threads, acting like `waitpid()` for threads. |
| **Data Partitioning** | Lines 184–188 | Dividing a large dataset into disjoint chunks so threads can work independently. |
| **Shared Memory** | Lines 47–49 | All threads access the same `A`, `B`, and `C` arrays in memory. |

---

## Test Results (Verified)

| Test | Input | Expected Output | Status |
|------|-------|-----------------|--------|
| Baseline | `1` thread | Processes rows 0-999. CORRECT. | ✅ PASS |
| Even split | `4` threads | 250 rows each. CORRECT. | ✅ PASS |
| Uneven split | `3` threads | Threads get 333, 333, 334 rows. CORRECT. | ✅ PASS |
| Max threads | `1000` threads | 1 row each. CORRECT. | ✅ PASS |
| Error handling | `1001` threads | Error message, prevents creation. | ✅ PASS |

---

## Viva Questions & Answers

**Q: Why didn't you use mutex locks (`pthread_mutex_t`) in this program?**
A: Because of our data partitioning strategy. A race condition only occurs if two threads write to the same memory location at the same time. Since we give every thread a strictly disjoint, non-overlapping range of rows, no two threads ever write to the same `C[i][j]`. Reads (from `A` and `B`) can happen concurrently without issues. Thus, mutexes are completely unnecessary and omitting them maximizes parallel speed.

**Q: Why do we pass a struct to the thread instead of just global variables?**
A: `pthread_create` only allows us to pass a single `void *` pointer. Since each thread needs distinct information (its thread ID, start row, and end row), we pack that data into a `ThreadArgs` struct and pass a pointer to that struct. If we used global variables for `start_row`, threads would overwrite each other's boundaries before they even started running.

**Q: What is the difference between `pthread_exit()` and returning from the function?**
A: In standard worker threads, they behave similarly. However, `pthread_exit()` is explicitly designed to terminate a thread and can trigger thread-specific cleanup handlers. It is the canonical POSIX way to end a thread's execution.

**Q: Why use `clock_gettime(CLOCK_MONOTONIC)` instead of the standard `time()`?**
A: `time()` measures in whole seconds, which is useless for operations that take milliseconds. `CLOCK_MONOTONIC` provides nanosecond resolution. Also, unlike the wall clock (`CLOCK_REALTIME`), the monotonic clock is guaranteed never to jump backwards (e.g., if the system time synchronizes with an NTP server during the test), ensuring accurate elapsed time measurements.
