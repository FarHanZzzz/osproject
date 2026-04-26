# Task C: Parallel Matrix Adder — The Viva Defense Walkthrough

Imagine you're walking into the viva. The examiner asks: *"How did you approach Task C, and why did you write the code this way?"*

This document is your exact thought process, from reading the question for the very first time to writing the final line of code.

---

## Step 1: Reading the Question & Finding the Prerequisites
When I first saw the PDF for Task C, the prompt asked to:
1. Add two **large matrices** (A + B = C).
2. Use **multiple threads** (pthreads) to speed up the computation.
3. Each thread should handle a **portion of the rows** independently.

**My immediate thought process:**
"Matrix addition is trivially simple: `C[i][j] = A[i][j] + B[i][j]`. The real challenge isn't the math — it's splitting the work across threads and proving I understand *why* this is faster and *why* it's safe."

**Prerequisites I had to learn before coding:**
1.  **What is a thread?** → A lightweight worker inside a single process. Unlike `fork()` (Task A) which creates a *separate* process with its own memory, threads share the SAME memory space.
2.  **How do I create threads in C?** → `pthread_create()` from `<pthread.h>`.
3.  **How do I wait for them?** → `pthread_join()` — similar to `waitpid()` for processes.
4.  **Do I need locks/mutexes?** → NO! Each thread writes to *different rows*. No conflict = no lock needed.

---

## Step 2: The Mental Blueprint (ELI5)
*   **Me (The Main Thread):** I'm a manager with a giant 1000×1000 spreadsheet.
*   **The Delegation:** I hire 4 workers (threads). I rip the spreadsheet into 4 strips of 250 rows each.
*   **The Magic:** All 4 workers share the same spreadsheet (shared memory!). They each do their rows simultaneously.
*   **The Wait:** I sit at my desk until all 4 workers say "Done!" (`pthread_join`).

**Why faster?** On a 4-core CPU, all 4 threads run physically at the same time. ~4× speedup!

---

## Step 3: Writing the Code (Line-by-Line Rationale)

### 1. The Headers
```c
#include <stdio.h>    // For printf
#include <stdlib.h>   // General utilities
#include <pthread.h>  // POSIX threading — pthread_create, pthread_join
```
*Compile flag:* Must add `-lpthread`: `gcc -o matrix_add matrix_adder.c -lpthread`

### 2. Global Arrays
```c
#define ROWS 1000
#define COLS 1000
int A[ROWS][COLS], B[ROWS][COLS], C[ROWS][COLS];
```
*Why global?* (1) Each matrix is ~4MB — too large for the stack. (2) Global = automatically visible to ALL threads.

### 3. The Thread Argument Struct
```c
typedef struct {
    int start_row;  // inclusive
    int end_row;    // exclusive
} ThreadArgs;
```
`pthread_create` only lets us pass ONE `void *` argument, so we pack row info into a struct.

### 4. The Thread Function
```c
void *add_rows(void *arg) {
    ThreadArgs *t = (ThreadArgs *)arg;
    for (int i = t->start_row; i < t->end_row; i++)
        for (int j = 0; j < COLS; j++)
            C[i][j] = A[i][j] + B[i][j];
    return NULL;
}
```
Each thread ONLY loops through its assigned rows. They never overlap → no race condition.

### 5. Launching & Joining Threads
```c
ThreadArgs args1 = {0, 250}, args2 = {250, 500}, args3 = {500, 750}, args4 = {750, 1000};
pthread_create(&thread1, NULL, add_rows, &args1);  // ... ×4
pthread_join(thread1, NULL);  // ... ×4  — wait for all to finish
```
`pthread_join` is the thread equivalent of `waitpid()`. Without it, `main()` could print results before threads finish.

### 6. Verification
We fill A with 1s, B with 2s. Every C[i][j] should be 3. We spot-check `C[0][0]`, `C[500][500]`, `C[999][999]`.

---

## Step 4: Problems Faced & "What Ifs" (Viva Goldmine)

### 1. "Why No Mutex/Lock?"
**Answer:** Race conditions happen when threads write to the SAME location. Here, Thread 1 writes rows 0–249, Thread 2 rows 250–499, etc. — no overlap. This is an "embarrassingly parallel" problem.

### 2. "Why 4 Threads? Why Not 1000?"
**Answer:** Thread creation has overhead. 1000 threads for 1000 rows = more scheduling overhead than benefit. 4 threads matches a typical 4-core CPU.

### 3. "How Is This Different From Task A (fork)?"

| Feature | Task A (`fork`) | Task C (`pthread`) |
|---|---|---|
| Creates | Separate process | Thread (same process) |
| Memory | Completely separate | Shared globals |
| Communication | Needs pipes (IPC) | Direct memory access |
| Overhead | Heavy | Light |

### 4. "What If Rows Don't Divide Evenly?"
**Answer:** Give the last thread the remainder: `{750, 1001}`. Here 1000÷4=250 exactly, so no issue.

### 5. "Could You Use fork() Instead?"
**Answer:** Yes, but each child gets its own COPY of the 12MB matrices. You'd need IPC to send results back. Threads are the natural fit because they share memory.
