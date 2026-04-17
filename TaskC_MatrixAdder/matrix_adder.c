/* Feature-test macro: expose POSIX.1-2001 extensions (clock_gettime, etc.) */
#define _POSIX_C_SOURCE 199309L

/*
 * ============================================================
 * Task C — Multi-threaded Parallel Matrix Adder
 * CSC413/CSE315 — OS Programming Assignment, Spring 2026
 * ============================================================
 *
 * Description:
 *   Adds two large NxN matrices (A + B = C) in parallel using
 *   POSIX threads.  Each thread owns a disjoint row range, so
 *   no two threads ever write to the same memory address —
 *   making mutexes completely unnecessary.
 *
 * Usage:
 *   ./matrix_add <num_threads>
 *   e.g.:  ./matrix_add 4
 *
 * Build:
 *   gcc -Wall -Wextra -std=c11 -O2 -D_POSIX_C_SOURCE=199309L -o matrix_add matrix_adder.c -lpthread
 *
 * Performance test (Linux):
 *   time ./matrix_add 1
 *   time ./matrix_add 4
 *
 * OS Concepts demonstrated:
 *   - pthread_create()  : spawn threads
 *   - pthread_join()    : synchronise / wait for threads
 *   - Data partitioning : disjoint row ranges → zero write conflicts
 *   - No mutex needed   : independent memory regions per thread
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

/* ---- Matrix dimensions ---- */
#define ROWS 1000
#define COLS 1000

/* ---- Statically allocated matrices (avoids heap fragmentation overhead) ---- */
static int A[ROWS][COLS];
static int B[ROWS][COLS];
static int C[ROWS][COLS];   /* result */
static int C_ref[ROWS][COLS]; /* single-thread reference for correctness check */

/* ================================================================
 * ThreadArgs
 *   Passed to each worker thread.  Only the row range differs
 *   between threads; all threads share the same matrix pointers
 *   but write to completely separate rows.
 * ================================================================ */
typedef struct {
    int thread_id;
    int start_row;   /* inclusive */
    int end_row;     /* exclusive */
} ThreadArgs;

/* ================================================================
 * thread_add_rows
 *   Worker function: adds rows [start_row, end_row) of A and B
 *   into C.  No synchronisation needed — each thread writes to
 *   its own private row range.
 * ================================================================ */
static void *thread_add_rows(void *arg)
{
    ThreadArgs *ta = (ThreadArgs *)arg;

    printf("  [Thread %2d] Processing rows %4d – %4d\n",
           ta->thread_id, ta->start_row, ta->end_row - 1);

    for (int i = ta->start_row; i < ta->end_row; i++) {
        for (int j = 0; j < COLS; j++) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }

    pthread_exit(NULL);
}

/* ================================================================
 * init_matrices
 *   Fill A and B with deterministic values so the result is
 *   easy to verify: A[i][j] = i+j, B[i][j] = i*j % 100
 * ================================================================ */
static void init_matrices(void)
{
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            A[i][j] = i + j;
            B[i][j] = (i * j) % 100;
        }
    }
}

/* ================================================================
 * verify_result
 *   Compute the expected result single-threaded and compare.
 *   Returns 1 if correct, 0 otherwise.
 * ================================================================ */
static int verify_result(void)
{
    /* Build reference */
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            C_ref[i][j] = A[i][j] + B[i][j];

    /* Compare */
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            if (C[i][j] != C_ref[i][j])
                return 0;

    return 1;
}

/* ================================================================
 * elapsed_ms
 *   Returns milliseconds between two timespec values.
 * ================================================================ */
static double elapsed_ms(struct timespec *start, struct timespec *end)
{
    return (end->tv_sec  - start->tv_sec)  * 1000.0
         + (end->tv_nsec - start->tv_nsec) / 1e6;
}

/* ================================================================
 * main
 * ================================================================ */
int main(int argc, char *argv[])
{
    /* ---- Parse argument ---- */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        fprintf(stderr, "Example: %s 4\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads < 1 || num_threads > ROWS) {
        fprintf(stderr, "ERROR: num_threads must be between 1 and %d.\n", ROWS);
        return EXIT_FAILURE;
    }

    printf("=== Parallel Matrix Adder (%dx%d) ===\n", ROWS, COLS);
    printf("Threads requested: %d\n\n", num_threads);

    /* ---- Initialise input matrices ---- */
    init_matrices();

    /* ---- Compute row chunk size ---- */
    int chunk      = ROWS / num_threads;   /* rows per thread */
    int remainder  = ROWS % num_threads;   /* extra rows for the last thread */

    if (chunk == 0) {
        fprintf(stderr,
            "WARNING: More threads (%d) than rows (%d). "
            "Reducing to %d thread(s).\n",
            num_threads, ROWS, ROWS);
        num_threads = ROWS;
        chunk = 1;
        remainder = 0;
    }

    /* ---- Allocate thread handles and argument structs ---- */
    pthread_t  *threads = malloc((size_t)num_threads * sizeof(pthread_t));
    ThreadArgs *args    = malloc((size_t)num_threads * sizeof(ThreadArgs));
    if (!threads || !args) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    /* ---- Start timer ---- */
    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    /* ---- Spawn threads ---- */
    int current_row = 0;
    for (int t = 0; t < num_threads; t++) {
        args[t].thread_id = t;
        args[t].start_row = current_row;
        /* Last thread absorbs remainder rows */
        args[t].end_row   = current_row + chunk + (t == num_threads - 1 ? remainder : 0);
        current_row       = args[t].end_row;

        int rc = pthread_create(&threads[t], NULL, thread_add_rows, &args[t]);
        if (rc != 0) {
            fprintf(stderr, "ERROR: pthread_create failed for thread %d: %s\n",
                    t, strerror(rc));
            free(threads);
            free(args);
            return EXIT_FAILURE;
        }
    }

    /* ---- Join all threads (wait for completion) ---- */
    for (int t = 0; t < num_threads; t++) {
        int rc = pthread_join(threads[t], NULL);
        if (rc != 0) {
            fprintf(stderr, "WARNING: pthread_join failed for thread %d: %s\n",
                    t, strerror(rc));
        }
    }

    /* ---- Stop timer ---- */
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double ms = elapsed_ms(&t_start, &t_end);

    /* ---- Verify correctness ---- */
    printf("\nVerifying result... ");
    fflush(stdout);
    if (verify_result()) {
        printf("CORRECT ✓\n");
    } else {
        printf("MISMATCH ✗  (bug in partitioning!)\n");
        free(threads);
        free(args);
        return EXIT_FAILURE;
    }

    /* ---- Print a small sample of the result ---- */
    printf("\nSample output C[0][0..4]: ");
    for (int j = 0; j < 5; j++) printf("%5d ", C[0][j]);
    printf("\nSample output C[999][995..999]: ");
    for (int j = 995; j < 1000; j++) printf("%5d ", C[999][j]);
    printf("\n");

    /* ---- Timing report ---- */
    printf("\n--- Performance ---\n");
    printf("  Matrix size  : %d x %d (%lu elements)\n",
           ROWS, COLS, (unsigned long)ROWS * COLS);
    printf("  Threads used : %d (chunk=%d rows each", num_threads, chunk);
    if (remainder) printf(", last thread gets %d extra", remainder);
    printf(")\n");
    printf("  Wall time    : %.3f ms\n", ms);

    free(threads);
    free(args);
    return EXIT_SUCCESS;
}
