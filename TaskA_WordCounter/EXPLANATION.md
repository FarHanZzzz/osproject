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

If no files are provided, it uses `sample1.txt` and `sample2.txt` by default.

---

## 📖 Code Walkthrough

### Part 1: Dynamic File Input (Enhancement)

```c
int main(int argc, char *argv[]) {
    if (argc > 1) {
        // User provided files on the command line
        num_files = argc - 1;
        for (int i = 0; i < num_files; i++)
            files[i] = argv[i + 1];
    } else {
        // Fall back to default sample files
        files[0] = "sample1.txt";
        files[1] = "sample2.txt";
    }
}
```

🎯 **Dynamic Tasks:** The user can specify ANY number of files (up to 10) via the command line. The program forks one child per file automatically. No hardcoding!

---

### Part 2: Error Reporting (-1 Through Pipe)

```c
int count_words(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return -1;  // Error code! Sent through pipe to parent
    // ... count words ...
    return count;
}
```

🚨 **Error Reporting:** If a file doesn't exist, the child sends `-1` through the pipe instead of a word count. The parent checks for `-1` and reports the error:

```c
if (result == -1) {
    printf("  %s : ERROR (file not found)\n", files[i]);
    errors++;
} else {
    printf("  %s : %d words\n", files[i], result);
    total += result;
}
```

---

### Part 3: Dynamic fork() Loop

```c
for (int i = 0; i < num_files; i++) {
    pipe(pipes[i]);      // Create pipe BEFORE fork
    pids[i] = fork();    // Clone the process

    if (pids[i] == 0) {
        // Child: close unused pipe ends, count words, write result, exit
        close(pipes[i][0]);
        for (int j = 0; j < i; j++) { close(pipes[j][0]); close(pipes[j][1]); }
        int result = count_words(files[i]);
        write(pipes[i][1], &result, sizeof(result));
        close(pipes[i][1]);
        exit(0);
    }
    close(pipes[i][1]);  // Parent closes write end
}
```

🧬 **`fork()` in a loop** creates one child per file dynamically. Each child gets its own pipe for sending results back. The key rules:
- `pipe()` BEFORE `fork()` — so both parent and child share the pipe
- Children close ALL other pipes — prevents deadlock
- Parent closes write ends — so `read()` doesn't block forever

---

### Part 4: Zombie Prevention

```c
for (int i = 0; i < num_files; i++) {
    waitpid(pids[i], &status, 0);
}
```

🧟 **`waitpid()` cleans up dead children.** Without it, terminated child processes become "zombies" — dead processes still taking up PID slots.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| `fork()` | Photocopier for programs. Makes an identical copy. |
| `pipe()` | A one-way tube between two processes. One pours, one drinks. |
| `waitpid()` | Teacher says "Kid, are you done?" and waits. Prevents zombies. |
| Zombie | A dead process that hasn't been cleaned up yet. |
| IPC | Any way for two processes to talk. Pipes are one type. |
| Deadlock | Two people each waiting for the other to go first. |
| Error Code (-1) | Child sends -1 through pipe if file not found. Parent detects this. |
| Dynamic Tasks | Number of children determined at runtime from command-line args. |
