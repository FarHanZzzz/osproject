# 🧮 Task A — Word Counter (ELI5 Edition)

## The Story

Imagine you're a **teacher** with a stack of essays. You need to count every word in every essay. You could do it yourself (boring, slow), OR you could hand each essay to a **student helper** and say *"count the words and tell me the number."*

That's exactly what this program does:
- **You** = the parent process
- **Student helpers** = child processes (created with `fork()`)
- **Telling you the number** = sending it through a pipe

---

## 🎯 The Big Picture

```
YOU (parent)
  │
  ├── "Hey Kid 1, count words in essay1.txt"  ──→  Kid 1 counts ──→ sends 500 back through a tube
  ├── "Hey Kid 2, count words in essay2.txt"  ──→  Kid 2 counts ──→ sends 300 back through a tube
  └── "Hey Kid 3, count words in essay3.txt"  ──→  Kid 3 counts ──→ sends 200 back through a tube
  
YOU add them up: 500 + 300 + 200 = 1000 total words! 🎉
```

Each kid gets their own private **tube** (pipe). Nobody can mess with anyone else's tube.

---

## 🔧 How to Run It

```bash
gcc -o word_counter word_counter.c
./word_counter file1.txt file2.txt file3.txt
```

---

## 📖 Code Walkthrough — The Fun Version

### Part 1: The Word Counting Helper

```c
int count_words(const char *filename) {
    FILE *fp = fopen(filename, "r");       // Open the essay
    if (!fp) return -1;                     // Can't find it? Return -1 = "error!"

    int count = 0;
    char word[4096];

    while (fscanf(fp, "%4095s", word) == 1) {   // Read one word at a time
        count++;                                  // Tally it up
    }

    fclose(fp);
    return count;    // "Teacher, I counted 500 words!"
}
```

🍎 **Think of `fscanf` like a kid reading a book word by word.** It skips spaces, tabs, and newlines automatically. Each time it finds a word, it says "got one!" and moves on. When there are no more words, it stops.

**Why `%4095s`?** Imagine the word buffer is a box that fits 4096 letters. If we don't set a limit, a ridiculously long "word" (like 10,000 characters with no spaces) would overflow the box and crash the program. `%4095s` = "only grab up to 4095 letters" (leaving 1 spot for the `\0` terminator).

---

### Part 2: Setting Up the Tubes (Pipes)

```c
int pipes[MAX_FILES][2];    // One tube per file
pid_t pids[MAX_FILES];      // Remember each kid's ID
```

🧃 **Each pipe is like a juice box straw** — it has two ends:
- `pipes[i][0]` = the **drinking end** (parent reads from here)
- `pipes[i][1]` = the **pouring end** (child writes into here)

---

### Part 3: The Magic of fork()

```c
for (int i = 0; i < num_files; i++) {
    pipe(pipes[i]);        // Step 1: Make the tube FIRST
    pids[i] = fork();      // Step 2: Clone yourself
```

🧬 **`fork()` is like a photocopier for programs.** You press the button and suddenly there are TWO identical copies of the program running at the same time. The original is the **parent**, the copy is the **child**.

**How do you tell them apart?**
- `fork()` returns `0` to the child → "I'm the copy!"
- `fork()` returns the child's PID to the parent → "I made a copy, their ID is 12345"

**Why `pipe()` BEFORE `fork()`?** Because `fork()` copies everything — including open pipe ends. If we create the pipe first, both parent and child automatically get access to the same tube. If we create the pipe after, only one of them has it!

---

### Part 4: What the Kid Does

```c
if (pids[i] == 0) {
    // I'm the child! 👶

    close(pipes[i][0]);    // I don't need the drinking end, I only pour

    // Close ALL other kids' tubes (very important!)
    for (int j = 0; j < i; j++) {
        close(pipes[j][0]);
        close(pipes[j][1]);
    }

    int result = count_words(argv[i + 1]);           // Count words
    write(pipes[i][1], &result, sizeof(result));      // Pour the answer into my tube

    close(pipes[i][1]);    // Done pouring, close the tube
    exit(0);               // "Bye teacher, I'm done!"
}
```

🚰 **Why close other kids' tubes?** Imagine Kid 2 holds onto Kid 1's tube. When Kid 1 finishes and closes their tube, the parent tries to drink from it — but the OS says *"wait, Kid 2 still has the pouring end open, maybe more juice is coming!"* The parent waits forever. **Deadlock!** So every kid must close every tube they don't own.

---

### Part 5: What the Teacher (Parent) Does

```c
close(pipes[i][1]);    // Close MY pouring end — I only drink!
```

☠️ **This one line prevents the #1 beginner mistake.** If the parent keeps the pouring end open, then when the parent tries to drink (`read()`), the OS thinks: *"Hey, the parent itself could pour more juice! Better wait..."* The parent waits on itself forever = **deadlock**.

**Rule: Always close the pipe end you don't use. Always.**

---

### Part 6: Collecting the Answers

```c
for (int i = 0; i < num_files; i++) {
    int result = 0;
    read(pipes[i][0], &result, sizeof(result));    // Drink from kid i's tube
    total += result;
    close(pipes[i][0]);                             // Done drinking, close it
}
```

**Why `read()` BEFORE `waitpid()`?** Imagine Kid 1 is trying to pour a LOT of juice but the tube is full (pipe buffer = 64KB). Kid 1 is stuck waiting for the parent to drink some. But the parent called `waitpid()` and is stuck waiting for Kid 1 to finish. Both are waiting on each other = **deadlock**. Reading first drains the tube so the kid can finish.

---

### Part 7: Cleaning Up (No Zombies!)

```c
for (int i = 0; i < num_files; i++) {
    waitpid(pids[i], &status, 0);    // "Kid i, are you done? OK, you can leave."
}
```

🧟 **What's a zombie?** When a child process dies, it doesn't fully disappear. Its exit code stays in the OS like a ghost, waiting for the parent to acknowledge it. Until `waitpid()` is called, the dead child is a **zombie process** — using up a PID slot for nothing. `waitpid()` cleans it up.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| `fork()` | Photocopier for programs. Makes an identical copy. |
| `pipe()` | A one-way tube between two processes. One pours, one drinks. |
| `waitpid()` | Teacher says "Kid, are you done?" and waits. Prevents zombies. |
| Zombie | A dead process that hasn't been cleaned up yet. |
| IPC | Any way for two processes to talk. Pipes are one type. |
| Deadlock | Two people each waiting for the other to go first. Nobody moves. |
