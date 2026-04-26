// ============================================================
// Task B — The Elusive Cursor (Win32 API)
// ============================================================
// PROBLEM (from PDF): Create a background program that makes
// the mouse cursor jump randomly every few hundred milliseconds.
// The program should be invisible (no visible window) and can
// be stopped using a keyboard shortcut (Ctrl+Shift+Q).
//
// KEY WIN32 API FUNCTIONS USED:
//   SetCursorPos(x, y)      — forcefully move the mouse
//   GetCursorPos(&point)    — track current mouse position
//   SetTimer()              — create the jitter interval
//   GetSystemMetrics()      — get screen boundaries dynamically
//   RegisterHotKey()        — secret kill switch (Ctrl+Shift+Q)
//
// Compile (MinGW):  gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows
// ============================================================

#include <windows.h>  // The main Windows API header
#include <stdlib.h>   // For rand() and srand()
#include <time.h>     // For time() — seed the RNG

// Global screen dimensions (fetched dynamically via GetSystemMetrics)
int screenWidth;
int screenHeight;

// ----------------------------------------------------------
// THE WINDOW PROCEDURE (Callback / Event Handler)
// ----------------------------------------------------------
// Windows sends messages (events) to this function.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    // --------------------------------------------------
    // EVENT: Timer fired (happens every 400 milliseconds)
    // --------------------------------------------------
    if (msg == WM_TIMER) {
        // Get the current mouse position on screen
        POINT pt;
        GetCursorPos(&pt);

        // Generate a random jump between -25 and +25 pixels
        int jumpX = (rand() % 51) - 25;
        int jumpY = (rand() % 51) - 25;

        // Calculate the new cursor position
        int newX = pt.x + jumpX;
        int newY = pt.y + jumpY;

        // Use GetSystemMetrics() to enforce screen boundaries
        // SM_CXSCREEN = screen width, SM_CYSCREEN = screen height
        // This ensures the cursor stays within the visible screen
        if (newX < 0)            newX = 0;
        if (newY < 0)            newY = 0;
        if (newX > screenWidth)  newX = screenWidth;
        if (newY > screenHeight) newY = screenHeight;

        // MOVE THE MOUSE to the new position!
        SetCursorPos(newX, newY);
        return 0;
    }

    // --------------------------------------------------
    // EVENT: Hotkey pressed (Ctrl+Shift+Q)
    // --------------------------------------------------
    if (msg == WM_HOTKEY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------
// MAIN FUNCTION (WinMain for Windows GUI programs)
// ----------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    srand((unsigned int)time(NULL));

    // ======================================================
    // STEP 1: Get screen dimensions using GetSystemMetrics()
    // ======================================================
    // Instead of hardcoding 1920x1080, we ask the OS for the
    // actual screen size. This works on ANY resolution!
    screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // ======================================================
    // STEP 2: Register a Window Class
    // ======================================================
    WNDCLASSA wc = {0};
    wc.lpfnWndProc  = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = "SimpleCursorClass";
    RegisterClassA(&wc);

    // ======================================================
    // STEP 3: Create an INVISIBLE message-only window
    // ======================================================
    HWND hwnd = CreateWindowExA(
        0, "SimpleCursorClass", "Hidden", 0,
        0, 0, 0, 0,
        HWND_MESSAGE,  // Makes the window invisible
        NULL, hInst, NULL
    );

    // ======================================================
    // STEP 4: Register Ctrl+Shift+Q as a global hotkey
    // ======================================================
    RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_SHIFT, 'Q');

    // ======================================================
    // STEP 5: Start the jitter timer (fires every 400ms)
    // ======================================================
    SetTimer(hwnd, 1, 400, NULL);

    // ======================================================
    // STEP 6: The Infinite Message Loop
    // ======================================================
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        DispatchMessage(&msg);
    }

    return 0;
}
