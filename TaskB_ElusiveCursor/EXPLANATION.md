# 🖱️ Task B — The Elusive Cursor (ELI5 Edition)

## The Story

Imagine you're a **ghost** haunting someone's computer. You're invisible — no window, no icon, nothing. But every half second, you **nudge their mouse cursor** a few pixels in a random direction. The person thinks their mouse is broken. 😈

To stop the ghost, there's a secret password: **Ctrl+Shift+Q**.

That's this program. It's a stealthy Windows prank.

---

## 🎯 The Big Picture

```
Step 1: Create an INVISIBLE window (ghost form)
Step 2: Set an alarm that rings every 400ms
Step 3: Every time the alarm rings → nudge the cursor randomly
Step 4: If someone types Ctrl+Shift+Q → ghost disappears
```

---

## 🔧 How to Run It (Windows Only)

```cmd
gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows
elusive_cursor.exe
:: Press Ctrl+Shift+Q to stop
```

---

## 📖 Code Walkthrough — The Fun Version

### Part 1: Ghost Entry Point (WinMain)

```c
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
```

🚪 **Normal programs use `main()`. Windows GUI programs use `WinMain()`.** Think of it like two different front doors — one for console programs, one for window programs. Windows sends you some info when your program starts:
- `hInst` = your program's ID badge
- The rest? We don't care about them (unused).

---

### Part 2: Becoming Invisible

```c
// Step 1: Register a "window class" (required by Windows, like filling out a form)
WNDCLASSA wc;
wc.lpfnWndProc  = WndProc;           // "When something happens, call THIS function"
wc.lpszClassName = "ElusiveCursorClass";
RegisterClassA(&wc);

// Step 2: Create an INVISIBLE window
HWND hwnd = CreateWindowExA(
    ...,
    HWND_MESSAGE,    // 👻 THIS is the magic word — makes the window invisible
    ...
);
```

👻 **What is `HWND_MESSAGE`?** It creates a "message-only" window. This window:
- Has no visual form (completely invisible)
- Doesn't show up in the taskbar
- Can't be found by normal window searches
- BUT can still receive messages (timer ticks, hotkey presses)

It's like being a ghost — you can hear everything, but nobody can see you. Only Task Manager reveals the process.

---

### Part 3: The Kill Switch

```c
RegisterHotKey(hwnd, HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'Q');
```

⌨️ **Why do we need a hotkey?** Since the window is invisible, the user can't click X to close it. `RegisterHotKey()` tells Windows: *"Whenever ANYONE presses Ctrl+Shift+Q, send me a message."* This works globally — even when other apps have focus. It's our secret kill switch.

---

### Part 4: Setting the Alarm

```c
SetTimer(hwnd, TIMER_ID, 400, NULL);    // Ring every 400ms
```

⏰ **`SetTimer` vs `sleep()` — why does it matter?**

Think of it this way:
- **`sleep(400)`** = You fall asleep for 400ms. Someone knocking on your door? You don't hear them. Ctrl+Shift+Q? Ignored. You're ASLEEP.
- **`SetTimer(400)`** = You set an alarm for 400ms and go about your day. When the alarm rings, you do the cursor jitter. Meanwhile, you can still hear the doorbell (hotkey). You're AWAKE.

If we used `sleep()`, the kill switch would never work because the program would be frozen!

---

### Part 5: The Message Loop (Ghost's Daily Life)

```c
MSG msg;
while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

📬 **This is like checking your mailbox in a loop:**
1. `GetMessage()` = "Any mail?" (blocks until something arrives)
2. `TranslateMessage()` = "Let me read it"
3. `DispatchMessage()` = "OK, let me act on it" (calls WndProc)

The loop runs forever until `GetMessage()` receives `WM_QUIT` (which happens when Ctrl+Shift+Q is pressed) and returns 0.

**This is how ALL Windows apps work** — they're just mailbox-checking loops.

---

### Part 6: Handling Messages (WndProc)

```c
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TIMER:   // ⏰ Alarm rang!
        case WM_HOTKEY:  // ⌨️ Kill switch pressed!
        case WM_DESTROY: // 💀 Window is being destroyed
    }
}
```

**What is a callback?** YOU write `WndProc`, but WINDOWS calls it. It's like leaving your phone number at a restaurant — you don't call them, they call you when your table is ready.

---

### Part 7: The Cursor Jitter (the prank itself!)

```c
case WM_TIMER: {
    // Where's the cursor right now?
    POINT pt;
    GetCursorPos(&pt);

    // Random nudge: -25 to +25 pixels in each direction
    int dx = (rand() % 51) - 25;
    int dy = (rand() % 51) - 25;

    // Keep it on screen (so cursor doesn't vanish off the edge)
    int newX = clamp(pt.x + dx, 0, screenWidth - 1);
    int newY = clamp(pt.y + dy, 0, screenHeight - 1);

    // MOVE IT! 🖱️💨
    SetCursorPos(newX, newY);
}
```

🎲 **The math:** `rand() % 51` gives 0 to 50. Subtract 25 → gives -25 to +25. So the cursor jumps randomly in any direction, up to 25 pixels. Small enough to be annoying, big enough to be noticeable.

---

### Part 8: Clean Shutdown

```c
case WM_HOTKEY: {
    KillTimer(hwnd, TIMER_ID);         // Stop the alarm
    UnregisterHotKey(hwnd, HOTKEY_ID); // Release the key combo
    PostQuitMessage(0);                 // "I'm done!" → WM_QUIT → loop ends
}
```

🛑 `PostQuitMessage(0)` puts a WM_QUIT message in the mailbox. When `GetMessage()` sees WM_QUIT, it returns 0, and the while loop exits cleanly.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| Message Loop | Checking your mailbox forever until you get a "quit" letter |
| WndProc | A callback — you write the function, Windows calls it |
| `SetTimer` | An alarm clock that doesn't freeze you while waiting |
| `RegisterHotKey` | A secret global keyboard shortcut |
| `HWND_MESSAGE` | Invisible ghost window — can hear but can't be seen |
| Event-Driven | "When X happens, do Y" instead of "do step 1, 2, 3..." |
