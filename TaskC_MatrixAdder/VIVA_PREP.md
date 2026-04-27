# Task C — Parallel Matrix Adder: Viva Preparation

---

## 🧠 What Is The Problem?

The assignment asks: *"Add two large matrices A and B together (C = A + B) using multiple threads, where each thread handles a specific set of rows."*

The twist: a 1000×1000 matrix means 1,000,000 additions. Doing this in one thread is slow. By splitting the rows across multiple threads, all threads run **simultaneously** on different CPU cores.

This tests your understanding of:
- What threads are and how they differ from processes
- How to safely share memory between threads
- How to measure the speedup from parallelism

---

## 💬 How To Explain The Problem (Your Opening Line)

> *"The problem is to add two 1000×1000 matrices in parallel using multiple threads. Each thread is assigned a specific chunk of rows and works independently. No two threads touch the same memory location, so no locking is needed, which makes it fast."*

---

## ⚙️ How Your Code Solves It — Step By Step

### Step 1 — Dynamic Thread Count
- User runs `./matrix_add 4` → 4 threads
- Default is 4 if no argument given
- `atoi(argv[1])` converts the string "4" to integer 4
- Capped at `MAX_THREADS = 16` for safety

### Step 2 — Fill Matrices With Test Data
- A = all 1s, B = all 2s
- This means every cell of C should be 3 — easy to verify!

### Step 3 — Calculate Row Partitions (Remainder Handling)
- `rows_per_thread = ROWS / num_threads` (integer division)
- `remainder = ROWS % num_threads` (leftover rows)
- Example: 1000 rows / 3 threads = 333 rows each, with 1 leftover
- The LAST thread always gets `rows_per_thread + remainder` rows

### Step 4 — Launch All Threads
- `pthread_create(&threads[i], NULL, add_rows, &args[i])`
- Each thread gets a `ThreadArgs` struct telling it its start_row and end_row
- All threads start nearly simultaneously

### Step 5 — Wait For All Threads
- `pthread_join(threads[i], NULL)` — waits for thread i to finish
- Without this, main() might print results before threads are done computing

### Step 6 — Verify Results
- Spot-check `C[0][0]`, `C[500][500]`, `C[999][999]` — all should be 3
- Full scan of every cell confirms correctness

---

## 🔥 Challenges & What You'd Say You Faced

> *"The first challenge was understanding why the matrices are declared globally instead of inside main(). A 1000×1000 integer matrix is 4MB. The default thread stack is only 8MB. Putting three such matrices in main() would risk a stack overflow crash. Global variables live in the data segment, which has much more space."*

> *"The second challenge was figuring out the row distribution. When thread count doesn't divide evenly into the row count, you get a remainder. For example, 1000 / 3 = 333 remainder 1. If you just give every thread 333 rows, one row is never processed. My solution: the last thread always gets everything from its start_row to ROWS (the actual end), which automatically absorbs any remainder."*

> *"I also considered whether I needed mutex locks. The answer is no — because each thread writes to a completely separate set of rows. Thread A writes rows 0–332, Thread B writes rows 333–665. They never write to the same memory address. So there's zero chance of a data race."*

---

## 🤔 Choices You Made & Why

| Choice | Why |
|--------|-----|
| Global arrays A, B, C | Too large for stack; globals live in data segment with no size limit |
| No mutexes/locks | Threads write to non-overlapping rows — no shared write targets = no race condition |
| `ThreadArgs` struct | Clean way to pass multiple arguments to a thread (pthread_create only takes one arg) |
| Last thread handles remainder | Simple, guaranteed to cover all rows without skipping any |
| 1000×1000 matrix size | Large enough to show meaningful speedup in performance tests |
| `pthread_join()` | Ensures main() waits for all results before verifying or printing |

---

## 📚 Concepts You Need To Know

### Thread vs Process
| Thread | Process |
|--------|---------|
| Runs inside a process | Standalone OS unit |
| Shares memory with siblings | Has own private memory |
| Lighter to create (faster) | Heavier to create (slower) |
| Communicate via shared variables | Communicate via pipes/IPC |
| Need locks for shared writes | No accidental sharing |

> Threads are like workers in the same office sharing the same desk and files.
> Processes are like workers in separate offices with their own desks.

### `pthread_create()`
- Creates a new thread that runs a specific function
- `pthread_create(&tid, NULL, function, arg)`:
  - `tid` = thread ID (output)
  - `NULL` = default attributes
  - `function` = the function the thread will execute
  - `arg` = single argument passed to the function (must be `void*`)

### `pthread_join()`
- Makes the calling thread wait until the target thread finishes
- Like `waitpid()` but for threads
- Without it: main() exits while threads are still running → undefined behavior

### Data Race / Race Condition
- Happens when two threads access the same memory, at least one writes, and there's no synchronization
- Example: Thread A and Thread B both try to write `C[5][5]` — which value wins?
- **Why our code has NO race condition**: Thread A writes rows 0–249, Thread B writes rows 250–499. They NEVER overlap.

### Mutex (What We DON'T Need Here)
- A mutex is a lock: only one thread can hold it at a time
- Used when threads share write targets
- We don't use one because each thread owns exclusive rows

### Data Partitioning (From the PDF)
- The PDF specifically requires:
  - **Data Partitioning**: Thread A only touches rows 0–9, Thread B only rows 10–19, etc.
  - **Independent Writes**: No two threads write to the same address
  - **Read-Only Inputs**: A and B are never modified by threads, only read

### Performance Testing
- `time ./matrix_add 1` → runs with 1 thread, shows "real" time
- `time ./matrix_add 4` → runs with 4 threads, should be ~4x faster
- Real speedup depends on CPU cores and memory bandwidth

---

## ✅ Quick Viva Q&A

**Q: Why don't you need a mutex in this program?**
> Because each thread writes to a completely separate range of rows in matrix C. Thread 1 writes rows 0–249, Thread 2 writes rows 250–499, and so on. Two threads never write to the same memory address, so there is no possibility of a data race.

**Q: What would happen if you removed `pthread_join()`?**
> The main function would reach the verification code before threads finish computing. You'd be reading garbage values from matrix C — whatever happened to be in memory before the threads wrote anything.

**Q: Why are the matrices global and not local variables in main?**
> Three 1000×1000 int arrays = 3 × 4MB = 12MB total. The default stack size is typically 8MB. Declaring them locally in main would overflow the stack and crash the program. Global variables are stored in the data segment, which can handle this size.

**Q: What is the ThreadArgs struct used for?**
> `pthread_create` only lets you pass ONE argument to a thread function (as a `void*`). We need to pass multiple values: which thread it is, what row to start at, and what row to end at. We pack all these into a struct, and the thread unpacks it.

**Q: How does the remainder row handling work?**
> Integer division: 1000 / 3 = 333, remainder 1. The first two threads get 333 rows each. Instead of also giving the last thread 333 rows (which would skip row 999), we set its `end_row = ROWS` (= 1000), so it automatically gets rows 666–999 = 334 rows, covering the remainder.
