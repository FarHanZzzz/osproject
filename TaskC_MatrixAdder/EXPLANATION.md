# Task C: Matrix Adder (Concepts First!)

Before diving into the code, let's understand the core concept of **Threads**.

---

## 🧠 Core Concept: Processes vs. Threads
In Task A, we learned about `fork()`, which creates a brand new **Process** (a clone). If a Process is a house, then a `fork()` builds a completely separate, identical house next door. They can't easily share a kitchen.

A **Thread**, however, is like adding a new roommate to your *current* house. 
- You both share the same kitchen (memory).
- You can both chop vegetables on the same counter at the same time.
- Because you don't have to build a whole new house, creating a thread is *much faster and lighter* than creating a process!

In this task, we have two massive lists of numbers (1000x1000 Matrices) that need to be added together. If one roommate does all the math, it takes a long time. So, we hire **4 roommates (threads)**. We tell Roommate 1 to do the first 250 rows, Roommate 2 to do the next 250, and so on. They all write their answers onto the *same shared piece of paper* (the global matrix `C`).

---

## 💻 How the Code Works

This program is extremely simple and **hardcoded** to use exactly 4 threads. It divides 1000 rows perfectly into chunks of 250.

### Step-by-Step Walkthrough

1. **Defining the Shared Memory**
   ```c
   int A[ROWS][COLS];
   int B[ROWS][COLS];
   int C[ROWS][COLS];
   ```
   These are our matrices. Because they are declared outside of any function (Global Variables), all threads can see and touch them without needing a "Pipe" to communicate!

2. **Telling Threads What To Do**
   ```c
   ThreadArgs args1 = {0,   250};
   ThreadArgs args2 = {250, 500};
   // ...
   ```
   We create a simple struct. It's a sticky note that says: *"Hey Thread 1, you start at row 0 and stop at row 250."*

3. **Starting the Threads**
   ```c
   pthread_create(&thread1, NULL, add_rows, &args1);
   ```
   This is where the magic happens. We spawn the first thread. We tell it to run the `add_rows` function, and we hand it the `args1` sticky note. We do this 4 times.

4. **The Work Function**
   ```c
   void *add_rows(void *arg) {
       ThreadArgs *t = (ThreadArgs *)arg;
       for (int i = t->start_row; i < t->end_row; i++) {
           for (int j = 0; j < COLS; j++) {
               C[i][j] = A[i][j] + B[i][j];
           }
       }
   }
   ```
   All 4 threads are running this function at the exact same time. Thread 1 is adding row 0, while Thread 2 is adding row 250. Because they are working on *different rows*, they never bump into each other. No "locks" are needed!

5. **Waiting for the End**
   ```c
   pthread_join(thread1, NULL);
   ```
   Just like `waitpid()` in Task A, `pthread_join()` makes the main program pause and wait for the roommate to finish their math before exiting the program.
