# Task A — Word Counter: Viva Preparation

---

## 🧠 What Is The Problem?

The assignment asks: *"Count the total number of words across multiple text files."*

The **twist** is you cannot just write a simple loop in one program. You must use **multiple child processes** — one per file — and have them **send their results back to the parent** through pipes.

This tests your understanding of:
- How the OS creates new processes (`fork`)
- How processes communicate with each other (`pipe`)
- How the parent collects and combines results (`waitpid`, `read`)

---

## 💬 How To Explain The Problem (Your Opening Line)

> *"The problem is: I need to count words in multiple files at the same time using separate processes, not just one process doing it one by one. Each child process handles one file and sends its result back to the parent through a pipe."*

---

## ⚙️ How Your Code Solves It — Step By Step

### Step 1 — Dynamic File Input
- You accept file names from the command line: `./word_counter a.txt b.txt c.txt`
- If no files given, you fall back to `sample1.txt` and `sample2.txt`
- This satisfies the **"Dynamic Tasks"** enhancement in the PDF

### Step 2 — Create a Pipe BEFORE Forking
- For each file, you call `pipe(pipes[i])` first, then `fork()`
- Why before? Because `fork()` copies the process — if the pipe exists before fork, both parent and child automatically share the same pipe file descriptors
- After fork, the child only needs the **write** end, parent only needs the **read** end

### Step 3 — Fork One Child Per File
- `fork()` is called inside a loop — one child per file
- Each child:
  1. Closes its read end (it only writes)
  2. Closes all other pipes from previous iterations (prevents deadlock)
  3. Counts words using `count_words()`
  4. Writes the result (or -1 on error) into its pipe
  5. Calls `exit(0)` to terminate

### Step 4 — Parent Reads Results
- After the loop, the parent reads from each pipe using `read()`
- If result is -1 → file not found (Error Reporting enhancement)
- Otherwise → add to total

### Step 5 — Zombie Prevention
- `waitpid()` is called for each child PID
- Without this, finished children become "zombie processes" lingering in the process table

---

## 🔥 Challenges & What You'd Say You Faced

> *"The trickiest part was managing the pipe file descriptors carefully. If a child keeps the read end of a pipe open, the parent's `read()` will block forever waiting for data that never comes — this is called a deadlock. I had to make sure every child closes all pipe ends it doesn't need."*

> *"Another challenge was closing pipes from previous loop iterations inside new child processes. When you fork in a loop, child i inherits all pipes created in iterations 0 through i-1. If child i doesn't close those old pipes, the parent's `read()` on those old pipes will never return."*

---

## 🤔 Choices You Made & Why

| Choice | Why |
|--------|-----|
| One pipe per file (not one shared pipe) | Keeps results separated — parent knows which count belongs to which file |
| `pipe()` called before `fork()` | Only way for parent and child to share the same pipe |
| Closing unused pipe ends in children | Prevents deadlock |
| `waitpid()` after `read()` | Parent already has all results, so we can wait for cleanup safely |
| Error code `-1` through the pipe | Elegant — the pipe already exists, no extra channel needed for errors |

---

## 📚 Concepts You Need To Know

### `fork()`
- Creates a **child process** that is an exact copy of the parent
- After `fork()`, both parent and child run from the same line
- `fork()` returns `0` inside the child, and the child's PID inside the parent
- Think of it as a photocopier for processes

### `pipe()`
- Creates a one-way communication channel between two processes
- Has two ends: `[0]` = read end, `[1]` = write end
- Data flows one direction only: write in → read out
- Like a physical tube — you pour in one side, pull out the other

### `read()` and `write()`
- `write(fd, &data, size)` — push bytes into the pipe
- `read(fd, &data, size)` — pull bytes out of the pipe
- `read()` **blocks** (waits) until data is available — this is why we need to close unused ends

### `waitpid()`
- Parent calls this to "collect" a finished child
- Without it: child finishes but its entry stays in the process table → "zombie process"
- `waitpid(pid, &status, 0)` waits for that specific child

### Inter-Process Communication (IPC)
- Any mechanism that lets two processes exchange data
- Pipes are one type of IPC
- Others include: shared memory, message queues, sockets, signals

### Deadlock (in pipe context)
- Happens when: parent is waiting to read, but child is waiting to write... but no one moves
- Prevented by closing all unused pipe ends on both sides

### Zombie Process
- A child that has exited but whose exit status hasn't been collected by the parent yet
- It takes up a slot in the process table
- Fixed with `waitpid()`

---

## ✅ Quick Viva Q&A

**Q: Why do you use fork() and not just a function call?**
> Because the assignment requires processes (not just functions). A child process is a fully separate unit of execution with its own memory — multiple children can run truly in parallel on multi-core systems.

**Q: Why can't you just use a global variable instead of a pipe?**
> After `fork()`, the child gets a COPY of the parent's memory. If the child modifies a variable, it modifies its own copy — the parent's copy is unchanged. Pipes are the correct IPC mechanism for sending data between separate processes.

**Q: What happens if you forget to close the write end in the parent?**
> The parent's `read()` call will block forever. The OS only sends EOF to the read end when ALL write ends are closed. If the parent still holds the write end open, the OS thinks someone might still write, so `read()` keeps waiting.

**Q: What is the Error Reporting enhancement?**
> If a file can't be opened, `count_words()` returns -1. The child sends -1 through the pipe. The parent checks for -1 and prints "ERROR (file not found)" instead of a word count.
