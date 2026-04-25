// ============================================================
// Task C — Parallel Matrix Adder using pthreads
// ============================================================
// PROBLEM (from PDF): Add two large matrices (A + B = C) using
// multiple threads to speed up the computation. Each thread
// should handle a portion of the rows independently.
//
// HOW WE SOLVE IT:
//   1. Create two 1000x1000 matrices (A and B) filled with data.
//   2. Create exactly 4 threads.
//   3. Each thread is assigned exactly 250 rows (1000 / 4 = 250).
//   4. Each thread adds A[i][j] + B[i][j] and stores it in C[i][j]
//      for its assigned rows ONLY.
//   5. Because threads work on DIFFERENT rows, there is no conflict
//      and NO locks/mutexes are needed.
//   6. The main thread waits for all 4 to finish, then prints results.
//
// Compile: gcc -o matrix_add matrix_adder.c -lpthread
// Run:     ./matrix_add
// ============================================================

#include <stdio.h>    // For printf
#include <stdlib.h>   // For general utilities
#include <pthread.h>  // For pthread_create, pthread_join (threading library)

// The size of our matrices: 1000 rows x 1000 columns
#define ROWS 1000
#define COLS 1000

// ----------------------------------------------------------
// GLOBAL MATRICES
// ----------------------------------------------------------
// We declare these outside of any function (globally) for two reasons:
//   1. They are too large for the stack (1000*1000*4 bytes = ~4MB each).
//   2. Global variables are shared by ALL threads automatically,
//      so every thread can read A, B and write to C without
//      needing pipes or any other communication mechanism.
int A[ROWS][COLS];  // First input matrix
int B[ROWS][COLS];  // Second input matrix
int C[ROWS][COLS];  // Result matrix (A + B)

// ----------------------------------------------------------
// THREAD ARGUMENT STRUCT
// ----------------------------------------------------------
// This is like a "sticky note" we hand to each thread.
// It tells the thread: "You are responsible for rows
// from start_row to end_row."
typedef struct {
    int start_row;  // First row this thread works on (inclusive)
    int end_row;    // Last row this thread works on (exclusive)
} ThreadArgs;

// ----------------------------------------------------------
// THREAD FUNCTION: add_rows
// ----------------------------------------------------------
// This is the function that each thread executes.
// All 4 threads run this function simultaneously, but each
// one receives a different ThreadArgs telling it which rows to process.
// Because they work on different rows, they never step on each other!
void *add_rows(void *arg) {
    // Cast the generic void* pointer back to our ThreadArgs struct
    ThreadArgs *t = (ThreadArgs *)arg;

    // Loop through ONLY the rows assigned to this thread
    for (int i = t->start_row; i < t->end_row; i++) {
        // For each column in this row...
        for (int j = 0; j < COLS; j++) {
            // Add the corresponding elements from A and B,
            // and store the result in C
            C[i][j] = A[i][j] + B[i][j];
        }
    }

    return NULL;  // Thread is done — return nothing
}

// ----------------------------------------------------------
// MAIN FUNCTION
// ----------------------------------------------------------
int main() {
    printf("=== Parallel Matrix Adder (4 Threads, 1000x1000) ===\n\n");

    // ======================================================
    // STEP 1: Fill the matrices with dummy data
    // ======================================================
    // We fill A with all 1s and B with all 2s.
    // So every element of C should be 3 (1 + 2 = 3).
    // This makes it easy to verify correctness!
    printf("Filling matrices A (all 1s) and B (all 2s)...\n");
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            A[i][j] = 1;  // Every element of A is 1
            B[i][j] = 2;  // Every element of B is 2
        }
    }

    // ======================================================
    // STEP 2: Create thread variables and argument "sticky notes"
    // ======================================================
    // We hardcode exactly 4 threads.
    // 1000 rows / 4 threads = 250 rows per thread.
    pthread_t thread1, thread2, thread3, thread4;

    // Each ThreadArgs tells a thread which rows to handle:
    //   Thread 1: rows   0 to 249
    //   Thread 2: rows 250 to 499
    //   Thread 3: rows 500 to 749
    //   Thread 4: rows 750 to 999
    ThreadArgs args1 = {0,   250};
    ThreadArgs args2 = {250, 500};
    ThreadArgs args3 = {500, 750};
    ThreadArgs args4 = {750, 1000};

    // ======================================================
    // STEP 3: Launch all 4 threads
    // ======================================================
    // pthread_create() starts a new thread.
    // Arguments:
    //   &thread1   = where to store the thread handle
    //   NULL       = default thread attributes
    //   add_rows   = the function the thread will execute
    //   &args1     = the argument to pass to the function
    printf("Launching 4 threads...\n");
    printf("  Thread 1: rows   0 - 249\n");
    printf("  Thread 2: rows 250 - 499\n");
    printf("  Thread 3: rows 500 - 749\n");
    printf("  Thread 4: rows 750 - 999\n\n");

    pthread_create(&thread1, NULL, add_rows, &args1);
    pthread_create(&thread2, NULL, add_rows, &args2);
    pthread_create(&thread3, NULL, add_rows, &args3);
    pthread_create(&thread4, NULL, add_rows, &args4);

    // ======================================================
    // STEP 4: Wait for all 4 threads to finish
    // ======================================================
    // pthread_join() blocks the main thread until the specified
    // thread finishes its work. This is similar to waitpid()
    // that we used for child processes in Task A.
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);

    // ======================================================
    // STEP 5: Verify the results
    // ======================================================
    // Since A = all 1s and B = all 2s, every C[i][j] should be 3.
    printf("All threads finished! Verifying results...\n");
    printf("  C[0][0]     = %d  (expected 3)\n", C[0][0]);
    printf("  C[500][500] = %d  (expected 3)\n", C[500][500]);
    printf("  C[999][999] = %d  (expected 3)\n", C[999][999]);
    printf("\nDone! Matrix addition complete.\n");

    return 0;  // Program finished successfully
}
