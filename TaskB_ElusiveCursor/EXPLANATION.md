# 🖱️ Task B — Elusive Cursor (ELI5 Edition)

> **Quick Overview:** This is a Windows-only prank program. It runs invisibly in the background and nudges the mouse cursor by a random amount every 400 milliseconds. The user presses `Ctrl+Shift+Q` to stop it. **Key OS concept: Event-Driven Programming using the Windows Message Loop.**

---

Imagine you're a **mischievous ghost** haunting someone's computer. Every few moments, you grab their mouse and nudge it a tiny bit in a random direction. The person is confused because they can't see you — you're invisible! The only way they can stop you is by saying the **magic words** (pressing Ctrl+Shift+Q).

That's exactly what this program does:
- **You (the ghost)** = the invisible background program
- **Grabbing the mouse** = calling `SetCursorPos()` every 400ms
- **Being invisible** = creating a hidden, message-only window
- **Magic words** = a global hotkey registered with the OS

---

## 🎯 The Big Picture

```
Program starts
  │
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

The program just sits there, invisible, endlessly waiting for events. Every 400ms, the timer event arrives and the mouse gets nudged.

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

---

## 📖 Code Walkthrough — The Fun Version

### Part 1: The Event-Driven Model (Message Loop)

```c
MSG msg;
while (GetMessage(&msg, NULL, 0, 0) > 0) {
    DispatchMessage(&msg);
}
```

📞 **Think of this as a call center receptionist.** The receptionist sits at their desk doing absolutely nothing until the phone rings. When it rings:
1. `GetMessage()` picks up the phone (waits for an event)
2. `DispatchMessage()` transfers the call to the right department (`WndProc`)
3. The receptionist sits back down and waits for the next call

The loop runs FOREVER until someone tells it to stop (via `PostQuitMessage()`), which makes `GetMessage()` return 0, ending the `while` loop.

**Why is this better than polling?** Imagine the receptionist calling every department every second asking "got any work?" That wastes CPU. Instead, the receptionist sleeps until an event actually happens. This is called **event-driven programming** and it uses almost zero CPU.

---

### Part 2: The Invisible Window

```c
HWND hwnd = CreateWindowExA(
    0, "SimpleCursorClass", "Hidden", 0,
    0, 0, 0, 0,
    HWND_MESSAGE,    // ← THIS is the magic!
    NULL, hInst, NULL
);
```

👻 **Why do we need a window at all?** Windows only sends messages (timer events, hotkey events) to **windows**. No window = no messages = our program is deaf. But we don't WANT a visible window. So we use `HWND_MESSAGE`, which creates a **message-only window** — it exists in memory but has no visual presence whatsoever. Not on the screen, not in the taskbar, not even in Alt+Tab!

**Why register a window class first?** Every window in Windows must have a "class" that tells the OS which function handles its messages. Think of it like: "Before you can have a dog, the breed must exist." We register `SimpleCursorClass` as a breed, then create a dog of that breed.

---

### Part 3: The Timer

```c
SetTimer(hwnd, 1, 400, NULL);
```

⏰ **This tells Windows: "Hey, send me a `WM_TIMER` message every 400 milliseconds."** It's like setting an alarm that goes off repeatedly. Every time the alarm rings, our `WndProc` function wakes up and does its thing (move the mouse).

**Why 400ms?** Fast enough to be annoying, slow enough that the user can still kinda use their computer. If it were 10ms, the mouse would be unusable. If it were 5000ms, the user might not even notice.

---

### Part 4: The Mouse Nudge

```c
if (msg == WM_TIMER) {
    POINT pt;
    GetCursorPos(&pt);           // Where is the mouse right now?

    int jumpX = (rand() % 51) - 25;   // Random number from -25 to +25
    int jumpY = (rand() % 51) - 25;

    int newX = pt.x + jumpX;
    int newY = pt.y + jumpY;

    // Don't let the mouse fly off the screen
    if (newX < 0)    newX = 0;
    if (newY < 0)    newY = 0;
    if (newX > 1920) newX = 1920;
    if (newY > 1080) newY = 1080;

    SetCursorPos(newX, newY);    // MOVE IT!
}
```

🎲 **The random math explained:**
- `rand() % 51` gives a number from 0 to 50 (51 possible values)
- Subtracting 25 shifts it to -25 to +25
- So the mouse jumps up to 25 pixels in any direction — enough to be annoying, not enough to teleport across the screen

**Why hardcode 1920x1080?** The "proper" way is to call `GetSystemMetrics(SM_CXSCREEN)` to ask Windows the actual screen size. We hardcoded it to keep the code dead simple and easy to explain. On a 1920x1080 monitor (which is the most common), this works perfectly.

---

### Part 5: The Kill Switch

```c
RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 'Q');

// Inside WndProc:
if (msg == WM_HOTKEY) {
    PostQuitMessage(0);    // Tell the message loop to stop
}
```

🔑 **`RegisterHotKey` is a system-wide shortcut.** Unlike normal keyboard shortcuts that only work when your app is focused, this one works from ANYWHERE on the desktop. Even if you're in Chrome or playing a game, pressing Ctrl+Shift+Q will send a `WM_HOTKEY` message to our invisible window.

`PostQuitMessage(0)` doesn't kill the program directly. It puts a `WM_QUIT` message into the queue, which causes `GetMessage()` to return 0 on the next loop, which gracefully ends the `while` loop.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| Message Loop | A receptionist sitting at a desk, waiting for the phone to ring. Picks up, transfers the call, waits for the next one. |
| `WndProc` | The "department" that handles the call. Checks what type of event it is and reacts. |
| `HWND_MESSAGE` | An invisible room. Messages can be delivered here, but nobody can see the room. |
| `SetTimer()` | An alarm clock that rings every N milliseconds and sends a `WM_TIMER` event. |
| `SetCursorPos()` | Teleport the mouse to exact (x, y) coordinates on screen. |
| `RegisterHotKey()` | Register a keyboard shortcut that works globally, even when the app isn't focused. |
| Event-Driven | Instead of constantly checking "anything happen?", you sleep until something DOES happen. Saves CPU. |
| `WM_TIMER` | A message that means "your timer alarm just went off!" |
| `WM_HOTKEY` | A message that means "the user pressed your registered hotkey!" |
| `PostQuitMessage()` | Doesn't kill the app instantly. It tells the message loop to end gracefully on the next cycle. |
