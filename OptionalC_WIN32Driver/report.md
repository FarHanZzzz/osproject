# Optional C — WIN32 Kernel-Mode Driver Report
## CSC413/CSE315 — OS Programming Assignment, Spring 2026

---

## 1. Overview

This report documents the implementation of a Windows kernel-mode driver (WDM) that:
1. Prints `"Hello Kernel! Farhan Sadeque"` via `DbgPrint()` when loaded
2. Prints `"Goodbye Kernel!"` via `DbgPrint()` when unloaded
3. **(Bonus)** Creates `C:\kernel_test.txt` using `ZwCreateFile()` + `ZwWriteFile()` from kernel space (ring 0)

---

## 2. Kernel Space vs User Space

### Ring Model (x86/x64)

```
Ring 0 (Kernel)     — Full hardware access, no memory protection
  ↓ System call boundary (syscall / sysenter)
Ring 3 (User)       — Restricted access, protected by MMU
```

| Aspect | User Space (Ring 3) | Kernel Space (Ring 0) |
|--------|--------------------|-----------------------|
| **Memory** | Virtual, protected per-process | Physical, shared across system |
| **Crash impact** | Segfault → only your process dies | Bug → BSOD (entire system crash) |
| **API** | Win32 API (`CreateFile`, `printf`) | NT Native API (`ZwCreateFile`, `DbgPrint`) |
| **Entry point** | `main()` / `WinMain()` | `DriverEntry()` |
| **Output** | Console / stdout | Kernel debug buffer (DebugView / WinDbg) |

### Why This Matters

Our driver's `DriverEntry()` runs in **ring 0** — it has direct access to all hardware, all memory, and can execute privileged CPU instructions. This is why:
- A null pointer dereference → BSOD (not just a crash)
- File I/O uses `Zw*` functions (NT native), not `fopen()`/`CreateFile()`
- Output uses `DbgPrint()` (kernel debug buffer), not `printf()`

---

## 3. DriverEntry — The Kernel's main()

### Comparison with Linux

| Windows WDM | Linux Kernel Module | Purpose |
|-------------|---------------------|---------|
| `DriverEntry()` | `module_init()` | Called on load |
| `DriverUnload()` | `module_exit()` | Called on unload |
| `DbgPrint()` | `printk()` | Kernel log output |
| DebugView | `dmesg` | View kernel log |
| `sc create`/`sc start` | `insmod` | Load the module |
| `sc stop`/`sc delete` | `rmmod` | Unload the module |
| `NTSTATUS` | `int` (0 = success) | Return code convention |

### DriverEntry Prototype

```c
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
```

- `DriverObject` — represents this driver in the kernel's object manager
- `RegistryPath` — the driver's configuration key in the Windows registry
- Returns `STATUS_SUCCESS` (0x00000000) to stay loaded; any error code unloads immediately

---

## 4. DbgPrint — Kernel-Mode Output

Unlike `printf()` which writes to a file descriptor (stdout → terminal), `DbgPrint()`:
- Writes to the **kernel debug message buffer**
- Is consumed by a **kernel debugger** (WinDbg) or **DebugView** (Sysinternals)
- Supports format strings similar to `printf` (`%s`, `%d`, `%wZ` for UNICODE_STRING)
- There is **no console** in kernel mode — the concept doesn't exist

### Viewing DbgPrint Output

1. Download **DebugView** from: https://learn.microsoft.com/en-us/sysinternals/downloads/debugview
2. Run as **Administrator**
3. Enable: **Capture → Capture Kernel** + **Enable Verbose Kernel Output**
4. Load the driver → messages appear in the DebugView window

---

## 5. NTSTATUS Return Codes

Windows kernel functions return `NTSTATUS` — a 32-bit structured error code:

| NTSTATUS | Meaning |
|----------|---------|
| `0x00000000` | `STATUS_SUCCESS` — operation completed |
| `0xC0000022` | `STATUS_ACCESS_DENIED` — insufficient privileges |
| `0xC0000034` | `STATUS_OBJECT_NAME_NOT_FOUND` — file/path not found |
| `0xC000003A` | `STATUS_OBJECT_PATH_NOT_FOUND` — directory doesn't exist |

The `NT_SUCCESS(status)` macro checks if the high bit is 0 (success/info) vs 1 (warning/error).

---

## 6. ZwCreateFile — Kernel-Mode File I/O (Bonus)

### Why Zw* Instead of CreateFile?

`CreateFile()` is a **Win32 API** — it exists in `kernel32.dll` and runs in user space. Kernel-mode drivers cannot call Win32 functions. Instead, they use the **NT Native API**:

- `ZwCreateFile()` → creates or opens a file
- `ZwWriteFile()` → writes data to a file
- `ZwClose()` → closes a handle

### NT Path Format

The kernel doesn't understand drive letters (`C:\`). It uses object manager paths:

```
User space:      C:\kernel_test.txt
Kernel space:    \DosDevices\C:\kernel_test.txt
                 ↓
                 \Device\HarddiskVolume2\kernel_test.txt   (actual device path)
```

`\DosDevices\C:` is a **symbolic link** maintained by the object manager that maps the user-friendly drive letter to the actual NT device path.

### Key Kernel Structures

```c
UNICODE_STRING fileName;        // NT kernel uses UTF-16, not char*
OBJECT_ATTRIBUTES objAttr;      // Describes the target object (file)
IO_STATUS_BLOCK ioStatusBlock;  // Receives I/O completion status
```

### What We Expect

If successful:
```
[KernelDriver] ZwCreateFile NTSTATUS = 0x00000000
[KernelDriver] ZwWriteFile NTSTATUS = 0x00000000
[KernelDriver] Bytes written: 133
[KernelDriver] File handle closed.
```

And a file `C:\kernel_test.txt` will appear with the message:
```
Hello from kernel mode! This file was created by a WDM driver running in ring 0.
If you can read this, the ZwCreateFile + ZwWriteFile calls succeeded!
```

---

## 7. Driver Signature Enforcement

Windows requires all kernel drivers to be digitally signed with a WHQL (Microsoft) certificate. This prevents unsigned rootkits from loading.

### How to Disable (for Testing Only)

```
Shift + Click "Restart" → Troubleshoot → Advanced Options →
Startup Settings → Restart → Press F7 "Disable driver signature enforcement"
```

> ⚠️ This is **temporary** — the next normal reboot re-enables signing.

### Why This Security Measure Exists

Unsigned kernel code has **full system access**:
- Read/write any memory (including other processes)
- Intercept system calls (rootkit behavior)
- Access hardware directly (disk, network, keyboard)
- Disable security software

Driver signing ensures that only code verified by Microsoft can run in ring 0.

---

## 8. Step-by-Step Testing Procedure

### Prerequisites
- Windows 10/11 (64-bit)
- Visual Studio 2022 with WDK installed
- Sysinternals DebugView

### Build Steps
1. Open Visual Studio → New Project → search "WDM" → **Empty WDM Driver**
2. Add `driver.c` to the project
3. Build → produces `driver.sys` in the output directory

### Load and Test (Admin Command Prompt)

```cmd
:: Step 1 — Register the driver
sc create myDriver binPath= "C:\full\path\to\driver.sys" type= kernel

:: Step 2 — Open DebugView as Admin, enable Capture Kernel

:: Step 3 — Load the driver
sc start myDriver
:: DebugView should show: "Hello Kernel! Farhan Sadeque"
:: Check for C:\kernel_test.txt (if ZwCreateFile succeeded)

:: Step 4 — Unload the driver
sc stop myDriver
:: DebugView should show: "Goodbye Kernel!"

:: Step 5 — Remove the driver registration
sc delete myDriver
```

### Expected DebugView Output

```
[KernelDriver] ==================================================
[KernelDriver] Hello Kernel! Farhan Sadeque
[KernelDriver] DriverEntry called successfully.
[KernelDriver] RegistryPath: <registry path>
[KernelDriver] ==================================================
[KernelDriver] Attempting ZwCreateFile (kernel-mode file I/O)...
[KernelDriver] ZwCreateFile NTSTATUS = 0x00000000
[KernelDriver] ZwWriteFile NTSTATUS = 0x00000000
[KernelDriver] Bytes written: 133
[KernelDriver] File handle closed.
---
[on sc stop]
[KernelDriver] ==================================================
[KernelDriver] Goodbye Kernel!
[KernelDriver] Driver has been unloaded successfully.
[KernelDriver] ==================================================
```

---

## 9. Challenges Encountered

1. **WDK Installation**: The Windows Driver Kit must be installed separately from Visual Studio, and its version must match the installed Windows SDK version exactly.

2. **Driver Signing**: Modern Windows (64-bit) enforces kernel-mode code signing. Testing requires booting with signing disabled every session.

3. **No Console in Kernel**: There is no `printf()` or stdout in kernel mode. All output goes through `DbgPrint()` to the debug buffer, which must be captured by DebugView or a kernel debugger.

4. **NT Path Format**: Kernel code cannot use `"C:\file.txt"` — it must use `"\\DosDevices\\C:\\file.txt"`. This is a common source of `STATUS_OBJECT_NAME_NOT_FOUND` errors.

5. **BSOD Risk**: Any bug in `DriverEntry` (null pointer, stack overflow, calling a function at wrong IRQL) causes an immediate Blue Screen of Death. Testing in a VM with snapshots is essential.

6. **IRQL Requirements**: `ZwCreateFile` can only be called at `PASSIVE_LEVEL` (the lowest IRQL). Calling it at a higher IRQL (e.g., from an ISR) causes a system crash.

---

## 10. Comparison: Linux Kernel Module vs Windows Kernel Driver

| Feature | Linux | Windows |
|---------|-------|---------|
| Source header | `<linux/init.h>`, `<linux/module.h>` | `<ntddk.h>` |
| Entry function | `module_init(fn)` | `DriverEntry()` |
| Exit function | `module_exit(fn)` | `DriverUnload()` (via DriverObject) |
| Output function | `printk(KERN_INFO ...)` | `DbgPrint(...)` |
| View output | `dmesg` | DebugView (Sysinternals) |
| Load command | `sudo insmod module.ko` | `sc create + sc start` |
| Unload command | `sudo rmmod module` | `sc stop + sc delete` |
| Build system | Kernel Makefile (`make -C /lib/modules/...`) | Visual Studio + WDK |
| Signing | Not required (warn only) | **Required** (must disable to test) |
| Crash impact | Kernel panic (system hangs) | BSOD (blue screen) |
| File format | `.ko` (ELF relocatable) | `.sys` (PE executable) |
