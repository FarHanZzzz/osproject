// ============================================================
// Task A — Word Counter using fork() and pipe()
// ============================================================
// PROBLEM (from PDF): Create a program that counts words in
// text files using multiple child processes. Each child counts
// words in one file and sends the result back to the parent
// using a pipe (Inter-Process Communication).
//
// HOW WE SOLVE IT:
//   1. Parent creates two pipes (one per file).
//   2. Parent calls fork() twice to create two child processes.
//   3. Each child opens its assigned file, counts the words,
//      and writes the count into its pipe.
//   4. Parent reads the counts from both pipes and prints the total.
//
// Compile: gcc -o word_counter word_counter.c
// Run:     ./word_counter
// ============================================================

#include <stdio.h>   // For printf, fopen, fscanf, fclose
#include <stdlib.h>  // For exit()
#include <unistd.h>  // For fork(), pipe(), read(), write()
#include <sys/wait.h> // For waitpid() — to wait for children to finish

// ----------------------------------------------------------
// HELPER FUNCTION: count_words
// Opens a file and counts how many words are inside it.
// A "word" is any sequence of characters separated by spaces,
// tabs, or newlines. fscanf("%s") handles this automatically.
// ----------------------------------------------------------
int count_words(const char *filename) {
    // Try to open the file for reading
    FILE *fp = fopen(filename, "r");

    // If the file doesn't exist, tell the user and return 0
    if (!fp) {
        printf("  [Child] ERROR: Could not open '%s'\n", filename);
        return 0;
    }

    int count = 0;       // This will hold our word count
    char word[256];      // A buffer to temporarily store each word

    // fscanf reads one word at a time from the file.
    // It skips all whitespace (spaces, tabs, newlines) automatically.
    // It returns 1 if it successfully read a word, or EOF when done.
    while (fscanf(fp, "%255s", word) == 1) {
        count++;  // We found a word! Increment the counter.
    }

    fclose(fp);   // Always close the file when done
    return count; // Send the count back to whoever called this function
}

// ----------------------------------------------------------
// MAIN FUNCTION
// This is where the program starts. It creates pipes, forks
// child processes, and collects results.
// ----------------------------------------------------------
int main() {
    printf("=== Word Counter (fork + pipe) ===\n\n");

    // ======================================================
    // STEP 1: Create two pipes
    // ======================================================
    // A pipe is like a one-way tube.
    //   pipe1[0] = the READ end  (parent reads from here)
    //   pipe1[1] = the WRITE end (child writes into here)
    // We need one pipe per file because each child sends
    // its result through its own dedicated pipe.
    int pipe1[2];  // Pipe for file 1 (sample1.txt)
    int pipe2[2];  // Pipe for file 2 (sample2.txt)

    pipe(pipe1);   // Create the first pipe
    pipe(pipe2);   // Create the second pipe

    // ======================================================
    // STEP 2: Fork Child 1 to count words in "sample1.txt"
    // ======================================================
    // fork() creates a clone of this program.
    // In the PARENT, fork() returns the child's PID (a positive number).
    // In the CHILD, fork() returns 0.
    // So we use "if (pid == 0)" to detect that we are the child.
    pid_t pid1 = fork();

    if (pid1 == 0) {
        // ---- WE ARE INSIDE CHILD 1 ----

        // Close the read end — the child only WRITES, never reads
        close(pipe1[0]);

        // Count the words in sample1.txt
        int words = count_words("sample1.txt");

        // Write the integer result into the pipe so the parent can read it
        // We use &words to pass a pointer to the integer,
        // and sizeof(words) tells write() how many bytes to send (4 bytes for an int).
        write(pipe1[1], &words, sizeof(words));

        // Close the write end — we are done sending data
        close(pipe1[1]);

        // Exit the child process. This is important!
        // Without exit(), the child would continue running the parent's code below.
        exit(0);
    }

    // ======================================================
    // STEP 3: Fork Child 2 to count words in "sample2.txt"
    // ======================================================
    // Same logic as above, but for the second file.
    pid_t pid2 = fork();

    if (pid2 == 0) {
        // ---- WE ARE INSIDE CHILD 2 ----

        close(pipe2[0]);  // Close read end

        int words = count_words("sample2.txt");

        write(pipe2[1], &words, sizeof(words));

        close(pipe2[1]);  // Close write end

        exit(0);          // Exit the child
    }

    // ======================================================
    // STEP 4: Parent waits for both children to finish
    // ======================================================
    // The parent must close the WRITE ends of the pipes.
    // If the parent keeps the write end open, read() will
    // block forever waiting for more data that never comes.
    close(pipe1[1]);
    close(pipe2[1]);

    // waitpid() pauses the parent until the specified child exits.
    // This prevents "zombie processes" (dead children that aren't cleaned up).
    waitpid(pid1, NULL, 0);  // Wait for Child 1
    waitpid(pid2, NULL, 0);  // Wait for Child 2

    // ======================================================
    // STEP 5: Read results from the pipes
    // ======================================================
    int result1 = 0;
    int result2 = 0;

    // read() pulls data from the READ end of the pipe.
    // The child already wrote an integer into pipe1[1],
    // so we read it from pipe1[0].
    read(pipe1[0], &result1, sizeof(result1));
    read(pipe2[0], &result2, sizeof(result2));

    // Close the read ends now that we've got the data
    close(pipe1[0]);
    close(pipe2[0]);

    // ======================================================
    // STEP 6: Print the final results
    // ======================================================
    printf("  Words in sample1.txt: %d\n", result1);
    printf("  Words in sample2.txt: %d\n", result2);
    printf("  ─────────────────────────\n");
    printf("  Total Words:          %d\n", result1 + result2);

    return 0;  // Program finished successfully
}
