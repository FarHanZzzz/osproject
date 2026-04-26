# Task A: Word Counter — The Viva Defense Walkthrough

Imagine you're walking into the viva. The examiner asks: *"How did you approach Task A, and why did you write the code this way?"* 

This document is your exact thought process, from reading the question for the very first time to writing the final line of code. Read this as a story—it will lock the concepts into your brain.

---

## Step 1: Reading the Question & Finding the Prerequisites
When I first saw the PDF for Task A, the prompt asked to:
1. Count words in multiple text files.
2. Use **multiple child processes**.
3. Send the result back to the parent using **Inter-Process Communication (IPC)**.

**My immediate thought process:**
"Okay, if I just wrote a normal C program, I'd open the file, loop through it, and count. But the prompt strictly forbids that simple approach. I have to create 'clones' of my program to do the work simultaneously."

**Prerequisites I had to learn before coding:**
1.  **How to clone a program?** → I learned about `fork()`. It literally splits the execution into two. One becomes the "Parent" and one becomes the "Child".
2.  **How do they talk?** → Processes don't share memory. If a child updates a variable `count = 5`, the parent *does not see it*. I needed a bridge.
3.  **What bridge to use?** → The prompt suggested IPC. The simplest IPC for parent-to-child data is a **Pipe**. I learned that a pipe is like a one-way walkie-talkie.

---

## Step 2: The Mental Blueprint (ELI5)
Before touching the keyboard, I drew this mental picture:
*   **Me (The Parent):** I have two text files. I need the total word count. I don't want to count them myself.
*   **My Setup:** I buy two walkie-talkies (Pipes). I keep the "listening" ends (`pipe[0]`) and hand out the "talking" ends (`pipe[1]`).
*   **The Delegation:** I clone myself twice (`fork()`). Child 1 gets file 1. Child 2 gets file 2.
*   **The Execution:** The children read their files, count the words, shout the final number into the walkie-talkie (`write`), and then die (`exit()`).
*   **The Result:** I wait for them to finish, listen to my walkie-talkies (`read`), add the two numbers, and print the total.

---

## Step 3: Writing the Code (Line-by-Line Rationale)

Here is exactly why I wrote every piece of code.

### 1. The Headers
```c
#include <stdio.h>    // I need this for printf and reading files (fopen, fscanf).
#include <stdlib.h>   // I need this for the exit() command so children can terminate.
#include <unistd.h>   // "UNIX Standard" - This gives me the magic OS tools: fork(), pipe(), read(), write().
#include <sys/wait.h> // I need this for waitpid() so the parent can wait for children.
```

### 2. The Word Counting Logic
*Why write a helper function?* Because I'm doing the exact same thing twice. Code reuse!
```c
int count_words(const char *filename) {
    FILE *fp = fopen(filename, "r"); // "r" means read mode.
    int count = 0;
    char word[256]; 
```
*Wait, how do I actually count words?* I could read character by character and check for spaces, but that's messy. I chose `fscanf(fp, "%255s", word)`.
*   **Why?** `fscanf` with `%s` automatically skips all whitespace (spaces, tabs, newlines). It grabs chunks of text. Every time it grabs a chunk, that's one word! It returns 1 if it succeeds.
```c
    while (fscanf(fp, "%255s", word) == 1) {
        count++; // Found a word, increment!
    }
    fclose(fp); // ALWAYS close files to free OS resources.
    return count;
}
```

### 3. The Main Setup (Pipes)
```c
int pipe1[2]; // Array of 2 integers. Why? A pipe needs an 'in' door and an 'out' door.
int pipe2[2];
pipe(pipe1);  // The OS hooks up the plumbing for pipe 1.
pipe(pipe2);  // The OS hooks up the plumbing for pipe 2.
```
*Note for Viva:* `pipe1[0]` is for **reading**. `pipe1[1]` is for **writing**. (Think: 0 looks like an open mouth receiving food, 1 looks like a pen writing).

### 4. Cloning and Executing (Child 1)
```c
pid_t pid1 = fork(); // BAM! We are now two identical processes running at exactly the same time.
```
*How do they know who is who?* `fork()` gives the Child a return value of `0`. It gives the Parent the Child's real ID.
```c
if (pid1 == 0) {
    // I AM THE CHILD.
    close(pipe1[0]); // I am only sending data. I don't need to read. Close it!
    
    int words = count_words("sample1.txt"); // Do the heavy lifting.
    
    // Send the data through the pipe.
    // Why &words? write() wants a memory address. 
    // Why sizeof(words)? write() needs to know exactly how many bytes to push down the tube.
    write(pipe1[1], &words, sizeof(words)); 
    
    close(pipe1[1]); // Done talking. Hang up the walkie-talkie.
    exit(0);         // CRITICAL: If I don't exit here, the child will continue running the rest of the main() function and try to fork Child 2!
}
```

### 5. Managing the Children (The Parent)
After launching both children, the parent must do some housekeeping.
```c
close(pipe1[1]); // The parent is only READING. It MUST close its write ends.
close(pipe2[1]);
```
*Viva Question:* **Why MUST the parent close its write ends?**
*Answer:* If the parent leaves its write end open, the pipe thinks "Hey, someone might still write to me." When the parent later calls `read()`, it will sit there completely frozen (blocking) forever, waiting for more data that will never come.

```c
waitpid(pid1, NULL, 0); // Parent goes to sleep until Child 1 dies.
waitpid(pid2, NULL, 0); // Parent goes to sleep until Child 2 dies.
```
*Viva Question:* **Why use waitpid?**
*Answer:* If the parent finishes and exits before the children, the children become "Orphans". Worse, when the children finish, they become "Zombies" holding onto OS resources because their parent isn't there to read their exit status.

### 6. Harvesting the Results
```c
int result1 = 0, result2 = 0;
read(pipe1[0], &result1, sizeof(result1)); // Suck the bytes out of the pipe directly into the result variable.
read(pipe2[0], &result2, sizeof(result2));

close(pipe1[0]); // Clean up!
close(pipe2[0]);

printf("Total Words: %d\n", result1 + result2); // Done!
```

---

## Step 4: Problems Faced & "What Ifs" (Viva Goldmine)

If the examiner asks: *"What problems did you face?" or "What else could you have tried?"*, here is how you answer confidently:

### 1. The "Infinite Freeze" Bug
**The Problem:** When I first wrote this, the program just froze at the end and never printed the total.
**The Cause:** I forgot to have the parent close its write ends (`close(pipe1[1])`). The `read()` function was waiting forever because it thought the parent itself might write something.
**The Fix:** Realizing that pipes are reference-counted by the OS. Every single write end across ALL processes must be closed before `read()` realizes the transmission is over.

### 2. The "Fork Bomb" Risk
**The Problem:** I initially forgot to put `exit(0);` at the end of the `if (pid == 0)` blocks.
**The Consequence:** Child 1 fell through the `if` statement, reached the second `fork()`, and cloned *itself*. Suddenly I had grandchildren processes, and everyone was writing to the same pipes causing total chaos. 
**The Fix:** Enforcing strict boundaries. A child does its job, and then forcefully `exit(0)`.

### 3. Why Not Use Threads Instead of Processes?
**The "What If":** "Why didn't you just use `pthread_create`?"
**Your Answer:** "The assignment explicitly required 'child processes'. But conceptually, if I used threads, I wouldn't even need pipes! Threads share the exact same memory space. I could have just given them a global integer pointer to update. Because `fork()` creates a completely separate memory space (a deep copy), they couldn't share variables, forcing me to use IPC via pipes to communicate."

### 4. Why Not Use Shared Memory?
**The "What If":** "Pipes are one way to do IPC. What about Shared Memory (`shmget`)?"
**Your Answer:** "Shared memory is much faster for huge amounts of data because you don't have to copy bytes through the kernel. However, for just passing back a single integer (the word count), it's total overkill. Shared memory requires setting up semaphores or mutexes to prevent race conditions. A pipe automatically handles the synchronization for me—`read()` safely waits until data is available."
