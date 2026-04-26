# 🖱️ Task B — Elusive Cursor (ELI5 Edition)

> **Quick Overview:** This is a Windows-only prank program. It runs invisibly in the background and nudges the mouse cursor by a random amount every 400 milliseconds. The user presses `Ctrl+Shift+Q` to stop it. **Key OS concept: Event-Driven Programming using the Windows Message Loop.**

---

Imagine you're a **mischievous ghost** haunting someone's computer. Every few moments, you grab their mouse and nudge it a tiny bit in a random direction. The person is confused because they can't see you — you're invisible! The only way they can stop you is by saying the **magic words** (pressing Ctrl+Shift+Q).

---

## 🎯 The Big Picture

```
Program starts
  │
  ├── GetSystemMetrics() → Get actual screen dimensions
  ├── Create an INVISIBLE window (no taskbar, no icon, nothing)
  ├── Register Ctrl+Shift+Q as a "kill switch"
  ├── Start a timer that fires every 400 milliseconds
  │
  └── INFINITE LOOP (Message Loop):
        │
        ├── Timer fires? → Get mouse position → Add random jump → Move mouse!
        ├── Ctrl+Shift+Q pressed? → Shut down cleanly
        └── Anything else? → Ignore it, keep looping
```

---

## 🔧 How to Run It

⚠️ **This is a Windows-only program!** It uses the Win32 API (`windows.h`).

```bash
# On Windows with MinGW:
gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows

# Run it:
elusive_cursor.exe

# To stop: Press Ctrl + Shift + Q
```

### How I Installed the Windows SDK

1. **MinGW (GCC for Windows):** Downloaded from the official MinGW-w64 site and added to PATH
2. **The Win32 API headers** (`windows.h`) come bundled with MinGW — no separate SDK install needed
3. **Linking:** The `-lgdi32 -luser32` flags link the Windows graphics and user interface libraries
4. **`-mwindows` flag:** Tells the compiler this is a GUI app (no console window)

---

## 📖 Code Walkthrough

### Part 1: Dynamic Screen Boundaries (GetSystemMetrics)

```c
screenWidth  = GetSystemMetrics(SM_CXSCREEN);
screenHeight = GetSystemMetrics(SM_CYSCREEN);
```

📐 **Instead of hardcoding 1920x1080**, we ask the OS for the actual screen dimensions. `SM_CXSCREEN` returns the width, `SM_CYSCREEN` returns the height. This works on ANY monitor resolution.

```c
if (newX > screenWidth)  newX = screenWidth;
if (newY > screenHeight) newY = screenHeight;
```

This prevents the cursor from flying off the screen edge regardless of the user's resolution.

---

### Part 2: The Event-Driven Model (Message Loop)

```c
MSG msg;
while (GetMessage(&msg, NULL, 0, 0) > 0) {
    DispatchMessage(&msg);
}
```

📞 **This is the heart of every Windows program.** `GetMessage()` waits for an event, `DispatchMessage()` routes it to `WndProc`. The loop runs forever until `PostQuitMessage()` is called.

---

### Part 3: The Invisible Window

```c
HWND hwnd = CreateWindowExA(0, "SimpleCursorClass", "Hidden", 0,
    0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, NULL);
```

👻 **`HWND_MESSAGE` creates a message-only window** — invisible, not in taskbar, not in Alt+Tab. We need it only to receive timer and hotkey events.

---

### Part 4: The Timer + Mouse Nudge

```c
SetTimer(hwnd, 1, 400, NULL);  // Fire WM_TIMER every 400ms
```

Every 400ms, `WndProc` receives `WM_TIMER` and:
1. Gets current cursor position via `GetCursorPos()`
2. Adds random offset (-25 to +25 pixels)
3. Clamps to screen boundaries
4. Moves cursor via `SetCursorPos()`

---

### Part 5: The Kill Switch

```c
RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 'Q');
```

🔑 **System-wide hotkey.** Works from anywhere — even if you're in another app. Pressing Ctrl+Shift+Q sends `WM_HOTKEY`, which calls `PostQuitMessage(0)` to end the message loop gracefully.

---

## Key Win32 API Functions Used

| Function | Purpose |
|----------|---------|
| `SetCursorPos(x, y)` | Forcefully move the mouse to (x, y) |
| `GetCursorPos(&point)` | Get the current mouse position |
| `SetTimer()` | Create a repeating timer event |
| `GetSystemMetrics()` | Get screen dimensions dynamically |
| `RegisterHotKey()` | Register a global keyboard shortcut |

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| Message Loop | A receptionist waiting for the phone to ring. Picks up, transfers, waits. |
| `WndProc` | The department that handles each call (event). |
| `HWND_MESSAGE` | An invisible room. Messages arrive but nobody can see it. |
| `SetTimer()` | An alarm clock that rings every N milliseconds. |
| `GetSystemMetrics()` | Asks the OS "how big is the screen?" — works on any resolution. |
| `SetCursorPos()` | Teleport the mouse to exact coordinates. |
| `RegisterHotKey()` | A global keyboard shortcut that works even when app isn't focused. |
| Event-Driven | Sleep until something happens. Much more efficient than polling. |
