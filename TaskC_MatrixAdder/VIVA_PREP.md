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
- `workers.emplace_back(add_rows, args);`
- Each thread gets a `WorkerArgs` struct telling it its start_row and end_row
- All threads start nearly simultaneously

### Step 5 — Wait For All Threads
- `w.join()` — waits for each worker thread to finish
- Without this, main() might print results before threads are done computing

### Step 6 — Verify Results
- Spot-check `C[0][0]`, `C[500][500]`, `C[999][999]` — all should be 3
- Full scan of every cell confirms correctness

---

## 🔥 Challenges & What You'd Say You Faced

> *"The first challenge was memory management. An 800×800 integer matrix is large. The default thread stack is limited (e.g., 8MB on Windows/Linux). If we allocated these arrays directly on the stack inside main, it could cause a stack overflow. Instead, we used `std::vector`, which safely allocates memory dynamically on the heap. Since all threads run in the same process, they share the heap and can access the matrices via pointers."*

> *"The second challenge was figuring out the row distribution. When thread count doesn't divide evenly into the row count, you get a remainder. For example, 1000 / 3 = 333 remainder 1. If you just give every thread 333 rows, one row is never processed. My solution: the last thread always gets everything from its start_row to ROWS (the actual end), which automatically absorbs any remainder."*

> *"I also considered whether I needed mutex locks. The answer is no — because each thread writes to a completely separate set of rows. Thread A writes rows 0–332, Thread B writes rows 333–665. They never write to the same memory address. So there's zero chance of a data race."*

---

## 🤔 Choices You Made & Why

| Choice | Why |
|--------|-----|
| `std::vector` | Allocates matrices on the heap to avoid stack overflow while allowing shared access |
| No mutexes/locks | Threads write to non-overlapping rows — no shared write targets = no race condition |
| `WorkerArgs` struct | Clean way to pass multiple arguments to a thread function |
| Remainder logic | Simple, guaranteed to distribute extra rows evenly to cover all rows without skipping any |
| 800×800 matrix size | Large enough to show meaningful speedup in performance tests |
| `w.join()` | Ensures main() waits for all results before verifying or printing |

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

### `std::thread`
- Modern C++ wrapper that creates a new OS-level thread to run a specific function
- `workers.emplace_back(add_rows, args)`:
  - Spawns a new thread executing `add_rows`
  - Passes the `args` directly to the function
  - Keeps a handle to the thread in the `workers` vector

### `std::thread::join()`
- Makes the calling thread (main) wait until the target thread finishes
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

**Q: What would happen if you removed `join()`?**
> The main function would reach the verification code before threads finish computing. You'd be reading garbage values from the result matrix, or the program might abort entirely because the main thread exited while child threads were active.

**Q: Why use `std::vector` instead of raw arrays inside main?**
> Three 800×800 int arrays is very large. Declaring them locally as raw arrays in main would overflow the stack and crash the program. `std::vector` dynamically allocates its data on the heap, which has plenty of space and is shared among all threads.

**Q: What is the WorkerArgs struct used for?**
> We need to pass multiple values to the thread: pointers to the matrices, what row to start at, and what row to end at. We pack all these into a `WorkerArgs` struct to keep the function signature clean and organized.

**Q: How does the remainder row handling work?**
> Integer division: 1000 / 3 = 333, remainder 1. The first two threads get 333 rows each. Instead of also giving the last thread 333 rows (which would skip row 999), we set its `end_row = ROWS` (= 1000), so it automatically gets rows 666–999 = 334 rows, covering the remainder.
