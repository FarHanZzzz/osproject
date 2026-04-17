# Task A — Word Counter
## How the Code Works — Complete Explanation

> **Source:** `word_counter.c` | **Language:** C | **OS Concepts:** fork, pipe, waitpid

---

## What This Program Does

Given a list of file names, this program counts the total number of words across all files.
It does this **in parallel**: each file gets its own child process and its own pipe.

```
$ ./word_counter essay.txt notes.txt readme.txt
  essay.txt       1234 word(s)
  notes.txt        567 word(s)
  readme.txt       890 word(s)
Grand Total: 2691 word(s)
```

---

## How It Works — Step by Step

### 1. Argument Parsing (Lines 68–80)

```c
if (argc < 2) {
    fprintf(stderr, "Usage: %s <file1> [file2] ...\n", argv[0]);
    return EXIT_FAILURE;
}
int num_files = argc - 1;
```

- `argc` = total arguments (including the program name itself)
- `argv[0]` = `"./word_counter"`, `argv[1]` = first filename, etc.
- `num_files = argc - 1` = number of actual files to process
- If no files given, print usage and exit with error code 1

### 2. Pipe + Fork Loop (Lines 93–146) — THE CORE

For each file, we do TWO things in order:

#### Step 2a: Create a pipe

```c
pipe(pipes[i]);
```

This creates a one-way communication channel in the kernel:
- `pipes[i][0]` = **read end** (parent will use this)
- `pipes[i][1]` = **write end** (child will use this)

The pipe is created BEFORE `fork()` so both parent and child inherit it.

#### Step 2b: Fork a child process

```c
pids[i] = fork();
```

`fork()` duplicates the entire process:
- Returns **0** in the child
- Returns the **child's PID** in the parent
- Returns **-1** on failure

After fork, both parent and child have ALL pipe ends open. This is dangerous — we must close the ones we don't need.

### 3. What the Child Does (Lines 108–136)

```c
if (pids[i] == 0) {
    // 1. Close pipe ends we don't need
    close(pipes[i][0]);  // We write, not read

    // 2. Count words in our assigned file
    int result = count_words_in_file(argv[i + 1]);

    // 3. Send result through pipe
    write(pipes[i][1], &result, sizeof(result));

    // 4. Clean up and exit
    close(pipes[i][1]);
    exit(EXIT_SUCCESS);
}
```

**Why close the read end?**
A pipe has two rules:
1. If ALL write ends are closed, `read()` returns 0 (EOF)
2. If ANY write end is open, `read()` blocks waiting for data

If the child keeps the read end open, it wastes a file descriptor.

**Why send -1 on error?**
If `fopen()` fails (file not found), the function returns -1. The parent reads this
-1 and knows the file couldn't be opened — this is the "error reporting" enhancement.

### 4. What the Parent Does After Forking

```c
close(pipes[i][1]);  // Close write end immediately
```

**This is the single most important line in the program.**

If the parent keeps the write end open, then after the child exits and its write end
closes, the parent's `read()` on `pipes[i][0]` will **still block** — because the OS
sees there's still a writer (the parent itself). This causes a **deadlock**.

### 5. Collecting Results (Lines 152–173)

```c
for (int i = 0; i < num_files; i++) {
    int result = 0;
    ssize_t bytes_read = read(pipes[i][0], &result, sizeof(result));

    if (bytes_read <= 0) {
        // No data — something went wrong
    } else if (result == -1) {
        // Child couldn't open the file
    } else {
        // Got a valid word count
        grand_total += result;
    }
    close(pipes[i][0]);
}
```

**Why read BEFORE waitpid?**
If we called `waitpid()` first, and a child's write fills the pipe buffer (64KB), the child
blocks on `write()`. The parent blocks on `waitpid()`. Both wait on each other → **deadlock**.

### 6. Reaping Children (Lines 180–189)

```c
for (int i = 0; i < num_files; i++) {
    waitpid(pids[i], &status, 0);
}
```

Every `fork()` must have a matching `waitpid()`. Without it, finished children become
**zombie processes** — they sit in the process table eating up PID slots. The kernel
keeps zombies around because the parent might want to read their exit code.

### 7. The Word Counter Itself (Lines 44–63)

```c
while (fscanf(fp, "%4095s", word) == 1) {
    count++;
}
```

`fscanf` with `%s` automatically:
1. Skips all whitespace (spaces, tabs, newlines)
2. Reads one non-whitespace token
3. Returns 1 on success, EOF on end-of-file

`%4095s` limits reading to 4095 chars (prevents buffer overflow — the array is 4096).

---

## Key OS Concepts

| Concept | Where in Code | What It Does |
|---------|---------------|--------------|
| `fork()` | Line 100 | Creates an identical copy of the process |
| `pipe()` | Line 95 | Creates a kernel-managed IPC channel (4KB–64KB buffer) |
| `close()` | Lines 120–123, 145 | Releases file descriptors; CRITICAL for pipe EOF |
| `read()` | Line 158 | Reads data from pipe; blocks until data or all writers close |
| `write()` | Line 131 | Sends data into pipe; blocks if buffer full |
| `waitpid()` | Line 182 | Waits for a specific child; prevents zombies |
| `exit()` | Line 136 | Terminates the child process cleanly |

---

## Error Handling

| Scenario | What Happens |
|----------|-------------|
| File not found | Child prints error to stderr, sends -1 through pipe |
| `fork()` fails | Parent prints error, exits |
| `pipe()` fails | Parent prints error, exits |
| No arguments | Prints usage, exits with code 1 |
| Too many files (>64) | Prints error, exits |

---

## Test Results (Verified)

| Test | Input | Expected | Actual | Status |
|------|-------|----------|--------|--------|
| Single file | `word_counter.c` | 816 | 816 | ✅ PASS |
| Multiple files | `word_counter.c Makefile` | 936 | 936 | ✅ PASS |
| Missing file | `Makefile ghost.txt word_counter.c` | 936 (successful) | 936 | ✅ PASS |
| Empty file | `/tmp/empty.txt` | 0 | 0 | ✅ PASS |
| One word | `/tmp/oneword.txt` | 1 | 1 | ✅ PASS |
| No args | *(none)* | Usage message, exit 1 | ✓ | ✅ PASS |
| `wc -w` match | All files | Exact match | ✓ | ✅ PASS |

---

## Viva Questions & Answers

**Q: What happens if you forget to close the write end of the pipe in the parent?**
A: `read()` in the parent will block forever. The OS thinks there's still a writer, so it waits
for more data. This is a deadlock — the parent waits on the pipe, the child is already done.

**Q: What is a zombie process?**
A: A process that has exited but hasn't been "reaped" by its parent via `wait()`/`waitpid()`.
Its PID and exit status remain in the kernel's process table. Use `waitpid()` to prevent this.

**Q: Why use one pipe per child instead of one shared pipe?**
A: If multiple children write to the same pipe, their `int` values can interleave. You can't
tell which child sent which result. One pipe per child eliminates this race condition entirely.

**Q: What's the difference between `pipe()` and a named pipe (FIFO)?**
A: `pipe()` creates an anonymous, in-memory pipe that only related processes (parent/child) can use.
A named pipe (FIFO) has a filesystem path and can be used between unrelated processes.
