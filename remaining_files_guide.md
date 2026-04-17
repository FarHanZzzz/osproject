# How to Complete the Remaining Files
## CSC413/CSE315 — OS Programming Assignment, Spring 2026

> This guide tells you exactly what still needs to be done, in which order,
> and what to write in each remaining file.

---

## Current Status

| File | Status | What's left |
|------|--------|-------------|
| `TaskA_WordCounter/word_counter.c` | ✅ Complete & tested | Nothing — ready to submit |
| `TaskA_WordCounter/Makefile` | ✅ Complete | Nothing |
| `TaskB_ElusiveCursor/elusive_cursor.c` | ❌ Not written | Needs Windows to compile & test |
| `TaskC_MatrixAdder/matrix_adder.c` | ✅ Complete & tested | Nothing |
| `TaskC_MatrixAdder/Makefile` | ✅ Complete | Nothing |
| `TaskD_MLQScheduler/mlq_scheduler.cpp` | ✅ Complete & tested | Nothing |
| `TaskD_MLQScheduler/Makefile` | ✅ Complete | Nothing |
| `TaskE_RealTimeScheduler/rt_scheduler.cpp` | ✅ Complete & tested | Nothing |
| `TaskE_RealTimeScheduler/Makefile` | ✅ Complete | Nothing |
| `OptionalA_KernelModule/simple.c` | ❌ Not written | Write it (Linux — do this next!) |
| `OptionalA_KernelModule/Makefile` | ❌ Not written | Write it |
| `OptionalB_JavaRMI/` | ❌ Empty | Needs your Java OOP project |
| `OptionalC_WIN32Driver/driver.c` | ❌ Not written | Needs Windows + WDK |

---

## 1. Task B — Elusive Cursor (Win32)

### What you need
- A Windows PC (or VM with Windows 10/11)
- MinGW-w64 installed (provides `gcc` for Windows with Win32 headers)
  - Download: https://www.mingw-w64.org/downloads/ → choose `MSYS2` installer

### Step-by-step instructions

#### Step 1: Create the source file
On Windows, create `TaskB_ElusiveCursor/elusive_cursor.c` with this content:

```c
/*
 * Task B — The Elusive Cursor (Win32)
 * Build:  gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32
 * Kill:   Press Ctrl+Shift+Q
 */
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define TIMER_ID    1
#define JITTER_MAX  25        // pixels to move in any direction
#define HOTKEY_ID   42        // arbitrary ID for our kill hotkey

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    srand((unsigned)time(NULL));

    // Register a minimal window class (we need a message loop)
    WNDCLASSA wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = "ElusiveCursorClass";
    RegisterClassA(&wc);

    // Create an invisible window (just to receive timer messages)
    HWND hwnd = CreateWindowExA(0, "ElusiveCursorClass", "Elusive Cursor",
        0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, NULL);

    // Secret kill switch: Ctrl + Shift + Q
    RegisterHotKey(hwnd, HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'Q');

    // Fire a timer every 400ms to jitter the cursor
    SetTimer(hwnd, TIMER_ID, 400, NULL);

    // Standard Win32 message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TIMER: {
            if (wParam != TIMER_ID) break;

            // Get screen dimensions (so cursor stays on screen)
            int screenW = GetSystemMetrics(SM_CXSCREEN);
            int screenH = GetSystemMetrics(SM_CYSCREEN);

            // Get current cursor position
            POINT pt;
            GetCursorPos(&pt);

            // Apply random jitter (positive or negative)
            int dx = (rand() % (JITTER_MAX * 2 + 1)) - JITTER_MAX;
            int dy = (rand() % (JITTER_MAX * 2 + 1)) - JITTER_MAX;

            // Clamp to screen bounds so cursor doesn't disappear
            int newX = pt.x + dx;
            int newY = pt.y + dy;
            if (newX < 0) newX = 0;
            if (newY < 0) newY = 0;
            if (newX >= screenW) newX = screenW - 1;
            if (newY >= screenH) newY = screenH - 1;

            SetCursorPos(newX, newY);
            break;
        }
        case WM_HOTKEY:
            if (wParam == HOTKEY_ID) {
                KillTimer(hwnd, TIMER_ID);
                UnregisterHotKey(hwnd, HOTKEY_ID);
                PostQuitMessage(0);  // Exit the message loop cleanly
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}
```

#### Step 2: Create the Makefile (Windows)
```makefile
CC      = gcc
CFLAGS  = -Wall -mwindows
TARGET  = elusive_cursor.exe
SRC     = elusive_cursor.c
LIBS    = -lgdi32 -luser32

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	del /f $(TARGET)

.PHONY: all clean
```

> **Note on `-mwindows`:** This flag tells GCC to build a Windows GUI application
> (no console window). Without it, a black console flashes when the exe runs.

#### Step 3: Compile and run (in MSYS2 terminal on Windows)
```bash
cd /path/to/TaskB_ElusiveCursor
make
./elusive_cursor.exe
# Cursor starts jumping every 400ms
# Press Ctrl+Shift+Q to stop it cleanly
```

#### What to explain in viva
1. **`SetTimer()`** creates a recurring timer event inside the Win32 message loop —
   it's not like `sleep()` which blocks everything. The message loop keeps running.
2. **`WinMain` instead of `main`** — Win32 GUI apps use `WinMain` as the entry point.
   The OS passes an instance handle, command line, and show-window flag.
3. **`RegisterHotKey()`** — tells the OS to intercept `Ctrl+Shift+Q` globally,
   even when our window doesn't have focus. Essential for a prank app with an escape hatch.
4. **`GetSystemMetrics(SM_CXSCREEN)`** — queries the OS for screen width/height.
   Without clamping, the cursor could move off-screen and "disappear."
5. **`HWND_MESSAGE`** — creates an invisible message-only window. It can receive messages
   (like WM_TIMER) without ever being visible. This keeps the prank stealthy.

---

## 2. Optional A — Linux Kernel Module

### What you need
- Your existing Linux system
- Kernel headers: `sudo apt install linux-headers-$(uname -r)`

### Step 1: Create `OptionalA_KernelModule/simple.c`

```c
/*
 * Optional A — Linux Kernel Module
 * CSC413/CSE315 — OS Programming Assignment, Spring 2026
 *
 * Demonstrates:
 *   - module_init() / module_exit() hooks
 *   - printk() to kernel log (read via dmesg)
 *   - GOLDEN_RATIO_PRIME from <linux/hash.h>
 *   - gcd() from <linux/gcd.h>
 *
 * Build:   make
 * Load:    sudo insmod simple.ko
 * Unload:  sudo rmmod simple
 * Output:  sudo dmesg | tail -10
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hash.h>   // GOLDEN_RATIO_PRIME
#include <linux/gcd.h>    // gcd(a, b)

/* Called when: sudo insmod simple.ko */
static int __init simple_init(void)
{
    printk(KERN_INFO "====================================\n");
    printk(KERN_INFO "CSC413 Kernel Module: Loading...\n");
    printk(KERN_INFO "GOLDEN_RATIO_PRIME = %lu\n", GOLDEN_RATIO_PRIME);
    printk(KERN_INFO "====================================\n");
    return 0;   // 0 = success; non-zero = module load fails
}

/* Called when: sudo rmmod simple */
static void __exit simple_exit(void)
{
    printk(KERN_INFO "====================================\n");
    printk(KERN_INFO "CSC413 Kernel Module: Removing...\n");
    printk(KERN_INFO "GCD(3700, 24) = %lu\n", gcd(3700, 24));
    printk(KERN_INFO "====================================\n");
}

/* Register the entry and exit points with the kernel */
module_init(simple_init);
module_exit(simple_exit);

/* Required metadata (build fails without MODULE_LICENSE) */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CSC413 Optional A - Kernel Module Demo");
MODULE_AUTHOR("Your Name Here");
```

### Step 2: Create `OptionalA_KernelModule/Makefile`

```makefile
# Kernel module Makefile
# MUST use TAB characters for indentation — spaces will break it

obj-m += simple.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

> ⚠️ **CRITICAL:** The lines under `all:` and `clean:` MUST start with a TAB character,
> not spaces. If you use spaces, you'll get: `Makefile:5: *** missing separator. Stop.`

### Step 3: Install kernel headers
```bash
sudo apt update
sudo apt install linux-headers-$(uname -r)
# Verify
ls /lib/modules/$(uname -r)/build
```

### Step 4: Build
```bash
cd ~/Downloads/Osproject/OptionalA_KernelModule
make
# Should produce simple.ko among other files
ls *.ko
```

### Step 5: Load, verify, unload
```bash
# Load the module
sudo insmod simple.ko

# Verify it's loaded
lsmod | grep simple

# Read the kernel log — should show GOLDEN_RATIO_PRIME
sudo dmesg | tail -8

# Unload the module
sudo rmmod simple

# Read the kernel log — should now show GCD(3700, 24) = 4
sudo dmesg | tail -8

# Clear the buffer (optional)
sudo dmesg -c
```

### Expected `dmesg` output
```
[after insmod]
[ 1234.567] ====================================
[ 1234.568] CSC413 Kernel Module: Loading...
[ 1234.569] GOLDEN_RATIO_PRIME = 11400714819323198485
[ 1234.570] ====================================

[after rmmod]
[ 1289.101] ====================================
[ 1289.102] CSC413 Kernel Module: Removing...
[ 1289.103] GCD(3700, 24) = 4
[ 1289.104] ====================================
```

### What to write in `report.md`
Create `OptionalA_KernelModule/report.md` with these sections:
1. **Kernel Space vs User Space** — ring 0 vs ring 3, why bugs crash the system
2. **Dynamic Kernel Extensions** — `insmod`/`rmmod` without rebooting
3. **`printk` vs `printf`** — kernel ring buffer, KERN_INFO log level, `dmesg`
4. **`GOLDEN_RATIO_PRIME`** — what it is (hash constant from φ), value on your machine
5. **`gcd(3700, 24)`** — show the calculation: 3700 = 4×925, 24 = 4×6, GCD = 4
6. **Challenges** — getting the Makefile tab right, installing kernel headers
7. **Testing procedure** — the exact `insmod`/`dmesg`/`rmmod` sequence

---

## 3. Optional B — Java RMI

### What you need
- Your existing Java GUI project from CSE213/CSC305
- JDK 8+ installed (`java -version`)

### Steps

1. **Identify 2-3 methods** in your existing project that are:
   - Security-sensitive (login, password check, data encryption), OR
   - Compute-intensive (heavy calculations, report generation, image processing)

2. **Create `OptionalB_JavaRMI/IMyService.java`** — the remote interface
   (see `OPTIONAL_SECTION_PLAN.md` for the full template)

3. **Create `OptionalB_JavaRMI/MyServiceImpl.java`** — server-side implementation

4. **Create `OptionalB_JavaRMI/Server.java`** — starts the RMI registry on port 1099

5. **Modify your existing GUI** to call the remote methods via stub instead of locally

6. **Test by running two terminals:**
   - Terminal 1: `java Server`
   - Terminal 2: `java YourGUIApp`

> See `OPTIONAL_SECTION_PLAN.md` → Optional B section for full code templates.

---

## 4. Optional C — WIN32 Driver

### What you need
- Windows PC with:
  - Visual Studio 2022 (Community) — free
  - Windows Driver Kit (WDK) — matches your Windows SDK version

### Steps

1. Install VS2022 + WDK from: https://learn.microsoft.com/windows-hardware/drivers/download-the-wdk
2. Open VS → New Project → search "WDM" → Empty WDM Driver
3. Add `driver.c` (see `OPTIONAL_SECTION_PLAN.md` for the full source)
4. Build → you get `driver.sys`
5. Reboot with driver signing disabled (Shift+Restart → F7)
6. Open DebugView as Administrator, enable Capture Kernel
7. Run the `sc create / sc start / sc stop / sc delete` commands
8. Observe output in DebugView

> See `OPTIONAL_SECTION_PLAN.md` → Optional C section for full `driver.c` source and step-by-step commands.

---

## Submission Checklist Before Zipping

```bash
# Run this to do a final check that everything compiles
cd ~/Downloads/Osproject
echo "=== Task A ===" && cd TaskA_WordCounter && make clean && make && cd ..
echo "=== Task C ===" && cd TaskC_MatrixAdder && make clean && make && cd ..
echo "=== Task D ===" && cd TaskD_MLQScheduler && make clean && make && cd ..
echo "=== Task E ===" && cd TaskE_RealTimeScheduler && make clean && make && cd ..
echo "=== All built! ==="
```

Then create the ZIP:
```bash
cd ~/Downloads

# Replace with your real details
SECTION=3
ID=XXXXXXX          # your student ID
COURSE=CSC413
NAME=YourName

zip -r "${SECTION}_${ID}_${COURSE}_${NAME}.zip" Osproject/ \
    --exclude "Osproject/.git/*"

echo "Done: ${SECTION}_${ID}_${COURSE}_${NAME}.zip"
ls -lh "${SECTION}_${ID}_${COURSE}_${NAME}.zip"
```

> ⚠️ **Double-check the ZIP naming format:** `<Section>_<ID>_<CourseName>_<StudentName>.zip`
> Example: `3_2120117_CSC413_Farhan_Sadeque.zip`
> The assignment says: *"Your submission may not be graded if this prefix does not match."*
