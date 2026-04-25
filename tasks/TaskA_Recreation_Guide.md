# Task A: Word Counter — Code Recreation Guide

This guide contains the exact code to solve the Task A Word Counter problem from scratch. You can use these files to completely recreate the project.

---

## 1. Project Setup

1. Create a new folder for the project:
   ```bash
   mkdir TaskA_WordCounter
   cd TaskA_WordCounter
   ```
2. Create two empty files:
   ```bash
   touch word_counter.c
   touch Makefile
   ```

---

## 2. The Source Code (`word_counter.c`)

Open the `word_counter.c` file and paste the exact C code below. This code utilizes `fork()`, `pipe()`, and `waitpid()` to solve the word-counting task in parallel.

```c
/*
 * ============================================================
 * Task A — Word Counter (fork + pipe)
 * ============================================================
 *
 * Description:
 *   The parent process forks one child per input file.
 *   Each child counts words in its assigned file and sends
 *   the result (or -1 on error) back to the parent via a
 *   dedicated pipe. The parent sums all results and prints
 *   the grand total.
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
             * ================================================ */
            /* Close ALL pipe ends we don't need to prevent deadlocks */
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
         * PARENT PROCESS
         * ================================================ */
        /* Close the WRITE end immediately to ensure EOF works for read() */
        close(pipes[i][1]);
    }

    /* -------------------------------------------------------
     * Collect results from all children.
     * ------------------------------------------------------- */
    int grand_total = 0;
    int any_error   = 0;

    for (int i = 0; i < num_files; i++) {
        int result = 0;
        ssize_t bytes_read = read(pipes[i][0], &result, sizeof(result));

        if (bytes_read <= 0) {
            fprintf(stderr, "WARNING: No data received from child for '%s'\n", argv[i + 1]);
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
     * ------------------------------------------------------- */
    for (int i = 0; i < num_files; i++) {
        int status;
        pid_t finished = waitpid(pids[i], &status, 0);
        if (finished == -1) {
            perror("waitpid");
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
```

### Why This Code? (C Code Breakdown based on Project Requirements)

To help you understand *why* the code was written this way, here is how the specific parts of the code map directly to your assignment's requirements:

#### 1. Core Requirement: Forking and Pipes
* **Process Forking (`fork()`)**: The prompt requires the parent to fork multiple children. We do this inside a loop (`for (int i = 0; i < num_files; i++)`) so that multiple files can be processed in parallel.
* **Pipe Creation (`pipe(pipes[i])`)**: The prompt states the child "sends the count back to the parent via a pipe". We create one pipe for each file *before* we `fork()`. This ensures both the parent and child processes share the exact same pipe channel. 
* **Parent Process Logic (Summing Results)**: The requirement states "The parent then sums up the results". We accomplish this using a loop in the parent process, where `read(pipes[i][0], ...)` collects the counts sent by all children, adding them up into the `grand_total` variable.

#### 2. Enhancement: Dynamic Tasks
* **Dynamic Input (`int num_files = argc - 1;`)**: The prompt asks to "Allow the user to input multiple file names". Instead of hardcoding file names in the code, our program reads the command-line arguments (`argc` and `argv[]`). This lets the user provide any number of files when running the program (e.g., `./word_counter file1.txt file2.txt`).
* **Dynamic Children Creation**: The prompt asks to "have the shell fork a child for each file". Because our `for` loop dynamically loops `num_files` times, it successfully forks exactly one child for each file provided dynamically by the user.

#### 3. Enhancement: Error Reporting
* **File Checking & Returning -1**: The prompt asks to "Have the child send an error code (like -1) through the pipe if a file cannot be found". Inside our `count_words_in_file` function, if `fopen()` fails to open a file (meaning it doesn't exist or is unreadable), the function immediately returns `-1`.
* **Error Code Transmission (`write(pipes[i][1], ...)`)**: In the child process block, whatever our counting function returns (either the real word count or the `-1` error code) gets sent straight through the pipe to the parent.
* **Error Handling by Parent**: When the parent reads the data from the pipe, it checks `if (result == -1)`. If it detects the error code, it prints an error message for that specific file and safely skips adding it to the `grand_total`.

#### 4. General Good Practices Included
* **Closing Unused Pipe Ends**: To prevent the program from deadlocking (freezing), the children close the pipe ends they aren't using, and the parent immediately closes its write ends.
* **Waiting for Children (`waitpid`)**: The parent loops through all child PIDs and waits for them to terminate. This is crucial in OS projects to prevent the children from becoming "zombie" processes after they finish their task.

---

## 3. The Build Script (`Makefile`)

Open `Makefile` and paste the exact code below. 
*(Note: Ensure the indented lines under targets like `all:`, `$(TARGET):`, and `clean:` use **Tabs**, not spaces!)*

```makefile
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
TARGET  = word_counter
SRC     = word_counter.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

test: $(TARGET)
	@echo "--- Test 1: Count words in Makefile (single file) ---"
	./$(TARGET) Makefile
	@echo ""
	@echo "--- Test 2: Count words in multiple files ---"
	./$(TARGET) Makefile $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all test clean
```

### Why This Code? (Makefile Breakdown)
- **Variables (`CC`, `CFLAGS`, `TARGET`, `SRC`)**: Defines the compiler (`gcc`), standard flags (warnings, C11 standard, debugging), the name of the final executable (`word_counter`), and the source code file.
  - *Reason*: Centralizing these variables makes it very easy to change the compiler or add flags later without modifying the whole file.
- **`all: $(TARGET)`**: The default target.
  - *Reason*: When you just type `make`, it looks for the first target (which is `all`), and ensures the `word_counter` executable is built.
- **`$(TARGET): $(SRC)`**: The actual compilation rule.
  - *Reason*: It tells `make` that the executable depends on `word_counter.c`. If `word_counter.c` is updated, running `make` will re-compile it using `gcc -Wall -Wextra -std=c11 -g -o word_counter word_counter.c`.
- **`test:`**: A custom target to run the program.
  - *Reason*: It provides a quick, automated way to verify the code is working correctly by feeding it its own source files (`Makefile` and `word_counter.c`).
- **`clean:`**: A cleanup target.
  - *Reason*: Allows you to easily delete the compiled binary by running `make clean`, keeping your workspace tidy.
- **`.PHONY:`**: Declares targets that are not actual files.
  - *Reason*: Prevents conflicts if you happen to create a file named `clean` or `test` in the same directory.

---

## 4. How To Compile and Run

To compile the code, open your terminal in the same directory as the files and type:
```bash
make
```

To run a quick test proving the script works using the Makefile and C file themselves as the input words:
```bash
make test
```

To run the program normally against any text file:
```bash
./word_counter essay.txt notes.txt
```
