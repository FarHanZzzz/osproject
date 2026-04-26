// ============================================================
// Task A — Word Counter using fork() and pipe()
// ============================================================
// PROBLEM (from PDF): Create a program that counts words in
// text files using multiple child processes. Each child counts
// words in one file and sends the result back to the parent
// using a pipe (Inter-Process Communication).
//
// ENHANCEMENTS:
//   - Dynamic Tasks: User provides file names via command line
//     (e.g., ./word_counter file1.txt file2.txt file3.txt)
//   - Error Reporting: Child sends -1 through the pipe if a
//     file cannot be found.
//
// Compile: gcc -o word_counter word_counter.c
// Run:     ./word_counter sample1.txt sample2.txt
// ============================================================

#include <stdio.h>   // For printf, fopen, fscanf, fclose
#include <stdlib.h>  // For exit()
#include <unistd.h>  // For fork(), pipe(), read(), write()
#include <sys/wait.h> // For waitpid()

#define MAX_FILES 10  // Maximum number of files we support

// ----------------------------------------------------------
// HELPER FUNCTION: count_words
// Opens a file and counts how many words are inside it.
// Returns -1 if the file cannot be opened (error reporting).
// ----------------------------------------------------------
int count_words(const char *filename) {
    FILE *fp = fopen(filename, "r");

    // ERROR REPORTING: If the file doesn't exist, return -1
    // This -1 will be sent through the pipe to the parent
    if (!fp) {
        printf("  [Child %d] ERROR: Could not open '%s'\n", getpid(), filename);
        return -1;
    }

    int count = 0;
    char word[4096];

    // fscanf reads one word at a time, skipping whitespace
    while (fscanf(fp, "%4095s", word) == 1) {
        count++;
    }

    fclose(fp);
    return count;
}

// ----------------------------------------------------------
// MAIN FUNCTION
// ----------------------------------------------------------
int main(int argc, char *argv[]) {
    printf("=== Word Counter (fork + pipe) ===\n\n");

    // ======================================================
    // STEP 1: Check command-line arguments (Dynamic Tasks)
    // ======================================================
    // The user provides file names on the command line.
    // If no files given, show usage and use default files.
    int num_files;
    char *files[MAX_FILES];

    if (argc > 1) {
        // User provided file names: ./word_counter file1.txt file2.txt
        num_files = argc - 1;
        if (num_files > MAX_FILES) num_files = MAX_FILES;
        for (int i = 0; i < num_files; i++) {
            files[i] = argv[i + 1];
        }
        printf("Processing %d file(s) from command line.\n\n", num_files);
    } else {
        // No arguments — use default sample files
        num_files = 2;
        files[0] = "sample1.txt";
        files[1] = "sample2.txt";
        printf("No files specified. Using defaults: sample1.txt, sample2.txt\n");
        printf("Usage: ./word_counter file1.txt file2.txt ...\n\n");
    }

    // ======================================================
    // STEP 2: Create pipes and fork children dynamically
    // ======================================================
    // One pipe per file. Each pipe is a one-way tube:
    //   pipes[i][0] = READ end  (parent reads from here)
    //   pipes[i][1] = WRITE end (child writes into here)
    int pipes[MAX_FILES][2];
    pid_t pids[MAX_FILES];

    for (int i = 0; i < num_files; i++) {
        // Create the pipe BEFORE forking so both parent and
        // child inherit the file descriptors
        pipe(pipes[i]);

        // fork() creates a child process
        pids[i] = fork();

        if (pids[i] == 0) {
            // ---- WE ARE INSIDE THE CHILD ----

            // Close the read end — the child only WRITES
            close(pipes[i][0]);

            // Close all previously created pipes (prevent deadlock)
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Count words in the assigned file
            // Returns -1 if file not found (error reporting)
            int result = count_words(files[i]);

            // Write the result into the pipe (could be -1 for error)
            write(pipes[i][1], &result, sizeof(result));

            close(pipes[i][1]);  // Done writing
            exit(0);             // Exit the child process
        }

        // ---- PARENT continues here ----
        // Close the write end — the parent only READS
        close(pipes[i][1]);
    }

    // ======================================================
    // STEP 3: Read results from all pipes
    // ======================================================
    int total = 0;
    int errors = 0;

    printf("Results:\n");
    for (int i = 0; i < num_files; i++) {
        int result = 0;
        read(pipes[i][0], &result, sizeof(result));
        close(pipes[i][0]);

        // Check for error code (-1 means file not found)
        if (result == -1) {
            printf("  %-20s : ERROR (file not found)\n", files[i]);
            errors++;
        } else {
            printf("  %-20s : %d words\n", files[i], result);
            total += result;
        }
    }

    // ======================================================
    // STEP 4: Wait for all children to finish (prevent zombies)
    // ======================================================
    for (int i = 0; i < num_files; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    // ======================================================
    // STEP 5: Print final summary
    // ======================================================
    printf("  ─────────────────────────\n");
    printf("  Total Words:          %d\n", total);
    if (errors > 0) {
        printf("  Files with errors:    %d\n", errors);
    }

    return 0;
}
