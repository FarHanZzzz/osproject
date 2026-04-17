# Context for Antigravity AI (Windows Boot Session)

Hello Antigravity! When you read this, I am currently booted into my Windows machine. 

## The Mission
I am working on my CSC413/CSE315 Operating Systems Spring 2026 Programming Assignment (refer to `OS Projects- Spring2026.pdf` in the root folder). I have already completed the mandatory section. 

**Your current objective is to help me complete Optional Section C (WIN32 Kernel-Mode Driver).**

## Optional C Details
- **Environment:** Windows 10/11, Visual Studio 2022, Windows Driver Kit (WDK).
- **Goal:** Write a Windows kernel-mode driver (`.sys`) that prints messages to the kernel debug log on load and unload, and (optionally) creates a file directly from kernel space.
- **Requirements:**
  1. Print `"Hello Kernel! <my name>"` via `DbgPrint()` in `DriverEntry`.
  2. Print `"Goodbye Kernel!"` via `DbgPrint()` in `DriverUnload`.
  3. (Extremely Difficult Bonus): Call `ZwCreateFile()` and `ZwWriteFile()` to write a file from kernel mode.

## Next Steps for You (The AI)
1. Ask me to confirm that I have Visual Studio and WDK installed.
2. Write the complete C code for `driver.c` including `<ntddk.h>`, `DriverEntry`, and `DriverUnload`.
3. Provide me with extremely clear, step-by-step instructions on:
   - How to create a new WDM Driver project in Visual Studio.
   - How to disable Driver Signature Enforcement in Windows boot settings (CRITICAL step).
   - How to install and run Sysinternals `DebugView` as Administrator.
   - How to use the command line (`sc create`, `sc start`, `sc stop`) to load and unload the driver.
4. Warn me to use a Virtual Machine if possible, as a kernel panic (BSOD) is likely if we attempt the `ZwCreateFile` implementation.
