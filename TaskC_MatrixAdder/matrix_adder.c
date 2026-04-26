// ============================================================
// Task C — Parallel Matrix Adder using pthreads
// ============================================================
// PROBLEM (from PDF): Add two large matrices (A + B = C) using
// multiple threads to speed up the computation.
//
// ENHANCEMENTS:
//   - Dynamic Threading: User inputs thread count via command
//     line (e.g., ./matrix_add 8). Default is 4 threads.
//   - Remainder Handling: If rows aren't evenly divisible by
//     thread count, the last thread handles the extra rows.
//   - Performance Testing: Use `time ./matrix_add 1` vs
//     `time ./matrix_add 4` to compare speedup.
//
// Compile: gcc -o matrix_add matrix_adder.c -lpthread
// Run:     ./matrix_add         (uses 4 threads)
//          ./matrix_add 8       (uses 8 threads)
//          time ./matrix_add 1  (benchmark with 1 thread)
// ============================================================

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ROWS 1000
#define COLS 1000
#define MAX_THREADS 16

// ----------------------------------------------------------
// GLOBAL MATRICES (shared by all threads)
// ----------------------------------------------------------
// Declared globally because:
//   1. Too large for the stack (~4MB each)
//   2. All threads can access them without pipes or IPC
int A[ROWS][COLS];
int B[ROWS][COLS];
int C[ROWS][COLS];

// ----------------------------------------------------------
// THREAD ARGUMENT STRUCT
// ----------------------------------------------------------
// Each thread gets a "sticky note" telling it which rows
// to process. start_row is inclusive, end_row is exclusive.
typedef struct {
    int thread_id;
    int start_row;
    int end_row;
} ThreadArgs;

// ----------------------------------------------------------
// THREAD FUNCTION: add_rows
// ----------------------------------------------------------
// Each thread adds A[i][j] + B[i][j] = C[i][j] for its
// assigned rows ONLY. No two threads touch the same rows,
// so NO locks/mutexes are needed.
void *add_rows(void *arg) {
    ThreadArgs *t = (ThreadArgs *)arg;

    for (int i = t->start_row; i < t->end_row; i++) {
        for (int j = 0; j < COLS; j++) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }

    return NULL;
}

// ----------------------------------------------------------
// MAIN FUNCTION
// ----------------------------------------------------------
int main(int argc, char *argv[]) {
    // ======================================================
    // STEP 1: Determine thread count (Dynamic Threading)
    // ======================================================
    // User can specify thread count via command line.
    // Default is 4 threads if no argument provided.
    int num_threads = 4;  // default

    if (argc > 1) {
        num_threads = atoi(argv[1]);
        if (num_threads < 1) num_threads = 1;
        if (num_threads > MAX_THREADS) num_threads = MAX_THREADS;
    }

    printf("=== Parallel Matrix Adder (%d Thread(s), %dx%d) ===\n\n",
           num_threads, ROWS, COLS);

    // ======================================================
    // STEP 2: Fill the matrices with dummy data
    // ======================================================
    printf("Filling matrices A (all 1s) and B (all 2s)...\n");
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            A[i][j] = 1;
            B[i][j] = 2;
        }
    }

    // ======================================================
    // STEP 3: Calculate row chunks with remainder handling
    // ======================================================
    // If 1000 rows / 3 threads = 333 rows each, with 1 leftover.
    // The last thread handles the extra rows (333 + 1 = 334).
    int rows_per_thread = ROWS / num_threads;
    int remainder = ROWS % num_threads;

    pthread_t threads[MAX_THREADS];
    ThreadArgs args[MAX_THREADS];

    printf("Launching %d threads...\n", num_threads);

    int current_row = 0;
    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i + 1;
        args[i].start_row = current_row;

        // Last thread gets the remainder rows
        if (i == num_threads - 1) {
            args[i].end_row = ROWS;  // includes any leftover rows
        } else {
            args[i].end_row = current_row + rows_per_thread;
        }

        printf("  Thread %d: rows %4d - %4d (%d rows)\n",
               args[i].thread_id,
               args[i].start_row,
               args[i].end_row - 1,
               args[i].end_row - args[i].start_row);

        current_row = args[i].end_row;
    }

    if (remainder > 0) {
        printf("  (Thread %d handles %d extra remainder row(s))\n",
               num_threads, remainder);
    }
    printf("\n");

    // ======================================================
    // STEP 4: Launch all threads
    // ======================================================
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, add_rows, &args[i]);
    }

    // ======================================================
    // STEP 5: Wait for all threads to finish
    // ======================================================
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // ======================================================
    // STEP 6: Verify the results
    // ======================================================
    printf("All threads finished! Verifying results...\n");
    printf("  C[0][0]     = %d  (expected 3)\n", C[0][0]);
    printf("  C[500][500] = %d  (expected 3)\n", C[500][500]);
    printf("  C[999][999] = %d  (expected 3)\n", C[999][999]);

    // Full verification
    int correct = 1;
    for (int i = 0; i < ROWS && correct; i++) {
        for (int j = 0; j < COLS && correct; j++) {
            if (C[i][j] != 3) correct = 0;
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
