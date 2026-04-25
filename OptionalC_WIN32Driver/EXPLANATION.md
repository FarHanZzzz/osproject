# Optional C — WIN32 Kernel-Mode Driver
## How the Code Works — Complete Explanation

> **Source:** `driver.c` | **Language:** C | **OS Concepts:** Kernel Space (Ring 0), DriverEntry, DbgPrint, NTSTATUS, ZwCreateFile, Kernel-Mode File I/O

---

## What This Program Does (And Why You Can't See It)

This is not a normal program. It is a **Windows Kernel-Mode Driver** — it runs inside the operating system itself, at the lowest and most privileged level of the CPU (Ring 0).

When loaded, it:
1. Prints `"Hello Kernel! Farhan Sadeque"` to the kernel debug log
2. Creates a file `C:\kernel_test.txt` directly from kernel space using NT Native APIs
3. When unloaded, prints `"Goodbye Kernel!"` to the kernel debug log

You will **never see any output on the screen** when this driver runs. There is no console, no terminal, no `printf()` in kernel mode. The output goes to an internal debug buffer that can only be viewed using a special tool called **DebugView** (or a kernel debugger like WinDbg).

```
[User runs: sc start myDriver]
  → OS calls DriverEntry()
  → DbgPrint writes to kernel debug buffer (invisible)
  → ZwCreateFile creates C:\kernel_test.txt from Ring 0
  → DebugView shows: "Hello Kernel! Farhan Sadeque"

[User runs: sc stop myDriver]
  → OS calls DriverUnload()
  → DebugView shows: "Goodbye Kernel!"
```

---

## Why This Is the Hardest Task

This task is rated **Extreme Difficulty** for good reason:

1. **A single bug = Blue Screen of Death.** In user space, a null pointer dereference crashes your program. In kernel space, it crashes the entire operating system.

2. **No safety net.** User-space programs are sandboxed by the OS — memory protection, exception handling, virtual address spaces. Kernel drivers have none of this. They can read and write ANY memory address, access ANY hardware, and execute ANY CPU instruction.

3. **No standard tools.** You can't use `printf()`, `fopen()`, `malloc()`, or any C standard library function. The kernel has its own completely separate API.

4. **Driver signing.** Windows refuses to load unsigned kernel code (to prevent rootkits). You must disable this security feature at every boot to test.

---

## How It Works — Step by Step

### 1. The Header (Line 43)

```c
#include <ntddk.h>
```

This single include replaces ALL of the standard C headers. In kernel mode:
- There is no `<stdio.h>` — no `printf`, no `scanf`, no `fopen`
- There is no `<stdlib.h>` — no `malloc`, no `free`
- There is no `<string.h>` — no `strcpy`, no `strlen`

Instead, `<ntddk.h>` (NT Device Driver Kit) provides the kernel's own version of everything:

| Standard C | Kernel Equivalent | Header |
|-----------|-------------------|--------|
| `printf()` | `DbgPrint()` | `<ntddk.h>` |
| `fopen()` / `CreateFile()` | `ZwCreateFile()` | `<ntddk.h>` |
| `fwrite()` | `ZwWriteFile()` | `<ntddk.h>` |
| `fclose()` | `ZwClose()` | `<ntddk.h>` |
| `char *` strings | `UNICODE_STRING` (UTF-16) | `<ntddk.h>` |
| `return 0` (success) | `return STATUS_SUCCESS` | `<ntddk.h>` |

### 2. DriverEntry — The Kernel's main() (Lines 183–214)

```c
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    DriverObject->DriverUnload = DriverUnload;

    DbgPrint("[KernelDriver] Hello Kernel! Farhan Sadeque\n");

    KernelWriteTestFile();

    return STATUS_SUCCESS;
}
```

**How is this different from `main()`?**

| `main()` | `DriverEntry()` |
|----------|-----------------|
| Called by the C runtime (CRT) | Called by the Windows I/O Manager |
| Receives `argc`, `argv` (command-line args) | Receives `DRIVER_OBJECT` and `RegistryPath` |
| Runs in Ring 3 (user space) | Runs in Ring 0 (kernel space) |
| Returns `int` (0 = success) | Returns `NTSTATUS` (0x00000000 = success) |
| Linked against `msvcrt.dll` | Linked against `ntoskrnl.exe` (the kernel itself) |

**What is `DRIVER_OBJECT`?**
It's a kernel structure that represents this driver. The OS creates it for us. Our only job is to register our unload function with it:

```c
DriverObject->DriverUnload = DriverUnload;
```

This tells the kernel: "When someone runs `sc stop`, call the `DriverUnload` function." Without this line, the driver **cannot be unloaded** — you would have to reboot to remove it.

**What is `RegistryPath`?**
The Windows registry path where this driver's configuration is stored (e.g., `\Registry\Machine\System\CurrentControlSet\Services\myDriver`). We print it for diagnostic purposes but don't use it otherwise.

**Why return `STATUS_SUCCESS`?**
If `DriverEntry` returns any non-success NTSTATUS code, the I/O Manager immediately unloads the driver and reports an error to `sc start`. Only `STATUS_SUCCESS` (0x00000000) keeps the driver loaded.

### 3. DriverUnload — The Cleanup Function (Lines 155–165)

```c
void DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    DbgPrint("[KernelDriver] Goodbye Kernel!\n");
}
```

This is the kernel equivalent of `module_exit()` in Linux. Called when the user runs `sc stop myDriver`. In a real driver, this is where you would:
- Free allocated kernel memory
- Delete device objects
- Cancel pending I/O requests
- Release hardware resources

Our driver is simple, so we just print a goodbye message.

**What is `UNREFERENCED_PARAMETER()`?**
A macro that tells the compiler "I know I'm not using this parameter, don't warn me about it." MSVC's `/W4` warning level would otherwise emit a warning for unused parameters.

### 4. DbgPrint — Kernel printf (Used throughout)

```c
DbgPrint("[KernelDriver] Hello Kernel! Farhan Sadeque\n");
```

**Why can't we use `printf()` in kernel mode?**

`printf()` writes to `stdout`, which is a **file descriptor** concept that only exists in user space. In kernel mode:
- There is no terminal attached to your driver
- There is no concept of `stdout` or `stderr`
- There is no C runtime library (CRT) at all

Instead, `DbgPrint()` writes to the **kernel debug message buffer** — a ring buffer in kernel memory. To read it, you need one of:
- **DebugView** (Sysinternals) — a GUI tool that displays kernel debug messages in real-time
- **WinDbg** — Microsoft's full kernel debugger

This is exactly analogous to Linux's `printk()` → `dmesg` pipeline.

### 5. ZwCreateFile — Writing Files from Ring 0 (Lines 67–137)

This is the **extremely difficult bonus** section. Here we prove the driver truly runs in kernel space by using NT Native APIs to create a file.

#### Step 5a: Initialize the File Path (Lines 85–87)

```c
UNICODE_STRING fileName;
RtlInitUnicodeString(&fileName, L"\\DosDevices\\C:\\kernel_test.txt");
```

**Why `\\DosDevices\\C:\\` instead of just `C:\`?**

The kernel doesn't understand drive letters. Drive letters like `C:` are a **user-space abstraction** maintained by the Win32 subsystem. Inside the kernel, all objects are accessed through the **Object Manager namespace**:

```
User-space path:     C:\kernel_test.txt
                     ↓
DosDevices symlink:  \DosDevices\C:  →  \Device\HarddiskVolume2
                     ↓
Kernel-space path:   \Device\HarddiskVolume2\kernel_test.txt
```

`\DosDevices\C:` is a symbolic link that maps the user-friendly drive letter to the actual NT device path.

**Why `UNICODE_STRING` instead of `char*`?**

The Windows kernel uses UTF-16 (wide characters) for all strings, not ASCII. `UNICODE_STRING` is a kernel structure containing:
```c
typedef struct {
    USHORT Length;         // Current length in bytes
    USHORT MaximumLength;  // Buffer size in bytes
    PWSTR  Buffer;         // Pointer to wide-char string
} UNICODE_STRING;
```

`RtlInitUnicodeString()` initializes this structure from a wide string literal (the `L""` prefix).

#### Step 5b: Set Up Object Attributes (Lines 95–102)

```c
OBJECT_ATTRIBUTES objAttr;
InitializeObjectAttributes(&objAttr, &fileName,
    OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
    NULL, NULL);
```

| Flag | Meaning |
|------|---------|
| `OBJ_CASE_INSENSITIVE` | File paths are case-insensitive on NTFS |
| `OBJ_KERNEL_HANDLE` | Handle is only valid in kernel mode — prevents user-mode processes from hijacking it |

#### Step 5c: Create the File (Lines 113–123)

```c
status = ZwCreateFile(&fileHandle,
    GENERIC_WRITE,              // We want to write
    &objAttr,                   // Path and flags
    &ioStatusBlock,             // Receives I/O completion info
    NULL,                       // Default allocation size
    FILE_ATTRIBUTE_NORMAL,      // Normal file
    0,                          // Exclusive access
    FILE_OVERWRITE_IF,          // Create or overwrite
    FILE_SYNCHRONOUS_IO_NONALERT,  // Synchronous I/O
    NULL, 0);
```

`ZwCreateFile()` is the kernel equivalent of Win32's `CreateFile()`. The `Zw` prefix means it's a **Native API** — it goes directly to the kernel's I/O Manager without passing through the Win32 subsystem.

The return value is an `NTSTATUS` — a 32-bit structured error code:

| NTSTATUS | Meaning |
|----------|---------|
| `0x00000000` | `STATUS_SUCCESS` |
| `0xC0000022` | `STATUS_ACCESS_DENIED` |
| `0xC0000034` | `STATUS_OBJECT_NAME_NOT_FOUND` |

#### Step 5d: Write to the File (Lines 133–142)

```c
CHAR data[] = "Hello from kernel mode! ...";
status = ZwWriteFile(fileHandle, NULL, NULL, NULL,
    &ioStatusBlock, data, sizeof(data) - 1, NULL, NULL);
```

This writes our message into the file. The `IO_STATUS_BLOCK` receives the number of bytes actually written (via `ioStatusBlock.Information`).

#### Step 5e: Close the Handle (Line 145)

```c
ZwClose(fileHandle);
```

**This is critical.** Unlike user-space programs where the OS cleans up handles on process exit, kernel-mode handles must be explicitly closed. A leaked kernel handle can persist until the system is rebooted, potentially locking the file.

---

## The Ring Model — Why This All Matters

```
┌─────────────────────────────────────────────┐
│  Ring 3 — User Space                        │
│  • Your browser, Word, games, Task Manager  │
│  • Protected by virtual memory (MMU)        │
│  • Crash = "program stopped working"        │
│  • Uses Win32 API: CreateFile, printf        │
├─────────────────────────────────────────────┤
│  ← System Call Boundary (syscall/sysenter) → │
├─────────────────────────────────────────────┤
│  Ring 0 — Kernel Space                      │
│  • OUR DRIVER RUNS HERE                     │
│  • Full hardware + memory access            │
│  • Crash = BSOD (Blue Screen of Death)      │
│  • Uses NT Native API: ZwCreateFile, DbgPrint│
└─────────────────────────────────────────────┘
```

When a user-space program calls `CreateFile("C:\\test.txt")`, here's what actually happens:
1. Win32 subsystem translates it to `NtCreateFile(\DosDevices\C:\test.txt)`
2. A `syscall` instruction switches CPU from Ring 3 → Ring 0
3. The kernel's I/O Manager calls the filesystem driver
4. The filesystem driver (which runs in Ring 0, just like ours!) does the actual disk I/O
5. Control returns to Ring 3 with the result

**Our driver skips steps 1–2 entirely** — we call `ZwCreateFile()` directly from Ring 0.

---

## Linux vs Windows — Side-by-Side Comparison

| Feature | Linux Kernel Module | Windows Kernel Driver |
|---------|--------------------|-----------------------|
| Source header | `<linux/init.h>`, `<linux/module.h>` | `<ntddk.h>` |
| Entry function | `module_init(fn)` macro | `DriverEntry()` function |
| Exit function | `module_exit(fn)` macro | `DriverObject->DriverUnload = fn` |
| Log output | `printk(KERN_INFO ...)` | `DbgPrint(...)` |
| View output | `dmesg` | DebugView (Sysinternals) |
| Load command | `sudo insmod module.ko` | `sc create + sc start` |
| Unload command | `sudo rmmod module` | `sc stop + sc delete` |
| Build system | Kernel Makefile | Visual Studio + WDK |
| Code signing | Not required (warning only) | **Required** (must disable to test unsigned) |
| Crash behavior | Kernel Panic (system hangs) | BSOD (blue screen, auto-restarts) |
| Binary format | `.ko` (ELF relocatable object) | `.sys` (PE executable) |
| Success code | `return 0` | `return STATUS_SUCCESS` |

---

## Key OS Concepts

| Concept | Where in Code | What It Does |
|---------|---------------|--------------|
| `DriverEntry()` | Line 183 | Kernel entry point — equivalent of `main()` and Linux's `module_init()` |
| `DriverUnload()` | Line 155 | Cleanup on unload — equivalent of Linux's `module_exit()` |
| `DbgPrint()` | Throughout | Kernel log output — equivalent of `printk()` / `printf()` |
| `NTSTATUS` | Return values | Structured 32-bit error codes used throughout the NT kernel |
| `DRIVER_OBJECT` | Line 183 | Kernel object representing this driver instance |
| `ZwCreateFile()` | Line 113 | NT Native API for kernel-mode file creation |
| `ZwWriteFile()` | Line 133 | NT Native API for kernel-mode file writing |
| `UNICODE_STRING` | Line 85 | Kernel string type (UTF-16, not char*) |
| `OBJECT_ATTRIBUTES` | Line 95 | Describes kernel objects (files, registry keys, etc.) |
| `IO_STATUS_BLOCK` | Lines 113, 133 | Receives asynchronous I/O completion status |

---

## Compilation & Build

### Prerequisites
- Visual Studio 2019/2022 Build Tools (`cl.exe`)
- Windows Driver Kit (WDK) 10.0.19041 (`ntddk.h`, kernel libs)

### Build Commands
```cmd
:: Compile (kernel mode flags)
cl /kernel /GS- /Gz /W4 /I "...\km" /I "...\shared" /c driver.c /Fo:driver.obj

:: Link (native subsystem, DriverEntry as entry point)
link /DRIVER /SUBSYSTEM:NATIVE /ENTRY:DriverEntry /OUT:driver.sys ntoskrnl.lib hal.lib wmilib.lib driver.obj
```

Or simply run the provided script:
```cmd
build_driver.bat
```

### Key Compiler Flags

| Flag | Why |
|------|-----|
| `/kernel` | Generate code suitable for kernel mode (no CRT, no exceptions) |
| `/GS-` | Disable stack buffer overrun checks (not available in kernel) |
| `/Gz` | Use `__stdcall` calling convention (required for kernel callbacks) |
| `/DRIVER` | Linker flag: produce a kernel-mode driver binary |
| `/SUBSYSTEM:NATIVE` | This is a kernel binary, not a console or GUI app |
| `/ENTRY:DriverEntry` | The entry point function name |

---

## Testing Procedure

### Step 1: Disable Driver Signature Enforcement
```
Shift + Click "Restart" → Troubleshoot → Advanced Options →
Startup Settings → Restart → Press F7
```
⚠️ This must be done **every boot session** — it's temporary.

### Step 2: Set Up DebugView
1. Download from: https://learn.microsoft.com/en-us/sysinternals/downloads/debugview
2. Run as **Administrator**
3. Enable: **Capture → Capture Kernel** + **Enable Verbose Kernel Output**

### Step 3: Load and Test (Admin Command Prompt)
```cmd
sc create myDriver binPath= "d:\osproject\OptionalC_WIN32Driver\driver.sys" type= kernel
sc start myDriver       ← DebugView shows "Hello Kernel!"
type C:\kernel_test.txt  ← Should show "Hello from kernel mode!"
sc stop myDriver        ← DebugView shows "Goodbye Kernel!"
sc delete myDriver
```

### Expected DebugView Output
```
[KernelDriver] ==================================================
[KernelDriver] Hello Kernel! Farhan Sadeque
[KernelDriver] DriverEntry called successfully.
[KernelDriver] RegistryPath: \Registry\Machine\System\...
[KernelDriver] ==================================================
[KernelDriver] Attempting ZwCreateFile (kernel-mode file I/O)...
[KernelDriver] ZwCreateFile NTSTATUS = 0x00000000
[KernelDriver] ZwWriteFile NTSTATUS = 0x00000000
[KernelDriver] Bytes written: 133
[KernelDriver] File handle closed.
---
[KernelDriver] ==================================================
[KernelDriver] Goodbye Kernel!
[KernelDriver] Driver has been unloaded successfully.
[KernelDriver] ==================================================
```

---

## Viva Questions & Answers

**Q: Why can't you use `printf()` in a kernel driver?**
A: `printf()` is a C standard library function that writes to `stdout` — a file descriptor concept that only exists in user space. In kernel mode, there is no terminal, no console, and no C runtime library. `DbgPrint()` writes to the kernel debug buffer, which is a ring buffer in kernel memory. You need DebugView or WinDbg to see it.

**Q: What would happen if you wrote `int *ptr = NULL; *ptr = 42;` inside `DriverEntry`?**
A: In user space, this causes a segmentation fault and your program crashes. In kernel space, there is no memory protection — the CPU would attempt to write to address 0, which triggers a **Bug Check** (BSOD). The entire operating system crashes. This is why kernel development is done in virtual machines with snapshots.

**Q: What is the difference between `ZwCreateFile()` and `CreateFile()`?**
A: `CreateFile()` is a Win32 API in `kernel32.dll` — it runs in user space (Ring 3) and internally makes a system call to the kernel. `ZwCreateFile()` is an NT Native API — it runs directly in kernel space (Ring 0) and does not cross any privilege boundary. Our driver uses `ZwCreateFile()` because Win32 APIs are not available in kernel mode.

**Q: Why does `\\DosDevices\\C:\\` work but `C:\\` does not?**
A: The kernel's Object Manager does not understand drive letters. `C:` is a user-space convenience maintained by the Win32 subsystem. `\DosDevices\C:` is a **symbolic link** in the kernel's object namespace that maps `C:` to the actual NT device path (e.g., `\Device\HarddiskVolume2`). Kernel code must always use the full Object Manager path.

**Q: What is NTSTATUS and why is it used instead of returning `int`?**
A: NTSTATUS is a 32-bit structured error code used throughout the Windows kernel. The high bits encode severity (success, info, warning, error), and the low bits encode the specific condition. This is far richer than Unix errno (a simple integer) and allows the kernel to distinguish hundreds of specific failure modes. `NT_SUCCESS(status)` is a macro that checks if the high bit is 0 (meaning success or informational).

**Q: How is this similar to the Linux Kernel Module (Optional A)?**
A: They are directly analogous. `DriverEntry()` ↔ `module_init()`, `DriverUnload()` ↔ `module_exit()`, `DbgPrint()` ↔ `printk()`, DebugView ↔ `dmesg`, `sc create/start` ↔ `insmod`, `sc stop/delete` ↔ `rmmod`. Both run in Ring 0, both can crash the system, and both write to invisible debug buffers.
