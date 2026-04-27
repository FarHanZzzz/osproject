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
// ============================================================

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
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    // --- WM_TIMER fires every 400ms (set up by SetTimer below) ---
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
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    // Seed rand() with current time so jumps are different each run
    srand((unsigned int)time(NULL));

    // === STEP 1: Get screen dimensions with GetSystemMetrics() ===
    // SM_CXSCREEN = screen width in pixels  (works on any monitor resolution)
    // SM_CYSCREEN = screen height in pixels
    screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // === STEP 2: Register a Window Class (blueprint for our window) ===
    WNDCLASSA wc = {0};               // Zero-initialize all fields
    wc.lpfnWndProc   = WndProc;       // Our event handler function
    wc.hInstance     = hInst;         // Associate with this application
    wc.lpszClassName = "CursorClass"; // Unique name for this class
    RegisterClassA(&wc);              // Register blueprint with Windows

    // === STEP 3: Create a message-only HIDDEN window ===
    // HWND_MESSAGE = special parent that makes the window invisible
    // (no taskbar entry, no visible UI, just receives messages)
    HWND hwnd = CreateWindowExA(
        0, "CursorClass", "Hidden", 0, // No style, class name, title, no style flags
        0, 0, 0, 0,                    // x, y, width, height all 0 (no visible area)
        HWND_MESSAGE, NULL, hInst, NULL // Message-only parent, no menu
    );

    // === STEP 4: Register Ctrl+Shift+Q as a global hotkey ===
    // This works even when our app is in the background.
    // MOD_CONTROL | MOD_SHIFT = both Ctrl and Shift must be held
    // 'Q' = the Q key. ID=1 is our arbitrary label for this hotkey.
    RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 'Q');

    // === STEP 5: Start the jitter timer (fires every 400 ms) ===
    // Every 400ms Windows will send WM_TIMER to our window.
    // ID=1 is our arbitrary label for this timer.
    SetTimer(hwnd, 1, 400, NULL);

    // === STEP 6: The Message Loop — keeps the program alive ===
    // GetMessage blocks until an event arrives, fills the MSG struct.
    // Returns > 0 for normal messages; returns 0 when WM_QUIT arrives.
    // DispatchMessage routes each message to WndProc().
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        DispatchMessage(&msg); // Send event to WndProc() for handling
    }
    // Ctrl+Shift+Q caused WM_QUIT → GetMessage returned 0 → loop exits

    return 0;
}
