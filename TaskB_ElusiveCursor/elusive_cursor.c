// ============================================================
// Task B — The Elusive Cursor (Win32 API)
// ============================================================
// PROBLEM (from PDF): Create a background program that makes
// the mouse cursor jump randomly every few hundred milliseconds.
// The program should be invisible (no visible window) and can
// be stopped using a keyboard shortcut (Ctrl+Shift+Q).
//
// HOW WE SOLVE IT:
//   1. Create an invisible window (required by Windows to receive events).
//   2. Start a repeating timer that fires every 400ms.
//   3. Every time the timer fires, get the mouse position,
//      add a random offset, and move the mouse.
//   4. Register Ctrl+Shift+Q as a global hotkey to quit.
//   5. Run an infinite "Message Loop" to keep the program alive.
//
// Compile (MinGW):  gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows
// Compile (MSVC):   cl elusive_cursor.c user32.lib gdi32.lib /Fe:elusive_cursor.exe
// ============================================================

#include <windows.h>  // The main Windows API header (gives us SetCursorPos, etc.)
#include <stdlib.h>   // For rand() and srand()
#include <time.h>     // For time() — used to seed the random number generator

// ----------------------------------------------------------
// THE "RECEPTIONIST" FUNCTION (Window Procedure)
// ----------------------------------------------------------
// Windows sends messages (events) to this function.
// We check what type of event it is and react accordingly.
// Think of it as: "If the phone rings, what do we do?"
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    // --------------------------------------------------
    // EVENT: Timer fired (happens every 400 milliseconds)
    // --------------------------------------------------
    if (msg == WM_TIMER) {
        // Get the current mouse position on screen
        POINT pt;
        GetCursorPos(&pt);  // pt.x and pt.y now hold the current coordinates

        // Generate a random jump between -25 and +25 pixels
        // rand() % 51 gives a number from 0 to 50
        // Subtracting 25 shifts it to the range -25 to +25
        int jumpX = (rand() % 51) - 25;
        int jumpY = (rand() % 51) - 25;

        // Calculate the new cursor position
        int newX = pt.x + jumpX;
        int newY = pt.y + jumpY;

        // Hardcoded screen boundary check (assuming 1920x1080 resolution)
        // This prevents the cursor from flying off the edge of the screen
        if (newX < 0)    newX = 0;      // Don't go past the left edge
        if (newY < 0)    newY = 0;      // Don't go past the top edge
        if (newX > 1920) newX = 1920;   // Don't go past the right edge
        if (newY > 1080) newY = 1080;   // Don't go past the bottom edge

        // MOVE THE MOUSE to the new position!
        // This is the core of the "elusive cursor" effect.
        SetCursorPos(newX, newY);
        return 0;
    }

    // --------------------------------------------------
    // EVENT: Hotkey pressed (Ctrl+Shift+Q)
    // --------------------------------------------------
    if (msg == WM_HOTKEY) {
        // The user pressed Ctrl+Shift+Q — time to shut down
        // PostQuitMessage(0) sends a WM_QUIT to our message loop,
        // which causes GetMessage() to return 0, ending the while loop.
        PostQuitMessage(0);
        return 0;
    }

    // For any other message we don't care about, let Windows handle it
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------
// MAIN FUNCTION (WinMain for Windows GUI programs)
// ----------------------------------------------------------
// Windows GUI programs use WinMain instead of main().
// hInst = handle to this program instance
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    // Seed the random number generator with the current time
    // so we get different jumps every time we run the program
    srand((unsigned int)time(NULL));

    // ======================================================
    // STEP 1: Register a "Window Class" (required by Windows)
    // ======================================================
    // Even though our window will be invisible, Windows requires
    // us to register a class before creating any window.
    WNDCLASSA wc = {0};                 // Initialize all fields to zero
    wc.lpfnWndProc  = WndProc;         // Tell Windows which function handles messages
    wc.hInstance     = hInst;           // Tell Windows which program this belongs to
    wc.lpszClassName = "SimpleCursorClass";  // A name for our window class
    RegisterClassA(&wc);               // Register it with the OS

    // ======================================================
    // STEP 2: Create an INVISIBLE window
    // ======================================================
    // HWND_MESSAGE makes this a "message-only" window.
    // It doesn't appear on screen, in the taskbar, or anywhere visible.
    // We need it only so Windows can send timer/hotkey events to us.
    HWND hwnd = CreateWindowExA(
        0,                      // No extra styles
        "SimpleCursorClass",    // Use the class we just registered
        "Hidden",               // Window title (doesn't matter, it's invisible)
        0,                      // No visible styles
        0, 0, 0, 0,            // Position and size (all zero — invisible)
        HWND_MESSAGE,           // THIS makes the window invisible
        NULL, hInst, NULL
    );

    // ======================================================
    // STEP 3: Register Ctrl+Shift+Q as a global hotkey
    // ======================================================
    // This lets us press Ctrl+Shift+Q from ANYWHERE on the desktop
    // to send a WM_HOTKEY message to our window, triggering a clean exit.
    RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 'Q');

    // ======================================================
    // STEP 4: Start the jitter timer
    // ======================================================
    // SetTimer tells Windows to send a WM_TIMER message to our window
    // every 400 milliseconds. Each time WndProc receives WM_TIMER,
    // it moves the cursor randomly.
    SetTimer(hwnd, 1, 400, NULL);

    // ======================================================
    // STEP 5: The Infinite Message Loop
    // ======================================================
    // This is the heart of every Windows program.
    // GetMessage() blocks (waits) until Windows sends us an event.
    // DispatchMessage() forwards the event to our WndProc function.
    // The loop runs forever until PostQuitMessage() is called,
    // which makes GetMessage() return 0, ending the loop.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        DispatchMessage(&msg);  // Send the message to WndProc
    }

    return 0;  // Program exits cleanly
}
