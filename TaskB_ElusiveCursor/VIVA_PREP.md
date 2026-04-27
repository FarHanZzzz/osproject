# Task B — The Elusive Cursor: Viva Preparation

---

## 🧠 What Is The Problem?

The assignment asks: *"Write a background Windows program that makes the mouse cursor jump or jitter randomly, without showing any visible window, and give yourself a secret hotkey to kill it."*

This tests your understanding of:
- How Windows programs handle events (the Win32 message system)
- Five specific Win32 API functions the PDF lists by name
- How to create invisible background applications

---

## 💬 How To Explain The Problem (Your Opening Line)

> *"The problem is to write a prank application that runs invisibly in the background and jitters the mouse cursor randomly every 400 milliseconds. It cannot have a visible window, and the only way to stop it is pressing Ctrl+Shift+Q — a global hotkey I registered."*

---

## ⚙️ How Your Code Solves It — Step By Step

### Step 1 — Get the screen size with `GetSystemMetrics()`
- `GetSystemMetrics(SM_CXSCREEN)` → screen width in pixels
- `GetSystemMetrics(SM_CYSCREEN)` → screen height in pixels
- Store them globally so you can clamp the cursor to stay on-screen

### Step 2 — Register a Window Class and Create a Hidden Window
- Windows needs a "window" to send messages to, even if it's invisible
- `HWND_MESSAGE` as the parent creates a **message-only window** — no taskbar icon, not visible at all
- This is how you run truly in the background

### Step 3 — Register the Kill Hotkey with `RegisterHotKey()`
- `RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 'Q')`
- This tells Windows: *"Watch for Ctrl+Shift+Q globally — send me `WM_HOTKEY` when it happens"*
- Works even when the user is in another application

### Step 4 — Start the Timer with `SetTimer()`
- `SetTimer(hwnd, 1, 400, NULL)` — fires a `WM_TIMER` message every 400ms
- This is the "annoying interval" the PDF mentions

### Step 5 — The Message Loop
- `GetMessage()` waits for events. `DispatchMessage()` routes them to `WndProc()`
- The loop runs forever until `PostQuitMessage()` is called

### Step 6 — WndProc Handles Events
- `WM_TIMER`: Get cursor position → add random jump → clamp to screen → `SetCursorPos()`
- `WM_HOTKEY`: Call `PostQuitMessage(0)` → loop exits → program terminates

---

## 🔥 Challenges & What You'd Say You Faced

> *"The biggest challenge was understanding that Windows GUI programs don't work like normal C programs. There's no `while(true)` loop — instead, everything is event-driven. The program sits idle until Windows sends it a message. I had to understand the WndProc callback pattern before anything else made sense."*

> *"I also had to figure out how to make the program truly invisible. Normally when you create a window, it shows in the taskbar. Using `HWND_MESSAGE` as the parent window makes it a message-only window — it exists in the system but has no visual presence at all."*

> *"Since this is a Windows-only program (.exe), it cannot be run or tested on Linux. The compilation uses MinGW (a Windows GCC port) with specific flags: `-lgdi32 -luser32 -mwindows` — these link to the Windows graphics and user-interface libraries. The `-mwindows` flag is critical: without it, the program opens a console window, breaking the invisibility."*

---

## 🤔 Choices You Made & Why

| Choice | Why |
|--------|-----|
| `HWND_MESSAGE` as parent | Makes window invisible — no taskbar entry, no visible UI |
| Timer interval of 400ms | Frequent enough to be annoying, slow enough to be clearly visible |
| Jump of ±25 pixels | Small enough to be subtle, large enough to be frustrating |
| `RegisterHotKey()` with global scope | Works even when another app is in the foreground — essential for kill switch |
| `WinMain` instead of `main` | Windows GUI programs require `WinMain` as their entry point |
| `rand() % 51 - 25` | Gives a uniform distribution from -25 to +25 |

---

## 📚 Concepts You Need To Know

### Win32 API
- **API** = Application Programming Interface — a set of functions provided by Windows
- Win32 API is the low-level interface to the Windows operating system
- Every function like `SetCursorPos`, `GetCursorPos`, `RegisterHotKey` is part of this
- You include `<windows.h>` to access all of them

### Event-Driven Programming
- Normal programs: you write code that runs top to bottom
- Windows GUI programs: you write "handlers" and wait for events
- Events (messages) include: timer fired, key pressed, mouse moved, window closed
- `WndProc` is your event handler — Windows calls it for you automatically

### The Message Loop
```
while (GetMessage(&msg, NULL, 0, 0) > 0) {
    DispatchMessage(&msg);  // → calls WndProc
}
```
- `GetMessage()` blocks until an event arrives
- `DispatchMessage()` calls your WndProc with that event
- `WM_QUIT` breaks the loop → program exits cleanly

### The Five Required API Functions

| Function | What It Does |
|----------|-------------|
| `SetCursorPos(x, y)` | Forcefully moves the mouse to pixel (x, y) |
| `GetCursorPos(&point)` | Reads where the cursor is right now |
| `SetTimer()` | Sets a repeating timer that fires WM_TIMER |
| `GetSystemMetrics()` | Gets system info like screen dimensions |
| `RegisterHotKey()` | Registers a global keyboard shortcut |

### Message-Only Window (`HWND_MESSAGE`)
- A window that exists to receive messages but has NO visual presence
- Does not appear in taskbar, Alt+Tab, or anywhere on screen
- Perfect for background daemon-style programs

### `HINSTANCE` and Handles
- In Win32, almost everything is a "handle" (a reference/ID to a resource)
- `HWND` = handle to a window
- `HINSTANCE` = handle to the application instance (the running program)
- Handles are opaque — you don't see the raw memory, Windows manages it

---

## ✅ Quick Viva Q&A

**Q: Why does this program use `WinMain` instead of `main`?**
> `WinMain` is the entry point for Windows GUI applications. Using `main` would give you a console window, breaking the invisible background requirement. The `-mwindows` compiler flag also suppresses the console.

**Q: How does the timer work?**
> `SetTimer(hwnd, 1, 400, NULL)` asks Windows to send a `WM_TIMER` message to our window every 400ms. We handle it in `WndProc` — this is where the cursor jitter code lives.

**Q: What happens if you don't clamp to screen boundaries?**
> `SetCursorPos` would try to move the cursor to negative coordinates or coordinates beyond the screen edge. Windows would clip it to the screen anyway, but our boundary check makes the behavior explicit and correct.

**Q: Why can't you just run an infinite while loop instead of the message pump?**
> A busy loop would peg the CPU at 100% usage for the entire time the program runs. The message pump (`GetMessage`) puts the thread to sleep between events — the OS only wakes it up when a timer fires or a hotkey is pressed. This is much more efficient.

**Q: How do you install the Windows SDK?**
> Install Visual Studio (Community edition is free) and check the "Desktop development with C++" workload, which includes the Windows SDK. Alternatively, install MinGW-w64 which bundles the necessary Windows headers and libraries for GCC on Windows.
