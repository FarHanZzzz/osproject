# Task A: Word Counter (Concepts First!)

Before diving into the code, let's understand the two massive Operating System concepts that make this program work: **Forking** and **Pipes**. 

---

## 🧠 Core Concept 1: What is `fork()`?
Imagine you are at a desk with a huge stack of paperwork. You realize it will take you 10 hours to read everything. But wait! You cast a magic spell called `fork()`. 

Instantly, a **clone** of you appears at the exact same desk. The clone has the same memories, the same pen, and the same stack of paperwork. 
- You are the **Parent Process**.
- The clone is the **Child Process**.

Now, instead of reading the whole stack yourself, you read half, and the clone reads the other half. You both finish in 5 hours instead of 10! In Operating Systems, `fork()` creates an identical duplicate of a running program so they can work on tasks simultaneously.

## 🧠 Core Concept 2: What is `pipe()`?
So you and your clone just finished counting your respective stacks of paper. How does the clone tell you their final number? You can't just talk to each other (processes have isolated memories). 

Enter the **Pipe**. A `pipe()` is literally an invisible tube connecting you and your clone. 
- The clone writes their final count on a piece of paper and shoves it into the **write end** of the tube.
- You stand at the **read end** of the tube, catch the paper, read the number, and add it to your total.

In OS terms, a pipe allows **Inter-Process Communication (IPC)** so a Child can send data back to the Parent.

---

## 💻 How the Code Works

This program is extremely simple and **hardcoded** to read exactly two files: `sample1.txt` and `sample2.txt`.

### Step-by-Step Walkthrough

1. **Creating the Pipes**
   ```c
   int pipe1[2]; 
   int pipe2[2];
   pipe(pipe1);
   pipe(pipe2);
   ```
   Before we create our clones, we build two communication tubes. `pipe1` will be for the first clone, and `pipe2` will be for the second clone.

2. **Creating Clone 1 (Forking)**
   ```c
   pid_t pid1 = fork();
   if (pid1 == 0) {
       int words = count_words("sample1.txt");
       write(pipe1[1], &words, sizeof(words));
       exit(0);
   }
   ```
   We call `fork()`. If `pid1 == 0`, it means *we are the clone*. We immediately count the words in `sample1.txt`, shove the result into `pipe1[1]` (the write end), and then "die" (`exit(0)`).

3. **Creating Clone 2 (Forking)**
   ```c
   pid_t pid2 = fork();
   if (pid2 == 0) {
       int words = count_words("sample2.txt");
       write(pipe2[1], &words, sizeof(words));
       exit(0);
   }
   ```
   The parent process calls `fork()` a second time. This second clone counts the words in `sample2.txt`, sends the answer through `pipe2`, and exits.

4. **Waiting and Reading**
   ```c
   waitpid(pid1, NULL, 0);
   waitpid(pid2, NULL, 0);
   
   read(pipe1[0], &result1, sizeof(result1));
   read(pipe2[0], &result2, sizeof(result2));
   ```
   The Parent process waits for both clones to finish their work (`waitpid`). Then, the parent walks over to the read ends of the tubes (`pipe1[0]` and `pipe2[0]`), pulls out the numbers, and prints the grand total!
