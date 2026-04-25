// Task A — Word Counter using fork() and pipe()
// Each child process counts words in one file and sends the count back to parent via a pipe.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_FILES 64

// Count words in a file. Returns word count, or -1 if file can't be opened.
int count_words(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "[Child %d] Can't open '%s': %s\n", getpid(), filename, strerror(errno));
        return -1;
    }

    int count = 0;
    char word[4096];

    // fscanf with %s reads one word at a time, skipping whitespace
    while (fscanf(fp, "%4095s", word) == 1) {
        count++;
    }

    fclose(fp);
    return count;
}

int main(int argc, char *argv[]) {
    // Check: user must give at least one filename
    if (argc < 2) {
        printf("Usage: %s <file1> [file2] [file3] ...\n", argv[0]);
        return 1;
    }

    int num_files = argc - 1;
    if (num_files > MAX_FILES) {
        printf("Error: too many files (max %d)\n", MAX_FILES);
        return 1;
    }

    int   pipes[MAX_FILES][2];  // pipes[i][0]=read end, pipes[i][1]=write end
    pid_t pids[MAX_FILES];      // store child PIDs

    printf("=== Word Counter (fork + pipe) ===\n");
    printf("Counting words in %d file(s)...\n\n", num_files);

    // For each file: create a pipe, then fork a child
    for (int i = 0; i < num_files; i++) {

        // Create pipe BEFORE fork so both parent and child get it
        if (pipe(pipes[i]) == -1) {
            perror("pipe failed");
            return 1;
        }

        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork failed");
            return 1;
        }

        if (pids[i] == 0) {
            // ---- CHILD PROCESS ----

            // Close pipe ends we don't need
            close(pipes[i][0]);  // child doesn't read, only writes

            // Close ALL other pipes (very important!)
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Count words and send result through pipe
            int result = count_words(argv[i + 1]);
            write(pipes[i][1], &result, sizeof(result));

            close(pipes[i][1]);
            exit(0);
        }

        // ---- PARENT PROCESS ----
        // Close the write end — parent only reads
        // If we forget this, read() will block forever!
        close(pipes[i][1]);
    }

    // Collect results from all children
    int total = 0;
    int errors = 0;

    for (int i = 0; i < num_files; i++) {
        int result = 0;
        ssize_t n = read(pipes[i][0], &result, sizeof(result));

        if (n <= 0) {
            printf("  %-40s  ERROR (no data)\n", argv[i + 1]);
            errors = 1;
        } else if (result == -1) {
            printf("  %-40s  ERROR (file not found)\n", argv[i + 1]);
            errors = 1;
        } else {
            printf("  %-40s  %d words\n", argv[i + 1], result);
            total += result;
        }

        close(pipes[i][0]);
    }

    // Wait for all children (prevents zombie processes)
    for (int i = 0; i < num_files; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    // Print total
    printf("\n----------------------------------------\n");
    if (errors)
        printf("Total (successful files only): %d words\n", total);
    else
        printf("Total: %d words\n", total);

    return 0;
}
