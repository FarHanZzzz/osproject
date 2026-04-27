// ============================================================
// Task C — Parallel Matrix Adder using pthreads
// ============================================================
//
// THE PROBLEM:
//   Add two large matrices A and B together (C = A + B).
//   A matrix is a 2D grid of numbers. If each matrix is
//   1000x1000, that means 1,000,000 additions to perform.
//   Doing this one cell at a time in a single loop is slow.
//
// THE SOLUTION — Parallel Threads:
//   We split the work across multiple THREADS. A thread is like
//   a mini-worker running inside the same program simultaneously.
//   Thread A handles rows 0-249, Thread B handles rows 250-499,
//   Thread C handles rows 500-749, Thread D handles rows 750-999.
//   All 4 threads run at the SAME TIME on different CPU cores.
//
// WHY NO LOCKS (mutex) ARE NEEDED:
//   The three key safety rules from the PDF:
//   a) Data Partitioning:   Thread A only touches rows 0-9, etc.
//   b) Independent Writes:  No two threads ever write to the same cell.
//   c) Read-Only Inputs:    A and B are never modified, just read.
//   Since threads never share write targets, they cannot interfere.
//
// ENHANCEMENTS (from the PDF):
//   - Dynamic Threading: ./matrix_add 8 uses 8 threads
//   - Remainder Handling: If rows % threads != 0, last thread
//     handles the leftover rows
//   - Performance Testing: time ./matrix_add 1 vs time ./matrix_add 4
//
// Compile: gcc -o matrix_add matrix_adder.c -lpthread
// Run:     ./matrix_add          (4 threads, default)
//          ./matrix_add 8        (8 threads)
//          time ./matrix_add 1   (benchmark 1 thread)
//          time ./matrix_add 4   (benchmark 4 threads)
// ============================================================

#include <stdio.h>    // printf() for output
#include <stdlib.h>   // atoi() — converts command-line string to integer
#include <pthread.h>  // pthread_create(), pthread_join() — POSIX thread functions

#define ROWS 1000       // Number of rows in each matrix
#define COLS 1000       // Number of columns in each matrix
#define MAX_THREADS 16  // Safety cap: don't allow more than 16 threads

// ----------------------------------------------------------
// GLOBAL MATRICES — shared memory accessible by ALL threads
// ----------------------------------------------------------
// These are declared globally (outside main) for two reasons:
//   1. Stack space: 1000x1000 ints = 4MB each. The thread stack
//      is only ~8MB, so putting these locally would cause a crash.
//   2. Sharing: All threads can access the same A, B, C arrays
//      without needing to pass them as parameters.
int A[ROWS][COLS]; // Input matrix A — filled with all 1s
int B[ROWS][COLS]; // Input matrix B — filled with all 2s
int C[ROWS][COLS]; // Output matrix C = A + B — threads write here

// ----------------------------------------------------------
// THREAD ARGUMENT STRUCT
// ----------------------------------------------------------
// When we create each thread, we need to tell it:
//   - WHICH thread number it is (for printing)
//   - WHICH rows it should process (start_row to end_row)
// We pack this info into a struct and pass it to pthread_create().
// Think of it as a "sticky note" given to each worker.
typedef struct {
    int thread_id;   // Thread number (1, 2, 3...) — just for printing
    int start_row;   // First row this thread should process (inclusive)
    int end_row;     // One past the last row (exclusive), like Python slicing
} ThreadArgs;

// ----------------------------------------------------------
// THREAD FUNCTION: add_rows
// ----------------------------------------------------------
// This is the function each thread executes after being created.
// pthread_create() requires the function signature: void* func(void* arg)
// The void* arg is our ThreadArgs struct, cast from void* back to ThreadArgs*.
//
// WHAT IT DOES: For every row assigned to this thread, loop through
// every column and compute C[row][col] = A[row][col] + B[row][col].
// ----------------------------------------------------------
void *add_rows(void *arg) {
    // Cast the generic void* argument back to our specific struct type.
    // arg is a pointer to a ThreadArgs; we cast it so C knows the layout.
    ThreadArgs *t = (ThreadArgs *)arg;

    // Outer loop: iterate over THIS thread's assigned rows only
    // t->start_row to t->end_row is the exclusive range this thread owns
    for (int i = t->start_row; i < t->end_row; i++) {
        // Inner loop: process every column in this row
        for (int j = 0; j < COLS; j++) {
            // The actual addition: C[i][j] = A[i][j] + B[i][j]
            // Since A=1 everywhere and B=2 everywhere, C will be 3 everywhere.
            C[i][j] = A[i][j] + B[i][j];
        }
    }

    return NULL; // Thread functions must return void*; NULL means "no result"
}

// ----------------------------------------------------------
// MAIN FUNCTION
// ----------------------------------------------------------
int main(int argc, char *argv[]) {
    // argc = number of command-line arguments (including program name)
    // argv[0] = "./matrix_add", argv[1] = "8" (if user typed ./matrix_add 8)

    // ======================================================
    // STEP 1: Read thread count from command line (Dynamic Threading)
    // ======================================================
    int num_threads = 4; // Default: use 4 threads if user gives no argument

    if (argc > 1) {
        // atoi() converts a string like "8" to the integer 8
        num_threads = atoi(argv[1]);
        if (num_threads < 1) num_threads = 1;             // Minimum 1 thread
        if (num_threads > MAX_THREADS) num_threads = MAX_THREADS; // Safety cap
    }

    printf("=== Parallel Matrix Adder (%d Thread(s), %dx%d) ===\n\n",
           num_threads, ROWS, COLS);

    // ======================================================
    // STEP 2: Fill matrices A and B with test data
    // ======================================================
    // A = all 1s, B = all 2s. So C = A+B should be all 3s.
    // This makes verification easy: just check that every C[i][j] == 3.
    printf("Filling matrices A (all 1s) and B (all 2s)...\n");
    for (int i = 0; i < ROWS; i++) {          // Loop through every row
        for (int j = 0; j < COLS; j++) {      // Loop through every column
            A[i][j] = 1; // Fill A with 1s
            B[i][j] = 2; // Fill B with 2s
        }
    }

    // ======================================================
    // STEP 3: Calculate which rows each thread handles
    // ======================================================
    // Division: 1000 rows / 4 threads = 250 rows per thread (no remainder).
    // Division: 1000 rows / 3 threads = 333 rows each, remainder = 1 row.
    // The last thread always handles any leftover "remainder" rows.
    int rows_per_thread = ROWS / num_threads;  // Base rows per thread (integer division)
    int remainder = ROWS % num_threads;        // Leftover rows after even division

    pthread_t threads[MAX_THREADS];   // Array of thread handles (OS identifiers)
    ThreadArgs args[MAX_THREADS];     // Array of "sticky notes" for each thread

    printf("Launching %d threads...\n", num_threads);

    int current_row = 0; // Track which row we are assigning next

    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i + 1;     // Thread IDs start at 1 (human-friendly)
        args[i].start_row = current_row; // This thread starts at the next unassigned row

        if (i == num_threads - 1) {
            // LAST THREAD: give it all remaining rows including any remainder.
            // This ensures no rows are accidentally skipped.
            args[i].end_row = ROWS;
        } else {
            // All other threads: get exactly rows_per_thread rows
            args[i].end_row = current_row + rows_per_thread;
        }

        printf("  Thread %d: rows %4d - %4d (%d rows)\n",
               args[i].thread_id,
               args[i].start_row,
               args[i].end_row - 1,                      // Print inclusive last row
               args[i].end_row - args[i].start_row);     // How many rows this thread has

        current_row = args[i].end_row; // Advance to the next unassigned row
    }

    if (remainder > 0) {
        // Inform the user that the last thread handled some extra rows
        printf("  (Thread %d handles %d extra remainder row(s))\n",
               num_threads, remainder);
    }
    printf("\n");

    // ======================================================
    // STEP 4: Launch all threads simultaneously
    // ======================================================
    for (int i = 0; i < num_threads; i++) {
        // pthread_create() starts a new thread running add_rows().
        // Arguments:
        //   &threads[i]  — OUTPUT: stores the thread handle (thread ID)
        //   NULL         — use default thread attributes
        //   add_rows     — the function this thread should execute
        //   &args[i]     — the "sticky note" (ThreadArgs) for this thread
        pthread_create(&threads[i], NULL, add_rows, &args[i]);
    }
    // All threads are now running in parallel on different CPU cores!

    // ======================================================
    // STEP 5: Wait for all threads to finish
    // ======================================================
    for (int i = 0; i < num_threads; i++) {
        // pthread_join() BLOCKS until thread i finishes its work.
        // Without this, main() might print results before threads are done.
        // NULL = we don't need the thread's return value.
        pthread_join(threads[i], NULL);
    }
    // All threads have finished — matrix C is fully computed.

    // ======================================================
    // STEP 6: Verify correctness
    // ======================================================
    // Spot-check 3 cells (corners + middle) to show correct results
    printf("All threads finished! Verifying results...\n");
    printf("  C[0][0]     = %d  (expected 3)\n", C[0][0]);
    printf("  C[500][500] = %d  (expected 3)\n", C[500][500]);
    printf("  C[999][999] = %d  (expected 3)\n", C[999][999]);

    // Full scan of the entire output matrix to confirm every cell is 3
    int correct = 1; // Assume correct until we find an error
    for (int i = 0; i < ROWS && correct; i++) {       // Stop early if error found
        for (int j = 0; j < COLS && correct; j++) {
            if (C[i][j] != 3) correct = 0; // Found a wrong value!
        }
    }
    printf("  Full verification: %s\n", correct ? "ALL CORRECT" : "ERRORS FOUND");

    printf("\nDone! Matrix addition complete.\n");
    printf("\nPerformance Testing Tip:\n");
    printf("  time ./matrix_add 1   (single-threaded baseline)\n");
    printf("  time ./matrix_add 4   (4 threads)\n");
    printf("  time ./matrix_add 8   (8 threads)\n");

    return 0;
}
