/*
 * Task B — The Elusive Cursor (Win32)
 * CSC413/CSE315 — OS Programming Assignment, Spring 2026
 *
 * Description:
 *   A stealthy background program that makes the mouse cursor jump
 *   randomly by a few pixels every few hundred milliseconds.
 *   Demonstrates Win32 API message loop, timers, hotkeys, and
 *   invisible message-only windows.
 *
 * Build (MinGW):
 *   gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows
 *
 * Build (MSVC):
 *   cl /W4 elusive_cursor.c user32.lib gdi32.lib /Fe:elusive_cursor.exe
 *
 * Kill:
 *   Press Ctrl+Shift+Q (the registered global hotkey)
 *
 * OS Concepts Demonstrated:
 *   - Win32 message loop (event-driven programming model)
 *   - WinMain() vs main() entry point differences
 *   - SetTimer() for non-blocking recurring events
 *   - RegisterHotKey() for global keyboard interception
 *   - Message-only windows (HWND_MESSAGE)
 *   - GetSystemMetrics() for querying OS display info
 *   - SetCursorPos() / GetCursorPos() for mouse I/O
 */

#include <windows.h>
#include <stdlib.h>
#include <time.h>

/* ---------- Configuration Constants ---------- */

#define TIMER_ID        1       /* ID for our recurring jitter timer          */
#define JITTER_INTERVAL 400     /* Timer fires every 400 ms                  */
#define JITTER_MAX      25      /* Max pixels to move in any direction        */
#define HOTKEY_ID       42      /* Arbitrary ID for the Ctrl+Shift+Q hotkey   */

/* ---------- Forward Declarations ---------- */

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/* ---------- Entry Point ---------- */

/*
 * WinMain — the Win32 GUI entry point.
 *
 * Unlike console's main(argc, argv), Windows GUI apps receive:
 *   hInst  — handle to this process's module (used for resource loading)
 *   hPrev  — always NULL on modern Windows (legacy from Win16)
 *   lpCmd  — the command-line string (not tokenised like argv)
 *   nShow  — how the window should be shown (SW_SHOW, SW_HIDE, etc.)
 *
 * We don't use hPrev, lpCmd, or nShow because our window is invisible.
 */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    /* Suppress unused parameter warnings */
    (void)hPrev;
    (void)lpCmd;
    (void)nShow;

    /* Seed the random number generator for cursor jitter */
    srand((unsigned int)time(NULL));

    /*
     * Step 1: Register a minimal window class.
     *
     * Even though our window will be invisible (message-only), Windows
     * still requires a registered class before CreateWindowEx can be called.
     * The key field is lpfnWndProc — it tells Windows which function
     * should handle messages for windows of this class.
     */
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = hInst;
    wc.lpszClassName  = "ElusiveCursorClass";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Failed to register window class!", "Error", MB_ICONERROR);
        return 1;
    }

    /*
     * Step 2: Create a message-only window.
     *
     * HWND_MESSAGE as the parent means this window:
     *   - Is never visible (no taskbar icon, no window frame)
     *   - Can still receive messages (WM_TIMER, WM_HOTKEY, etc.)
     *   - Cannot be found by EnumWindows or FindWindow
     *
     * This is what makes the prank "stealthy" — the user sees no window,
     * no icon, nothing. Only Task Manager would reveal the process.
     */
    HWND hwnd = CreateWindowExA(
        0,                      /* No extended window styles              */
        "ElusiveCursorClass",   /* Window class name (registered above)   */
        "Elusive Cursor",       /* Window title (invisible anyway)        */
        0,                      /* No window styles (invisible)           */
        0, 0, 0, 0,             /* Position and size (irrelevant)         */
        HWND_MESSAGE,           /* Parent = HWND_MESSAGE → invisible      */
        NULL,                   /* No menu                                */
        hInst,                  /* Application instance                   */
        NULL                    /* No creation parameters                 */
    );

    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create message window!", "Error", MB_ICONERROR);
        return 1;
    }

    /*
     * Step 3: Register a global hotkey (Ctrl + Shift + Q).
     *
     * RegisterHotKey() intercepts the key combo at the OS level,
     * even when our window doesn't have focus. This is essential
     * because our window is invisible — the user can't click on
     * it to give it focus.
     *
     * When the user presses Ctrl+Shift+Q, Windows sends WM_HOTKEY
     * to our message loop, which triggers a clean shutdown.
     *
     * MOD_CONTROL | MOD_SHIFT = requires both Ctrl and Shift held
     * 'Q' = the Q key (virtual key code)
     */
    if (!RegisterHotKey(hwnd, HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'Q')) {
        MessageBoxA(NULL,
            "Failed to register hotkey Ctrl+Shift+Q!\n"
            "Another program may be using this combo.",
            "Warning", MB_ICONWARNING);
        /* Non-fatal — continue anyway, user can kill via Task Manager */
    }

    /*
     * Step 4: Start a recurring timer.
     *
     * SetTimer() does NOT block the thread. It tells Windows to
     * inject a WM_TIMER message into our message queue every
     * JITTER_INTERVAL milliseconds. The actual jitter logic runs
     * in WndProc when WM_TIMER is received.
     *
     * This is fundamentally different from sleep()/usleep():
     *   - sleep() blocks everything (no messages processed)
     *   - SetTimer() is non-blocking (message loop remains responsive)
     *
     * If we used sleep(), the WM_HOTKEY for Ctrl+Shift+Q would
     * never be processed while sleeping → no kill switch.
     */
    SetTimer(hwnd, TIMER_ID, JITTER_INTERVAL, NULL);

    /*
     * Step 5: Run the Win32 message loop.
     *
     * GetMessage() blocks until a message is available, then:
     *   1. TranslateMessage() — converts key-down/up to WM_CHAR
     *   2. DispatchMessage()  — sends to WndProc for processing
     *
     * GetMessage() returns 0 when WM_QUIT is posted (by PostQuitMessage).
     * This cleanly terminates the loop when the hotkey is pressed.
     */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

/* ---------- Window Procedure (Message Handler) ---------- */

/*
 * WndProc — called by DispatchMessage() for every message.
 *
 * We handle three messages:
 *   WM_TIMER  — apply random jitter to the cursor position
 *   WM_HOTKEY — Ctrl+Shift+Q pressed → clean shutdown
 *   WM_DESTROY — window being destroyed → post quit message
 *
 * All other messages are forwarded to DefWindowProcA (default handling).
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

        case WM_TIMER: {
            /* Only respond to our specific timer ID */
            if (wParam != TIMER_ID)
                break;

            /*
             * Query the screen dimensions so we can clamp the cursor.
             * SM_CXSCREEN = screen width in pixels
             * SM_CYSCREEN = screen height in pixels
             *
             * GetSystemMetrics() queries the OS for hardware/display info.
             * We call it every tick (not cached) to handle multi-monitor
             * resolutions changing at runtime.
             */
            int screenW = GetSystemMetrics(SM_CXSCREEN);
            int screenH = GetSystemMetrics(SM_CYSCREEN);

            /* Get the current cursor position (absolute screen coordinates) */
            POINT pt;
            GetCursorPos(&pt);

            /*
             * Generate random jitter in range [-JITTER_MAX, +JITTER_MAX].
             *
             * (rand() % (2*JITTER_MAX + 1)) gives [0, 50]
             * Subtracting JITTER_MAX shifts to [-25, +25]
             *
             * This creates a uniform random displacement in both axes.
             */
            int dx = (rand() % (JITTER_MAX * 2 + 1)) - JITTER_MAX;
            int dy = (rand() % (JITTER_MAX * 2 + 1)) - JITTER_MAX;

            /* Calculate new position */
            int newX = pt.x + dx;
            int newY = pt.y + dy;

            /*
             * Clamp to screen bounds so the cursor doesn't vanish
             * off the edge of the display.
             */
            if (newX < 0)        newX = 0;
            if (newY < 0)        newY = 0;
            if (newX >= screenW) newX = screenW - 1;
            if (newY >= screenH) newY = screenH - 1;

            /*
             * Move the cursor to the new jittered position.
             * SetCursorPos() takes absolute screen coordinates.
             */
            SetCursorPos(newX, newY);
            return 0;
        }

        case WM_HOTKEY: {
            /*
             * Global hotkey triggered — check it's our Ctrl+Shift+Q.
             * wParam contains the hotkey ID we registered earlier.
             */
            if (wParam == HOTKEY_ID) {
                /* Stop the jitter timer */
                KillTimer(hwnd, TIMER_ID);

                /* Unregister the hotkey so the OS releases the combo */
                UnregisterHotKey(hwnd, HOTKEY_ID);

                /*
                 * Post WM_QUIT to the message queue.
                 * This causes GetMessage() to return 0, ending the loop.
                 * PostQuitMessage(0) means exit code 0 (success).
                 */
                PostQuitMessage(0);
            }
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }

    /* Default handler for all other messages */
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}
