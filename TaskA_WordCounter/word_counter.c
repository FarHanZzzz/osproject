// ============================================================
// Task A — Word Counter using fork() and pipe()
// ============================================================
//
// THE PROBLEM:
//   We need to count the total number of words across multiple
//   text files. Doing it one file at a time (sequentially) is
//   slow. The assignment asks us to use MULTIPLE PROCESSES so
//   each file is counted at the same time (in parallel).
//
// THE SOLUTION:
//   We use two core OS concepts:
//
//   1. fork() — Creates a CHILD PROCESS.
//      Think of fork() like a photocopier for programs.
//      After fork(), you have two identical programs running:
//        - The PARENT (original) continues managing everything.
//        - The CHILD (copy) does the counting for ONE file.
//      So if we have 3 files, we fork() 3 times and get 3 children.
//
//   2. pipe() — Creates a ONE-WAY COMMUNICATION TUBE.
//      A pipe has two ends:
//        - Write end [1]: The child puts its result in here.
//        - Read  end [0]: The parent pulls the result from here.
//      We create ONE pipe per file so each child has its own
//      private channel to send its word count back to the parent.
//
// ENHANCEMENTS (as required by the PDF):
//   - Dynamic Tasks: The user provides file names on the command
//     line: ./word_counter file1.txt file2.txt ...
//     The program automatically forks one child per file.
//   - Error Reporting: If a file does not exist, the child sends
//     the special value -1 through the pipe instead of a word count.
//     The parent checks for -1 and reports it as an error.
//
// HOW IT ALL FLOWS (step by step):
//   Parent creates pipe[0] → forks Child 0 → Child 0 counts file0 → writes count to pipe[0][1]
//   Parent creates pipe[1] → forks Child 1 → Child 1 counts file1 → writes count to pipe[1][1]
//   ...all children run at the same time (parallel)...
//   Parent reads from pipe[0][0], pipe[1][0] ... → adds them up → prints total
//
// Compile: gcc -o word_counter word_counter.c
// Run:     ./word_counter sample1.txt sample2.txt


//cd /home/farhan-sadeque/Downloads/Osproject/TaskA_WordCounter
//gcc -o word_counter word_counter.c
//./word_counter sample1.txt sample2.txt


#include <stdio.h>    // Gives us printf(), fopen(), fclose(), fscanf()
#include <stdlib.h>   // Gives us exit() — used to terminate child processes
#include <unistd.h>   // Gives us fork(), pipe(), read(), write(), getpid()
#include <sys/wait.h> // Gives us waitpid() — parent waits for children to finish

// MAX_FILES: The maximum number of files (and therefore children) we support.
// We need this to pre-declare array sizes since C requires fixed-size arrays.

#define MAX_FILES 10


// HELPER FUNCTION: count_words
// ----------------------------------------------------------
// PURPOSE: Open a file and count how many words are inside it.
//
// HOW IT WORKS:
//   fscanf(fp, "%s", word) reads ONE word at a time from the file.
//   A "word" is any sequence of non-whitespace characters.
//   It skips over spaces, newlines, tabs automatically.
//   When the file is exhausted, fscanf returns EOF (not 1), and we stop.
//
// RETURNS: the word count, OR -1 if the file could not be opened.
//          This -1 is the "error code" that travels through the pipe.
// ----------------------------------------------------------




int count_words(const char *filename) {
    // fopen() tries to open the file for reading ("r" = read mode).
    // If the file does not exist, fopen() returns NULL (a null pointer).
    FILE *fp = fopen(filename, "r");

    // Check if fopen() failed (file not found or no permission).
    // !fp means "fp is NULL" which means "file could not be opened".
    if (!fp) {
        // getpid() returns the Process ID of the currently running process.
        // This lets us see WHICH child process hit the error.
        printf("  [Child %d] ERROR: Could not open '%s'\n", getpid(), filename);
        return -1; // Return -1 as an error signal to the parent via the pipe
    }

    int count = 0;         // This will hold our running word count
    char word[4096];       // A temporary buffer to hold each word as we read it
                           // 4096 bytes is large enough for any realistic word

    // fscanf(fp, "%4095s", word):
    //   - fp      → read from this file
    //   - "%4095s" → read one whitespace-delimited token (a "word"), max 4095 chars
    //   - word    → store the token in this buffer
    //   Returns 1 if it successfully read a word, EOF when the file ends.
    while (fscanf(fp, "%4095s", word) == 1) {
        count++; // We successfully read one word, so increment the counter
    }

    fclose(fp); // Always close the file when done to release OS resources
    return count; // Send back the total word count
}

// ----------------------------------------------------------
// MAIN FUNCTION — The parent process logic
// ----------------------------------------------------------
int main(int argc, char *argv[]) {
    // argc = the number of command-line arguments (including the program name)
    // argv = array of strings: argv[0]="./word_counter", argv[1]="file1.txt", etc.

    printf("=== Word Counter (fork + pipe) ===\n\n");

    // ======================================================
    // STEP 1: Parse command-line arguments (Dynamic Tasks)
    // ======================================================
    // The user can run: ./word_counter a.txt b.txt c.txt
    // argc will then be 4: the program name + 3 file names
    // We use argc > 1 to check if any file names were provided.

    int num_files;          // How many files we will process
    char *files[MAX_FILES]; // Array of pointers to file name strings

    if (argc > 1) {
        // User gave us file names on the command line
        num_files = argc - 1; // Subtract 1 because argv[0] is the program name itself
        if (num_files > MAX_FILES) num_files = MAX_FILES; // Safety cap at 10 files

        // Copy pointers to the file name strings from argv into our files[] array
        for (int i = 0; i < num_files; i++) {
            files[i] = argv[i + 1]; // argv[1] is the first file, argv[2] the second, etc.
        }
        printf("Processing %d file(s) from command line.\n\n", num_files);
    } else {
        // No files given — fall back to the two sample files
        num_files = 2;
        files[0] = "sample1.txt"; // Default file 1
        files[1] = "sample2.txt"; // Default file 2
        printf("No files specified. Using defaults: sample1.txt, sample2.txt\n");
        printf("Usage: ./word_counter file1.txt file2.txt ...\n\n");
    }

    // ======================================================
    // STEP 2: Create pipes and fork one child per file
    // ======================================================
    // pipes[i][0] = READ  end of pipe i (parent reads from here)
    // pipes[i][1] = WRITE end of pipe i (child writes into here)
    // We MUST create the pipe BEFORE fork() so that both the
    // parent and child inherit the same pipe file descriptors.

    int pipes[MAX_FILES][2]; // 2D array: one pair of file descriptors per file
    pid_t pids[MAX_FILES];   // Store each child's Process ID so parent can wait for it

    for (int i = 0; i < num_files; i++) {
        // pipe(pipes[i]) creates a new pipe.
        // After this call:
        //   pipes[i][0] is open for reading
        //   pipes[i][1] is open for writing
        pipe(pipes[i]);

        // fork() creates a copy of the current process.
        // After fork(), BOTH parent and child continue from this line.
        // fork() returns:
        //   0      → we are INSIDE the child process
        //   >0     → we are INSIDE the parent (the value is the child's PID)
        //   -1     → fork failed (error)
        pids[i] = fork();

        if (pids[i] == 0) {
            // ---- WE ARE INSIDE THE CHILD PROCESS ----

            // Close the READ end of pipe[i] — the child only WRITES results.
            // Keeping unused pipe ends open causes "deadlock" (parent waits forever).
            close(pipes[i][0]);

            // Close ALL pipes from previous iterations (pipes[0] through pipes[i-1]).
            // If we don't do this, the parent's read() will block forever because
            // those old pipe write-ends are still open inside this child.
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]); // Close read  end of old pipes
                close(pipes[j][1]); // Close write end of old pipes
            }

            // Count words in the file assigned to THIS child (files[i])
            // This calls our helper function above. Returns -1 on error.
            int result = count_words(files[i]);

            // Write the result (an integer) into the pipe.
            // &result = pointer to the integer, sizeof(result) = 4 bytes.
            // The parent will read exactly these 4 bytes from the other end.
            write(pipes[i][1], &result, sizeof(result));

            close(pipes[i][1]); // Done writing — close the write end
            exit(0);            // Terminate the child process (don't let it loop!)
        }

        // ---- BACK IN PARENT PROCESS ----
        // Close the WRITE end of this pipe — the parent only READS.
        // This is critical: if the parent keeps the write end open,
        // read() will never return EOF and the parent will block forever.
        close(pipes[i][1]);
    }
    // At this point: all children are running in parallel, counting their files.

    // ======================================================
    // STEP 3: Collect results from all pipes
    // ======================================================
    int total = 0;  // Accumulator for total word count across all files
    int errors = 0; // Count of files that could not be opened

    printf("Results:\n");
    for (int i = 0; i < num_files; i++) {
        int result = 0;
        // read() BLOCKS (waits) here until the child writes its result.
        // Once the child calls write() and then exit(), read() will return.
        // We read exactly sizeof(int) = 4 bytes from the pipe.
        read(pipes[i][0], &result, sizeof(result));
        close(pipes[i][0]); // Done reading this pipe — close it

        // Check if the child sent back an error code
        if (result == -1) {
            // -1 means the child could not open the file
            printf("  %-20s : ERROR (file not found)\n", files[i]);
            errors++; // Increment error counter
        } else {
            // A valid word count came back — print it and add to total
            printf("  %-20s : %d words\n", files[i], result);
            total += result; // Add this file's word count to the grand total
        }
    }

    // ======================================================
    // STEP 4: Wait for all children to finish (prevent zombies)
    // ======================================================
    // A "zombie" process is a child that has finished but whose
    // exit status has not been collected by the parent.
    // waitpid(pid, &status, 0) tells the OS: "I've acknowledged
    // that child pid is done — clean it up from the process table."
    for (int i = 0; i < num_files; i++) {
        int status;
        waitpid(pids[i], &status, 0); // Wait specifically for child i to finish
    }

    // ======================================================
    // STEP 5: Print the final summary
    // ======================================================
    printf("  ─────────────────────────\n");
    printf("  Total Words:          %d\n", total);
    if (errors > 0) {
        // Only show the errors line if there were actually errors
        printf("  Files with errors:    %d\n", errors);
    }

    return 0; // 0 = program exited successfully
}
