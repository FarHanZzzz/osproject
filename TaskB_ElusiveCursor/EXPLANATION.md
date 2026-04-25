# Task B — The Elusive Cursor
## How the Code Works — Complete Explanation

> **Source:** `elusive_cursor.c` | **Language:** C | **OS Concepts:** Win32 Message Loop, Timers, Global Hotkeys, Message-Only Windows

---

## What This Program Does

This program is a stealthy "prank" application that makes your mouse cursor randomly jump by a few pixels every 400 milliseconds. It runs silently in the background — no window, no taskbar icon, no console — making it nearly invisible to the user.

The only way to stop it (by design) is to press the secret hotkey combination **Ctrl+Shift+Q**, which cleanly shuts down the program.

```
[Program starts]
  → Cursor starts jittering ±25 pixels in both X and Y directions
  → No visible window, no taskbar icon
  → Every 400ms, cursor jumps to a random nearby position

[User presses Ctrl+Shift+Q]
  → Timer stops, hotkey unregistered, program exits cleanly
```

---

## Why This Is an OS Assignment (Not Just a Prank)

This program demonstrates several core Windows operating system concepts:

1. **Event-Driven Programming** — The Win32 message loop is the foundation of all Windows GUI applications.
2. **Non-Blocking Timers** — `SetTimer()` is fundamentally different from `sleep()` and demonstrates how the OS manages recurring events.
3. **Global System Hooks** — `RegisterHotKey()` shows how the OS can intercept keyboard input system-wide, across all applications.
4. **Invisible Background Processes** — The message-only window (`HWND_MESSAGE`) demonstrates how processes can run without any visible UI.

---

## How It Works — Step by Step

### 1. The Entry Point: `WinMain` (Line 59)

```c
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
```

**Why `WinMain` instead of `main`?**

In Linux/POSIX, every program starts at `main(int argc, char *argv[])`. But Windows GUI applications use `WinMain` as their entry point. The OS passes four parameters:

| Parameter | What It Is |
|-----------|-----------|
| `hInst` | Handle to the current application instance (used for loading resources) |
| `hPrev` | Always `NULL` on modern Windows (legacy from 16-bit Windows 3.1) |
| `lpCmd` | The raw command-line string (not tokenized like `argv`) |
| `nShow` | How the window should be displayed (`SW_SHOW`, `SW_HIDE`, etc.) |

We don't use `hPrev`, `lpCmd`, or `nShow` because our window is invisible — but the OS still provides them.

### 2. Window Class Registration (Lines 73–82)

```c
WNDCLASSA wc;
ZeroMemory(&wc, sizeof(wc));
wc.lpfnWndProc   = WndProc;
wc.hInstance      = hInst;
wc.lpszClassName  = "ElusiveCursorClass";
RegisterClassA(&wc);
```

**Why do we need to register a "window class" if our window is invisible?**

In Win32, every window — even an invisible one — must belong to a registered class. The class tells Windows:
- Which function handles messages (`lpfnWndProc = WndProc`)
- Which application instance owns the class (`hInstance = hInst`)
- What the class is called (`lpszClassName`)

Think of it like a blueprint: the class is the design, the window is the built house.

### 3. The Invisible Window (Lines 91–103)

```c
HWND hwnd = CreateWindowExA(
    0,
    "ElusiveCursorClass",
    "Elusive Cursor",
    0,
    0, 0, 0, 0,
    HWND_MESSAGE,      // ← THIS is the key
    NULL, hInst, NULL
);
```

**The secret: `HWND_MESSAGE`**

Normally, `CreateWindowExA` creates a visible window on the desktop. But by passing `HWND_MESSAGE` as the parent, we create a **message-only window**. This type of window:

- ✅ Can receive and process Windows messages (WM_TIMER, WM_HOTKEY)
- ❌ Cannot be seen on screen
- ❌ Has no taskbar entry
- ❌ Cannot be found by `FindWindow()` or `EnumWindows()`

This is what makes the program "stealthy" — it exists only to receive messages from the OS.

### 4. Global Hotkey Registration (Lines 113–119)

```c
RegisterHotKey(hwnd, HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'Q');
```

**How does this work across ALL applications?**

`RegisterHotKey()` tells the Windows input system: "When the user presses Ctrl+Shift+Q, no matter what application is in the foreground, send `WM_HOTKEY` to my window."

This is an **OS-level keyboard hook**. The key combination is intercepted by the keyboard driver before any application sees it. This is essential because our window is invisible — it can never have keyboard focus, so normal key events would never reach it.

| Parameter | Value | Meaning |
|-----------|-------|---------|
| `hwnd` | Our window | Where to send the `WM_HOTKEY` message |
| `HOTKEY_ID` | `42` | An arbitrary ID to identify our hotkey |
| `MOD_CONTROL \| MOD_SHIFT` | Modifiers | Both Ctrl and Shift must be held |
| `'Q'` | Virtual key code | The Q key |

### 5. The Non-Blocking Timer (Line 132)

```c
SetTimer(hwnd, TIMER_ID, JITTER_INTERVAL, NULL);
```

**Why `SetTimer()` instead of `sleep()`?**

This is a critical OS concept. Let's compare:

| `sleep(400)` | `SetTimer(hwnd, ..., 400, ...)` |
|--------------|--------------------------------------|
| Blocks the entire thread | Returns immediately |
| No messages processed while sleeping | Message loop continues running |
| Hotkey would be ignored during sleep | Hotkey works at all times |
| Sequential: sleep → act → sleep | Event-driven: timer fires → OS sends message |

`SetTimer()` tells the OS: "Every 400 milliseconds, inject a `WM_TIMER` message into my message queue." The timer runs asynchronously — the message loop keeps processing other messages (like `WM_HOTKEY`) between timer fires.

If we used `sleep()`, pressing Ctrl+Shift+Q during a sleep period would do nothing — the program would be blocked and unable to process the hotkey event.

### 6. The Message Loop (Lines 142–146)

```c
MSG msg;
while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

**This is the heart of every Win32 application.**

The message loop is how Windows communicates with applications:

1. **`GetMessage()`** — Blocks until a message is available in the queue. Returns `0` when `WM_QUIT` is posted.
2. **`TranslateMessage()`** — Converts hardware key events (key-down/up) into character messages (`WM_CHAR`).
3. **`DispatchMessage()`** — Sends the message to the appropriate window procedure (`WndProc`).

This loop is fundamentally an **event-driven architecture**. The program doesn't decide when things happen — the OS does. The OS pushes events (timer fired, hotkey pressed) into the queue, and the loop processes them one by one.

### 7. The Window Procedure: `WndProc` (Lines 157–207)

This function handles three messages:

#### WM_TIMER — Cursor Jitter (Lines 160–195)

```c
case WM_TIMER: {
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    POINT pt;
    GetCursorPos(&pt);

    int dx = (rand() % (JITTER_MAX * 2 + 1)) - JITTER_MAX;
    int dy = (rand() % (JITTER_MAX * 2 + 1)) - JITTER_MAX;

    int newX = pt.x + dx;
    int newY = pt.y + dy;
    // ... clamp to screen bounds ...

    SetCursorPos(newX, newY);
}
```

Each time the timer fires:
1. Query screen dimensions via `GetSystemMetrics()` — an OS API that returns hardware/display info.
2. Read current cursor position via `GetCursorPos()` — returns absolute screen coordinates.
3. Generate random offset in range `[-25, +25]` for both X and Y.
4. Clamp to screen bounds so the cursor doesn't disappear off-screen.
5. Move the cursor via `SetCursorPos()`.

**Why clamp to screen bounds?**
Without clamping, the cursor could move beyond `(0,0)` to `(screenW-1, screenH-1)`, potentially going "off screen" where the user cannot see or access it. `GetSystemMetrics(SM_CXSCREEN)` queries the OS for the primary monitor width.

#### WM_HOTKEY — Kill Switch (Lines 197–207)

```c
case WM_HOTKEY: {
    if (wParam == HOTKEY_ID) {
        KillTimer(hwnd, TIMER_ID);
        UnregisterHotKey(hwnd, HOTKEY_ID);
        PostQuitMessage(0);
    }
}
```

When Ctrl+Shift+Q is pressed:
1. **`KillTimer()`** — Stop the jitter timer. No more `WM_TIMER` messages.
2. **`UnregisterHotKey()`** — Release the Ctrl+Shift+Q combo back to the system.
3. **`PostQuitMessage(0)`** — Posts `WM_QUIT` to the message queue. `GetMessage()` returns `0`, breaking the message loop and ending the program.

This is a **clean shutdown** — all OS resources (timer, hotkey registration) are released before exit.

---

## Key OS Concepts

| Concept | Where in Code | What It Does |
|---------|---------------|--------------|
| `WinMain()` | Line 59 | Win32 GUI entry point (replaces `main()`) |
| `RegisterClassA()` | Line 80 | Registers a window blueprint with the OS |
| `CreateWindowExA()` | Line 91 | Creates a window — `HWND_MESSAGE` makes it invisible |
| `RegisterHotKey()` | Line 113 | OS-level global keyboard interception |
| `SetTimer()` | Line 132 | Non-blocking recurring timer (vs `sleep()`) |
| `GetMessage()` / `DispatchMessage()` | Lines 142–146 | Win32 event-driven message loop |
| `GetCursorPos()` / `SetCursorPos()` | Lines 178, 194 | Read/write the system mouse cursor position |
| `GetSystemMetrics()` | Lines 170–171 | Query OS for display hardware information |
| `PostQuitMessage()` | Line 205 | Inject WM_QUIT to terminate the message loop |

---

## Compilation & Testing

### Build Command (MSVC)
```cmd
cl /W4 elusive_cursor.c user32.lib gdi32.lib /Fe:elusive_cursor.exe
```

### Build Command (MinGW, if available)
```cmd
gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows
```

### Test Results

| Test | Action | Expected | Actual | Status |
|------|--------|----------|--------|--------|
| Startup | Run `elusive_cursor.exe` | No visible window, cursor starts jittering | ✓ | ✅ PASS |
| Jitter | Observe cursor movement | Cursor jumps ±25px every 400ms | ✓ | ✅ PASS |
| Stealth | Check taskbar and Alt+Tab | No entry visible | ✓ | ✅ PASS |
| Kill switch | Press Ctrl+Shift+Q | Program exits cleanly, cursor stops moving | ✓ | ✅ PASS |
| Process visible | Open Task Manager | `elusive_cursor.exe` visible in process list | ✓ | ✅ PASS |

---

## Viva Questions & Answers

**Q: Why didn't you use `sleep()` or `usleep()` instead of `SetTimer()`?**
A: `sleep()` blocks the entire thread. While sleeping, the program cannot process any messages — including the kill switch hotkey (Ctrl+Shift+Q). `SetTimer()` is non-blocking: it tells the OS to inject timer events into the message queue, allowing the message loop to continue processing other events (like the hotkey) between timer fires.

**Q: What is a "message-only window" and why does this program need one?**
A: A message-only window (created with `HWND_MESSAGE` as parent) is invisible and has no taskbar presence, but can still receive OS messages like `WM_TIMER` and `WM_HOTKEY`. We need it because the Win32 timer and hotkey systems deliver their events as window messages — without a window, there's nowhere for the OS to send them.

**Q: What happens if the user presses Ctrl+Shift+Q while another application is in the foreground?**
A: It still works. `RegisterHotKey()` is a **global** registration — the OS intercepts the key combination at the input driver level, before any application sees it. The `WM_HOTKEY` message is delivered to our invisible window regardless of which application has focus.

**Q: Why do you call `GetSystemMetrics()` on every timer tick instead of caching the screen size?**
A: In a multi-monitor setup, the user could disconnect/reconnect a monitor or change their display resolution at any time. By querying every tick, we always have the current screen dimensions. The performance cost is negligible (it's a simple kernel query).

**Q: How is this different from a virus or malware?**
A: The key difference is the **kill switch**. Actual malware would not provide a clean shutdown mechanism. This program also doesn't persist across reboots (no registry entry, no startup folder), doesn't replicate itself, doesn't exfiltrate data, and is visible in Task Manager. It's a demonstration of Win32 API capabilities, not a security threat.
