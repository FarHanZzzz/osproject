// Task B — The Elusive Cursor (Win32)
// A stealthy background program that makes the mouse cursor jump randomly every few hundred ms.
// Press Ctrl+Shift+Q to stop the program.
//
// Compile (MinGW):  gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows
// Compile (MSVC):   cl elusive_cursor.c user32.lib gdi32.lib /Fe:elusive_cursor.exe

#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define TIMER_ID        1      // ID for our timer
#define JITTER_INTERVAL 400    // Move cursor every 400 ms
#define JITTER_MAX      25     // Max pixels to move in any direction
#define HOTKEY_ID       42     // ID for the Ctrl+Shift+Q kill hotkey

// This function handles all Windows messages
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

        case WM_TIMER: {
            if (wParam != TIMER_ID) break;

            // Get screen size so cursor doesn't go off-screen
            int screenW = GetSystemMetrics(SM_CXSCREEN);
            int screenH = GetSystemMetrics(SM_CYSCREEN);

            // Get current cursor position
            POINT pt;
            GetCursorPos(&pt);

            // Generate random offset between -25 and +25
            int dx = (rand() % (JITTER_MAX * 2 + 1)) - JITTER_MAX;
            int dy = (rand() % (JITTER_MAX * 2 + 1)) - JITTER_MAX;

            // Calculate new position
            int newX = pt.x + dx;
            int newY = pt.y + dy;

            // Keep cursor inside screen bounds
            if (newX < 0)        newX = 0;
            if (newY < 0)        newY = 0;
            if (newX >= screenW) newX = screenW - 1;
            if (newY >= screenH) newY = screenH - 1;

            // Move the cursor!
            SetCursorPos(newX, newY);
            return 0;
        }

        case WM_HOTKEY: {
            // Ctrl+Shift+Q was pressed — shut down cleanly
            if (wParam == HOTKEY_ID) {
                KillTimer(hwnd, TIMER_ID);
                UnregisterHotKey(hwnd, HOTKEY_ID);
                PostQuitMessage(0);
            }
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

// WinMain = entry point for Windows GUI apps (instead of main)
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    (void)hPrev; (void)lpCmd; (void)nShow;  // unused

    srand((unsigned int)time(NULL));  // seed random number generator

    // Step 1: Register a window class (required by Windows)
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = hInst;
    wc.lpszClassName  = "ElusiveCursorClass";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Failed to register window class!", "Error", MB_ICONERROR);
        return 1;
    }

    // Step 2: Create an invisible window (HWND_MESSAGE = no visible window)
    HWND hwnd = CreateWindowExA(
        0, "ElusiveCursorClass", "Elusive Cursor", 0,
        0, 0, 0, 0,
        HWND_MESSAGE,  // This makes the window invisible
        NULL, hInst, NULL
    );

    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create window!", "Error", MB_ICONERROR);
        return 1;
    }

    // Step 3: Register Ctrl+Shift+Q as a global kill hotkey
    RegisterHotKey(hwnd, HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'Q');

    // Step 4: Start the jitter timer (fires every 400ms)
    SetTimer(hwnd, TIMER_ID, JITTER_INTERVAL, NULL);

    // Step 5: Run the message loop (keeps the program alive)
    // GetMessage blocks until a message arrives, returns 0 on WM_QUIT
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
