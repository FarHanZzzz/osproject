# Context for Antigravity AI (Windows Boot Session)

Hello Antigravity! When you read this, I am currently booted into my Windows machine. 

## The Mission
I am working on my CSC413/CSE315 Operating Systems Spring 2026 Programming Assignment (refer to `OS Projects- Spring2026.pdf` in the root folder). I have already completed all the Linux-based tasks. 

**Your current objective is to help me complete Task B (The Elusive Cursor).**

## Task B Details
- **Environment:** Windows (Win32 API).
- **Goal:** Create a stealthy, background C/C++ program that makes the mouse cursor jump randomly by a few pixels every few hundred milliseconds.
- **Constraints:** Must use standard Windows headers `<windows.h>`, `<time.h>`, `<stdlib.h>`.
- **Requirements:** 
  1. Needs a `WinMain` entry point.
  2. Needs a `SetTimer` to apply jitter (e.g., using `GetCursorPos` and `SetCursorPos`).
  3. Needs an invisible window (`HWND_MESSAGE`) so the message loop can run.
  4. Must include a secret kill switch hotkey (e.g., `RegisterHotKey` for Ctrl+Shift+Q) so I can kill the prank.

## Next Steps for You (The AI)
1. Write the complete C code for this in a file named `elusive_cursor.c` in this folder.
2. Provide me with a Windows `Makefile` or the exact `gcc` command to compile it using MinGW (e.g., `gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows`).
3. Guide me through running it and testing the kill switch.
