// Task C — Parallel Matrix Adder using pthreads
// Adds two 1000x1000 matrices (A + B = C) using multiple threads.
// Each thread handles a chunk of rows — no locks needed because no two threads touch the same row.
//
// Compile: gcc -o matrix_add matrix_adder.c -lpthread
// Run:     ./matrix_add 4       (uses 4 threads)

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ROWS 1000
#define COLS 1000

// Matrices declared as static/global so they live in data segment (too big for stack)
static int A[ROWS][COLS];
static int B[ROWS][COLS];
static int C[ROWS][COLS];  // result matrix

// Each thread gets told which rows to work on
typedef struct {
    int id;
    int start_row;  // first row (inclusive)
    int end_row;    // last row (exclusive)
} ThreadArgs;

// This is what each thread runs
void *add_rows(void *arg) {
    ThreadArgs *t = (ThreadArgs *)arg;

    printf("  Thread %d: rows %d to %d\n", t->id, t->start_row, t->end_row - 1);

    for (int i = t->start_row; i < t->end_row; i++) {
        for (int j = 0; j < COLS; j++) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }

    return NULL;
}

// Fill matrices with some values so we have something to add
void fill_matrices() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            A[i][j] = i + j;
            B[i][j] = (i * j) % 100;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads < 1 || num_threads > ROWS) {
        printf("Error: threads must be between 1 and %d\n", ROWS);
        return 1;
    }

    printf("=== Parallel Matrix Adder (%dx%d) ===\n", ROWS, COLS);
    printf("Using %d thread(s)\n\n", num_threads);

    fill_matrices();

    // Figure out how many rows each thread gets
    int rows_per_thread = ROWS / num_threads;
    int extra_rows      = ROWS % num_threads;  // leftover rows go to last thread

    pthread_t  threads[num_threads];
    ThreadArgs args[num_threads];

    // Create all threads
    int current_row = 0;
    for (int i = 0; i < num_threads; i++) {
        args[i].id        = i;
        args[i].start_row = current_row;
        args[i].end_row   = current_row + rows_per_thread;

        // Last thread gets any extra rows
        if (i == num_threads - 1)
            args[i].end_row += extra_rows;

        current_row = args[i].end_row;

        pthread_create(&threads[i], NULL, add_rows, &args[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Show a small sample of the result
    printf("\nResult sample:\n");
    printf("  C[0][0] = %d\n", C[0][0]);
    printf("  C[0][1] = %d\n", C[0][1]);
    printf("  C[999][999] = %d\n", C[999][999]);

    printf("\nDone! Matrix addition complete.\n");
    return 0;
}
