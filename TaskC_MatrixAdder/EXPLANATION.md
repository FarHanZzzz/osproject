# 🧮 Task C — Parallel Matrix Adder (ELI5 Edition)

## The Story

Imagine you have two MASSIVE spreadsheets (1000 rows × 1000 columns each). You need to add them together cell by cell. Doing it alone? Takes forever.

So you hire **4 workers**. You split the spreadsheet into 4 horizontal strips:
- Worker 0: "You handle rows 0–249"
- Worker 1: "You handle rows 250–499"
- Worker 2: "You handle rows 500–749"
- Worker 3: "You handle rows 750–999"

They all work **at the same time** (parallel!). Nobody touches anyone else's rows. No arguments, no conflicts, no need for locks. Done in 1/4 the time! 🚀

---

## 🎯 The Big Picture

```
     Thread 0 ──→ rows   0-249  ──→ ┐
     Thread 1 ──→ rows 250-499  ──→ │──→ Complete Matrix C = A + B
     Thread 2 ──→ rows 500-749  ──→ │
     Thread 3 ──→ rows 750-999  ──→ ┘
```

---

## 🔧 How to Run It

```bash
gcc -o matrix_add matrix_adder.c -lpthread
./matrix_add 4     # use 4 workers
./matrix_add 1     # use 1 worker (for comparison)
```

---

## 📖 Code Walkthrough — The Fun Version

### Part 1: The Giant Spreadsheets

```c
#define ROWS 1000
#define COLS 1000

static int A[ROWS][COLS];    // Spreadsheet A
static int B[ROWS][COLS];    // Spreadsheet B
static int C[ROWS][COLS];    // Result: A + B
```

📊 **Why `static` and global?** Each spreadsheet is 1000×1000 × 4 bytes = **4 MB**. Three spreadsheets = 12 MB. The **stack** (where local variables live) is only 8 MB on Linux. Putting 12 MB on an 8 MB stack = 💥 crash! `static` global variables go in a different memory area (data segment) that has no size limit.

---

### Part 2: The Job Description (ThreadArgs)

```c
typedef struct {
    int id;           // "You are Worker 2"
    int start_row;    // "Start at row 500"
    int end_row;      // "Stop before row 750"
} ThreadArgs;
```

📋 **Why do we need a struct?** `pthread_create()` only lets you pass ONE argument. But we need to tell each worker three things (their ID, start row, end row). So we pack everything into a struct and hand the worker a pointer to it.

---

### Part 3: The Worker's Job

```c
void *add_rows(void *arg) {
    ThreadArgs *t = (ThreadArgs *)arg;    // "Let me read my job description"

    for (int i = t->start_row; i < t->end_row; i++) {
        for (int j = 0; j < COLS; j++) {
            C[i][j] = A[i][j] + B[i][j];     // Add one cell
        }
    }

    return NULL;    // "Boss, I'm done!"
}
```

🔒 **Why no locks/mutexes?** Think of it like 4 painters painting 4 different walls. Painter 1 never touches Wall 2. They don't need to take turns or ask permission — they're completely independent. In our code:
- Thread 0 writes C[0..249][*]
- Thread 1 writes C[250..499][*]
- No overlap = no conflict = no lock needed!

**What about reading A and B?** Reading shared data is ALWAYS safe. Only writing to the same spot causes problems. Multiple people can read the same book at the same time without issues.

---

### Part 4: Dividing the Work

```c
int rows_per_thread = ROWS / num_threads;    // 1000 / 4 = 250
int extra_rows      = ROWS % num_threads;    // 1000 % 4 = 0
```

🍕 **Like cutting a pizza:** 1000 rows ÷ 4 threads = 250 rows each, no leftovers.

But what about 1000 ÷ 3 = 333 remainder 1?
- Thread 0: rows 0–332 (333 rows)
- Thread 1: rows 333–665 (333 rows)
- Thread 2: rows 666–999 (**334 rows** — eats the leftover slice!)

```c
if (i == num_threads - 1)
    args[i].end_row += extra_rows;    // Last worker gets the extra slice
```

---

### Part 5: Hiring the Workers

```c
for (int i = 0; i < num_threads; i++) {
    // Fill in the job description
    args[i].id = i;
    args[i].start_row = current_row;
    args[i].end_row = current_row + rows_per_thread;

    // HIRE! Worker starts immediately
    pthread_create(&threads[i], NULL, add_rows, &args[i]);
}
```

🏃 **`pthread_create` is like saying "GO!"** The new thread starts running `add_rows` immediately, in parallel with the main thread. It's like clapping your hands and a worker appears and starts working.

**Arguments:**
- `&threads[i]` — "Here's your employee badge" (thread ID)
- `NULL` — default settings
- `add_rows` — "This is your job"
- `&args[i]` — "Here's your job description"

---

### Part 6: Waiting for Everyone to Finish

```c
for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);    // "Worker i, are you done?"
}
```

⏳ **`pthread_join` = "Wait until this worker finishes."** If we skip this, the main thread might print "Done!" and exit while the workers are still adding numbers. The result would be incomplete garbage.

This is like `waitpid()` in Task A, but for threads instead of processes.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| Thread | A lightweight worker that shares memory with other workers |
| Process (Task A) vs Thread | Process = separate house (own memory). Thread = roommate (shared memory). |
| `pthread_create` | "GO!" — starts a new worker immediately |
| `pthread_join` | "Wait until done" — don't leave before workers finish |
| Mutex | A lock for shared resources. WE DON'T NEED ONE because nobody shares rows! |
| Data Race | Two workers writing to the same cell at the same time = garbage. We avoid it by design. |
| Embarrassingly Parallel | Work that splits perfectly with zero overlap. The dream scenario! |
