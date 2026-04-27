// ============================================================
// Task B — The Elusive Cursor (Win32 API)
// ============================================================
//
// THE PROBLEM:
//   Write a background Windows program that makes the mouse
//   cursor jump/jitter randomly every few hundred milliseconds.
//   The program must be invisible (no visible window), use all
//   5 required Win32 API functions, and have a secret kill hotkey.
//
// THE SOLUTION — Windows Event-Driven Model:
//   Windows programs are EVENT-DRIVEN, meaning we don't write a
//   while(true) loop that runs code constantly. Instead:
//     1. We create a HIDDEN window (just to receive events).
//     2. We set a TIMER — every 400ms, Windows sends WM_TIMER.
//     3. We register a HOTKEY — Ctrl+Shift+Q sends WM_HOTKEY.
//     4. Our WndProc callback function responds to these events.
//
// THE 5 REQUIRED WIN32 API FUNCTIONS (from the PDF):
//   1. SetCursorPos(x, y)    — forcefully moves the cursor
//   2. GetCursorPos(&point)  — reads the cursor's current position
//   3. SetTimer()            — triggers our jitter every 400ms
//   4. GetSystemMetrics()    — gets the screen width and height
//   5. RegisterHotKey()      — registers the Ctrl+Shift+Q kill switch
//
// Compile (MinGW on Windows):
//   gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows
//
// cd /home/farhan-sadeque/Downloads/Osproject/TaskB_ElusiveCursor
// gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows
// ./elusive_cursor.exe
// ============================================================

// [OS CONCEPT: System APIs / Win32 API] 
// The OS provides functions (system calls/APIs) to interact with the system.
#include <windows.h>  // Master Win32 API header: HWND, POINT, MSG, WM_TIMER etc.
#include <stdlib.h>   // For rand() — generates random integers
#include <time.h>     // For time() — seeds the random number generator

// Globals: stored here so WndProc can use them without calling
// GetSystemMetrics() on every single timer tick (400x per second).
int screenWidth;   // Monitor width  in pixels (e.g., 1920)
int screenHeight;  // Monitor height in pixels (e.g., 1080)

// ----------------------------------------------------------
// WndProc — The Window Procedure (Windows calls this for us)
// ----------------------------------------------------------
// This is a CALLBACK function — we never call it directly.
// Windows calls it automatically whenever an event occurs.
// msg tells us WHAT happened; wParam/lParam carry extra details.
// ----------------------------------------------------------
// [OS CONCEPT: Callbacks & Event Handlers]
// This is a CALLBACK function — we never call it directly. The Operating System 
// calls it for us when hardware (timer/keyboard) triggers an event.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    // [OS CONCEPT: Asynchronous Events & Interrupts]
    // --- WM_TIMER fires every 400ms (set up by SetTimer below) ---
    // Similar to a hardware interrupt, the OS signals our process periodically.
    if (msg == WM_TIMER) {
        POINT pt;                   // Win32 struct: holds {pt.x, pt.y} pixel coords
        GetCursorPos(&pt);          // Fills pt with the cursor's current screen position

        // rand() % 51 gives 0..50; subtracting 25 gives -25..+25 pixel jump
        int jumpX = (rand() % 51) - 25;  // Random horizontal jitter
        int jumpY = (rand() % 51) - 25;  // Random vertical jitter

        int newX = pt.x + jumpX;   // New X position after jump
        int newY = pt.y + jumpY;   // New Y position after jump

        // Clamp to screen boundaries so the cursor cannot fall off the edge
        if (newX < 0)            newX = 0;            // Left edge
        if (newY < 0)            newY = 0;             // Top edge
        if (newX > screenWidth)  newX = screenWidth;  // Right edge
        if (newY > screenHeight) newY = screenHeight; // Bottom edge

        SetCursorPos(newX, newY);  // Forcefully teleport the cursor to new position
        return 0;                  // We handled this message
    }

    // --- WM_HOTKEY fires when Ctrl+Shift+Q is pressed anywhere ---
    if (msg == WM_HOTKEY) {
        // PostQuitMessage puts WM_QUIT into the queue.
        // GetMessage() in WinMain will receive it and exit the loop.
        PostQuitMessage(0);
        return 0; // We handled this message
    }

    // Pass any other messages to Windows' default handler
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------
// WinMain — Entry point for Windows GUI programs (replaces main)
// ----------------------------------------------------------
// [OS CONCEPT: Process Entry Point]
// Every process managed by the OS needs an entry point. For Windows GUI apps, it's WinMain.
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    // 1. Prepare for randomness
    // We use the current time as a seed so the cursor jumps randomly every time we run the app.
    srand((unsigned int)time(NULL));

    // 2. Find out how big the screen is
    // We need the width and height so we know the boundaries where the cursor can jump.
    screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 3. Create a blueprint for our invisible window
    // Every Windows program needs a "Window", even if we don't show it on the screen.
    WNDCLASSA wc = {0};               
    wc.lpfnWndProc   = WndProc;       // Tell Windows: "Send all events to the WndProc function"
    wc.hInstance     = hInst;         // Tie this blueprint to our application
    wc.lpszClassName = "CursorClass"; // Give our blueprint a name
    RegisterClassA(&wc);              // Save the blueprint

    // 4. Build the actual invisible window
    // [OS CONCEPT: OS Resources & Handles (HWND)]
    // We ask the OS to allocate a resource (a Window) and it gives us a Handle (HWND) to reference it.
    // HWND_MESSAGE tells Windows to make this window completely hidden (no taskbar icon, no visual).
    HWND hwnd = CreateWindowExA(0, "CursorClass", "Hidden", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, NULL);

    // 5. Create a "Kill Switch"
    // Register "Ctrl + Shift + Q" as a global shortcut. No matter what app is open, 
    // pressing this will send a signal to our program to stop.
    RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 'Q');

    // 6. Start the "Jitter" Timer
    // Tell Windows: "Send an alarm to my window every 400 milliseconds."
    SetTimer(hwnd, 1, 400, NULL);

    // 7. Keep the program alive (The Message Loop)
    // Programs normally close instantly. This loop forces the program to stay open and listen for:
    // - The 400ms timer alarm (to jump the cursor)
    // - The Ctrl+Shift+Q shortcut (to close the program)
    // [OS CONCEPT: The Message Queue & Inter-Process Communication (IPC)]
    // The OS collects hardware events (mouse, keyboard, timers) and puts them in a queue for our process.
    // GetMessage pulls them from the queue, blocking if empty (which saves CPU cycles).
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        DispatchMessage(&msg); // Forward the event to our WndProc function to actually handle it
    }

    return 0;
}
