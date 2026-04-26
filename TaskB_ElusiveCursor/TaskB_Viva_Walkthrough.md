# Task B: The Elusive Cursor — The Viva Defense Walkthrough

Imagine you're walking into the viva. The examiner asks: *"How did you approach Task B, and why did you write the code this way?"*

This document is your exact thought process, from reading the question for the very first time to writing the final line of code. Read this as a story—it will lock the concepts into your brain.

---

## Step 1: Reading the Question & Finding the Prerequisites
When I first saw the PDF for Task B, the prompt asked to:
1. Create a **background program** that makes the mouse cursor jump randomly.
2. The program should be **invisible** — no visible window, no console.
3. The user must be able to **stop it** using a keyboard shortcut (Ctrl+Shift+Q).

**My immediate thought process:**
"Okay, this isn't a normal console application. I need a program that sits in the background, messes with the mouse, and listens for a secret kill switch. This screams *Windows API*."

**Prerequisites I had to learn before coding:**
1.  **How do Windows programs work?** → They don't have `main()`. They use `WinMain()`. Everything is event-driven — the OS sends you *messages* (like a receptionist routing phone calls).
2.  **How do I make it invisible?** → Windows has something called a "message-only window" (`HWND_MESSAGE`). It's a window that exists *purely* to receive events. It never appears on screen.
3.  **How do I run code repeatedly?** → `SetTimer()`. It tells Windows to send me a `WM_TIMER` message every N milliseconds. No `while(true)` sleep loop needed.
4.  **How do I catch a keyboard shortcut system-wide?** → `RegisterHotKey()`. It lets my invisible program capture Ctrl+Shift+Q even when it's not in focus.

---

## Step 2: The Mental Blueprint (ELI5)
Before touching the keyboard, I drew this mental picture:
*   **Me (The Program):** I'm a sneaky gremlin hiding in the background.
*   **My Setup:** I build a secret hideout (invisible message-only window). I set an alarm clock that rings every 400ms (the timer). I also write down my emergency phone number (the hotkey).
*   **Every Time the Alarm Rings:** I peek at where the mouse is, roll two dice for X and Y, and shove the mouse to a new spot.
*   **When the Phone Rings (Ctrl+Shift+Q):** I self-destruct and cleanly exit.

---

## Step 3: Writing the Code (Line-by-Line Rationale)

Here is exactly why I wrote every piece of code.

### 1. The Headers
```c
#include <windows.h>  // The one-stop shop for ALL Windows API functions:
                      // SetCursorPos, GetCursorPos, RegisterHotKey, SetTimer, etc.
#include <stdlib.h>   // For rand() and srand() — generating random numbers.
#include <time.h>     // For time(NULL) — seeding the random number generator.
```
*Why no `<stdio.h>`?* Because this is a GUI program. There's no console to `printf` to. If I needed debug output, I'd use `OutputDebugString()`.

### 2. The Window Procedure (The Receptionist)
*What is this?* In Windows, every event (timer tick, hotkey press, mouse click) arrives as a "message." The OS calls *our* function and says "Hey, this happened, deal with it."

```c
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
```
*   `HWND hwnd` — which window received the message.
*   `UINT msg` — what type of event it is (`WM_TIMER`, `WM_HOTKEY`, etc.).
*   `WPARAM` / `LPARAM` — extra details (we don't need them here).

### 3. The Timer Handler (Moving the Mouse)
```c
if (msg == WM_TIMER) {
    POINT pt;
    GetCursorPos(&pt);  // Ask Windows: "Where is the mouse right now?"

    int jumpX = (rand() % 51) - 25;  // Random number from -25 to +25
    int jumpY = (rand() % 51) - 25;
```
*Wait, why `% 51 - 25`?*
*   `rand() % 51` gives `0` to `50` (that's 51 possible values).
*   Subtracting 25 shifts the range to `-25` to `+25`.
*   This gives us a small, annoying jitter in both directions.

```c
    int newX = pt.x + jumpX;
    int newY = pt.y + jumpY;

    // Clamp to screen boundaries (hardcoded 1920x1080)
    if (newX < 0)    newX = 0;
    if (newY < 0)    newY = 0;
    if (newX > 1920) newX = 1920;
    if (newY > 1080) newY = 1080;

    SetCursorPos(newX, newY);  // THE MONEY LINE — move the cursor!
}
```
*Viva Note:* `GetCursorPos` reads the current position. `SetCursorPos` overwrites it. The OS handles the rest.

### 4. The Hotkey Handler (The Kill Switch)
```c
if (msg == WM_HOTKEY) {
    PostQuitMessage(0);  // Tells our message loop to stop
}
```
*Why `PostQuitMessage` instead of just `exit(0)`?*
Because `PostQuitMessage(0)` sends a `WM_QUIT` message to our message loop. This lets the program exit *cleanly* — the `while(GetMessage(...))` loop naturally returns `0`, all resources get freed. `exit(0)` would kill everything abruptly.

### 5. WinMain — The Entry Point
```c
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    srand((unsigned int)time(NULL));  // Seed random so we get different jumps each run
```
*Why `srand(time(NULL))`?* Without seeding, `rand()` would produce the exact same sequence every time the program runs. The cursor would jump to the same places in the same order. `time(NULL)` gives a different seed every second.

### 6. Registering the Window Class & Creating the Invisible Window
```c
WNDCLASSA wc = {0};
wc.lpfnWndProc  = WndProc;        // "Use MY function to handle events"
wc.hInstance     = hInst;
wc.lpszClassName = "SimpleCursorClass";
RegisterClassA(&wc);

HWND hwnd = CreateWindowExA(0, "SimpleCursorClass", "Hidden", 0,
                            0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, NULL);
```
*Viva Key Point:* `HWND_MESSAGE` is the magic. It creates a window that:
- Does NOT appear on the taskbar.
- Does NOT show on screen.
- EXISTS purely to receive messages (timer ticks, hotkeys).

### 7. Setting Up the Timer & Hotkey
```c
RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 'Q');  // Ctrl+Shift+Q → sends WM_HOTKEY
SetTimer(hwnd, 1, 400, NULL);                            // Every 400ms → sends WM_TIMER
```

### 8. The Message Loop (The Heartbeat)
```c
MSG msg;
while (GetMessage(&msg, NULL, 0, 0) > 0) {
    DispatchMessage(&msg);  // Forward the message to WndProc
}
```
*This is the heart of EVERY Windows program.* `GetMessage()` blocks (waits) until there's an event. `DispatchMessage()` routes it to our `WndProc`. The loop runs forever until `PostQuitMessage(0)` is called, which makes `GetMessage()` return `0`.

---

## Step 4: Problems Faced & "What Ifs" (Viva Goldmine)

### 1. "Why Do You Need a Window at All?"
**The "What If":** "Can't you just use `SetTimer` without a window?"
**Your Answer:** "`SetTimer()` and `RegisterHotKey()` both need a window handle (`HWND`) to know *where* to send the messages. A message-only window is the lightest possible solution — it's invisible and uses almost no resources."

### 2. "Why 400 Milliseconds?"
**Your Answer:** "It's a balance. Too fast (like 50ms) makes the cursor unusable — you literally can't click anything. Too slow (like 2000ms) makes the effect barely noticeable. 400ms is annoying enough to be obvious but still lets you *kind of* use the computer."

### 3. "What If Multiple Monitors?"
**The Problem:** The hardcoded 1920x1080 boundary check only works for one screen.
**The Smarter Fix:** Use `GetSystemMetrics(SM_CXVIRTUALSCREEN)` and `GetSystemMetrics(SM_CYVIRTUALSCREEN)` to get the actual total screen area dynamically. But for simplicity, the assignment only expected single-monitor support.

### 4. "Why Not Use Threads?"
**The "What If":** "Why not just spawn a thread with `CreateThread` and do `while(true) { move mouse; Sleep(400); }`?"
**Your Answer:** "That would work for moving the mouse, but the hotkey listener still needs a message loop. Windows global hotkeys are message-based. Without a window and message loop, `RegisterHotKey` has nowhere to send the `WM_HOTKEY` message. The timer-based approach elegantly combines both needs into one event loop."

### 5. "How Is This Different from Task A?"
**Your Answer:** "Task A is Linux-based IPC using `fork()` and `pipe()` — completely different OS. Task B is Windows API, event-driven programming. Task A creates separate *processes*. Task B runs as a single process listening for OS events (timer + hotkey). They demonstrate two fundamentally different OS paradigms: Unix process model vs. Windows message-driven model."
