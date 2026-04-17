/*
 * ============================================================
 * Task A — Word Counter (fork + pipe)
 * CSC413/CSE315 — OS Programming Assignment, Spring 2026
 * ============================================================
 *
 * Description:
 *   The parent process forks one child per input file.
 *   Each child counts words in its assigned file and sends
 *   the result (or -1 on error) back to the parent via a
 *   dedicated pipe.  The parent sums all results and prints
 *   the grand total.
 *
 * Usage:
 *   ./word_counter <file1> [file2] [file3] ...
 *
 * Build:
 *   gcc -Wall -Wextra -o word_counter word_counter.c
 *
 * OS Concepts demonstrated:
 *   - fork()     : process creation
 *   - pipe()     : anonymous inter-process communication (IPC)
 *   - waitpid()  : synchronise parent with all children, prevents zombies
 *   - exit()     : child exits cleanly after writing result
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Maximum number of input files supported */
#define MAX_FILES 64

/* ----------------------------------------------------------------
 * count_words_in_file
 *   Opens the file at `path` and returns the number of whitespace-
 *   delimited words, or -1 if the file cannot be opened.
 * ---------------------------------------------------------------- */
static int count_words_in_file(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "[child pid=%d] ERROR: Cannot open '%s': %s\n",
                getpid(), path, strerror(errno));
        return -1;
    }

    int  count = 0;
    char word[4096];

    /* fscanf with %s skips all whitespace and reads one token at a time */
    while (fscanf(fp, "%4095s", word) == 1) {
        count++;
    }

    fclose(fp);
    return count;
}

/* ================================================================
 * main
 * ================================================================ */
int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [file2] [file3] ...\n", argv[0]);
        fprintf(stderr, "Example: %s essay.txt notes.txt readme.txt\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_files = argc - 1;
    if (num_files > MAX_FILES) {
        fprintf(stderr, "ERROR: Too many files (max %d).\n", MAX_FILES);
        return EXIT_FAILURE;
    }

    /* -------------------------------------------------------
     * Allocate one pipe per file.
     * pipes[i][0] = read end (used by parent)
     * pipes[i][1] = write end (used by child)
     * ------------------------------------------------------- */
    int   pipes[MAX_FILES][2];
    pid_t pids[MAX_FILES];

    printf("=== Word Counter — fork+pipe IPC ===\n");
    printf("Counting words in %d file(s)...\n\n", num_files);

    for (int i = 0; i < num_files; i++) {
        /* Create the pipe BEFORE forking so both parent and child inherit it */
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return EXIT_FAILURE;
        }

        pids[i] = fork();

        if (pids[i] < 0) {
            /* fork() failed */
            perror("fork");
            return EXIT_FAILURE;
        }

        if (pids[i] == 0) {
            /* ================================================
             * CHILD PROCESS
             * ================================================
             * Close ALL pipe ends we don't need.
             * - Close our own read end (we only write).
             * - Close OTHER pipes entirely (we only touch ours).
             * This is CRITICAL: if the parent still has a write
             * end open for any pipe, its read() will never see EOF.
             * ================================================ */
            for (int j = 0; j < num_files; j++) {
                if (j == i) {
                    close(pipes[j][0]); /* close read end of OUR pipe */
                } else {
                    close(pipes[j][0]); /* close other pipes' read ends */
                    close(pipes[j][1]); /* close other pipes' write ends */
                }
            }

            /* Count words */
            int result = count_words_in_file(argv[i + 1]);

            /* Send result (or -1) through our pipe */
            if (write(pipes[i][1], &result, sizeof(result)) == -1) {
                perror("write");
            }

            close(pipes[i][1]);
            exit(EXIT_SUCCESS);
        }

        /* ================================================
         * PARENT PROCESS (continues loop)
         * Close the WRITE end of this pipe immediately.
         * If we forget this, read() in the parent will
         * block forever because the pipe never gets EOF.
         * ================================================ */
        close(pipes[i][1]);
    }

    /* -------------------------------------------------------
     * Collect results from all children.
     * We read BEFORE waitpid so that children are not blocked
     * trying to write into a full pipe buffer.
     * ------------------------------------------------------- */
    int grand_total = 0;
    int any_error   = 0;

    for (int i = 0; i < num_files; i++) {
        int result = 0;
        ssize_t bytes_read = read(pipes[i][0], &result, sizeof(result));

        if (bytes_read <= 0) {
            fprintf(stderr, "WARNING: No data received from child for '%s'\n",
                    argv[i + 1]);
            any_error = 1;
        } else if (result == -1) {
            printf("  %-40s  ERROR (file not found or unreadable)\n", argv[i + 1]);
            any_error = 1;
        } else {
            printf("  %-40s  %6d word(s)\n", argv[i + 1], result);
            grand_total += result;
        }

        close(pipes[i][0]);
    }

    /* -------------------------------------------------------
     * Wait for ALL children to prevent zombie processes.
     * waitpid(-1, ...) waits for ANY child; we call it once
     * per child to reap all of them.
     * ------------------------------------------------------- */
    for (int i = 0; i < num_files; i++) {
        int status;
        pid_t finished = waitpid(pids[i], &status, 0);
        if (finished == -1) {
            perror("waitpid");
        } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "WARNING: child %d (pid=%d) exited with code %d\n",
                    i, finished, WEXITSTATUS(status));
        }
    }

    /* -------------------------------------------------------
     * Print grand total
     * ------------------------------------------------------- */
    printf("\n----------------------------------------\n");
    if (any_error) {
        printf("Grand Total (successful files only): %d word(s)\n", grand_total);
    } else {
        printf("Grand Total: %d word(s)\n", grand_total);
    }
    printf("========================================\n");

    return EXIT_SUCCESS;
}
