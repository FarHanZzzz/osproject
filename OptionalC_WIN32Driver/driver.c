/*
 * Optional C — WIN32 Kernel-Mode Driver
 * CSC413/CSE315 — OS Programming Assignment, Spring 2026
 *
 * Description:
 *   A Windows kernel-mode driver (WDM) that demonstrates:
 *     1. DriverEntry()  — prints "Hello Kernel! <name>" via DbgPrint on load
 *     2. DriverUnload() — prints "Goodbye Kernel!" via DbgPrint on unload
 *     3. [BONUS] ZwCreateFile() + ZwWriteFile() — creates a file from ring 0
 *
 * Build:
 *   Requires Visual Studio 2019/2022 + Windows Driver Kit (WDK).
 *   Build as an "Empty WDM Driver" project in Visual Studio.
 *
 * Load (Admin cmd):
 *   sc create myDriver binPath= "C:\path\to\driver.sys" type= kernel
 *   sc start myDriver
 *   sc stop myDriver
 *   sc delete myDriver
 *
 * View output:
 *   Use Sysinternals DebugView (run as Admin, enable Capture Kernel)
 *
 * OS Concepts Demonstrated:
 *   - Kernel space (ring 0) vs user space (ring 3)
 *   - DriverEntry as the kernel equivalent of main()
 *   - DbgPrint as the kernel equivalent of printf / printk
 *   - NTSTATUS return codes
 *   - Kernel-mode file I/O via NT native APIs (Zw* functions)
 *   - OBJECT_ATTRIBUTES and UNICODE_STRING kernel structures
 *   - IO_STATUS_BLOCK for asynchronous I/O status reporting
 *
 * WARNING:
 *   This code runs in ring 0 with full hardware access.
 *   A bug here WILL cause a Blue Screen of Death (BSOD).
 *   Always test in a Virtual Machine with a snapshot!
 */

#include <ntddk.h>      /* Core Windows kernel development header      */
                         /* Provides: DriverEntry, DbgPrint, NTSTATUS, */
                         /* DRIVER_OBJECT, UNICODE_STRING, Zw* APIs    */

/* ---------- Forward Declarations ---------- */

DRIVER_UNLOAD DriverUnload;

/* ---------- Helper: Write a File from Kernel Mode ---------- */

/*
 * KernelWriteTestFile — demonstrates kernel-mode file creation.
 *
 * This function uses NT Native APIs (the Zw* family) to:
 *   1. Create/overwrite a file at C:\kernel_test.txt
 *   2. Write a text message into it
 *   3. Close the file handle
 *
 * The Zw* functions bypass the Win32 subsystem entirely —
 * they go directly to the NT kernel's I/O manager. This is
 * the same path that kernel-mode drivers use for all file I/O.
 *
 * Key structures:
 *   UNICODE_STRING    — NT kernel uses UTF-16 strings, not char*
 *   OBJECT_ATTRIBUTES — describes the object (file) to create
 *   IO_STATUS_BLOCK   — receives status info from async I/O ops
 *
 * Key NT path format:
 *   "\\DosDevices\\C:\\filename" — the kernel doesn't understand
 *   drive letters natively. \\DosDevices\\ is a symbolic link
 *   that maps to the actual device path (e.g., \\Device\\HarddiskVolume2).
 */
static NTSTATUS KernelWriteTestFile(void)
{
    NTSTATUS          status;
    HANDLE            fileHandle;
    IO_STATUS_BLOCK   ioStatusBlock;
    OBJECT_ATTRIBUTES objAttr;
    UNICODE_STRING    fileName;

    /* Initialize the file path as a kernel UNICODE_STRING */
    RtlInitUnicodeString(&fileName, L"\\DosDevices\\C:\\kernel_test.txt");

    /*
     * Prepare OBJECT_ATTRIBUTES:
     *   OBJ_CASE_INSENSITIVE — file paths are case-insensitive on NTFS
     *   OBJ_KERNEL_HANDLE    — the handle is only valid in kernel mode
     *                          (prevents user-mode processes from using it)
     */
    InitializeObjectAttributes(
        &objAttr,
        &fileName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,       /* RootDirectory — not used (absolute path) */
        NULL        /* SecurityDescriptor — use default         */
    );

    /*
     * ZwCreateFile — the kernel-mode equivalent of CreateFile().
     *
     * Unlike Win32's CreateFile, this call:
     *   - Runs at IRQL PASSIVE_LEVEL (required for file I/O)
     *   - Returns NTSTATUS (not HANDLE/INVALID_HANDLE_VALUE)
     *   - Uses NT path format (\\DosDevices\\...)
     *   - Supports kernel-only handle flags (OBJ_KERNEL_HANDLE)
     *
     * FILE_OVERWRITE_IF = create if doesn't exist, overwrite if it does
     * FILE_SYNCHRONOUS_IO_NONALERT = synchronous I/O (blocks until done)
     */
    status = ZwCreateFile(
        &fileHandle,                /* Receives the file handle              */
        GENERIC_WRITE,              /* Desired access: write only            */
        &objAttr,                   /* Object attributes (path, flags)       */
        &ioStatusBlock,             /* Receives I/O status after completion   */
        NULL,                       /* AllocationSize — use default           */
        FILE_ATTRIBUTE_NORMAL,      /* Normal file attributes                */
        0,                          /* ShareAccess — exclusive access         */
        FILE_OVERWRITE_IF,          /* Disposition — create or overwrite      */
        FILE_SYNCHRONOUS_IO_NONALERT, /* CreateOptions — synchronous I/O     */
        NULL,                       /* EaBuffer — no extended attributes      */
        0                           /* EaLength                               */
    );

    DbgPrint("[KernelDriver] ZwCreateFile NTSTATUS = 0x%08X\n", status);

    if (NT_SUCCESS(status))
    {
        /*
         * Write a message into the file.
         *
         * ZwWriteFile parameters:
         *   fileHandle   — from ZwCreateFile
         *   Event        — NULL (not used with synchronous I/O)
         *   ApcRoutine   — NULL (no APC callback)
         *   ApcContext   — NULL
         *   IoStatusBlock — receives bytes written
         *   Buffer       — data to write
         *   Length       — number of bytes (excluding null terminator)
         *   ByteOffset   — NULL (append from current position)
         *   Key          — NULL (not used)
         */
        CHAR data[] = "Hello from kernel mode! This file was created by a WDM driver running in ring 0.\n"
                      "If you can read this, the ZwCreateFile + ZwWriteFile calls succeeded!\n";

        status = ZwWriteFile(
            fileHandle,
            NULL, NULL, NULL,
            &ioStatusBlock,
            data,
            sizeof(data) - 1,   /* -1 to exclude null terminator         */
            NULL,                /* ByteOffset — append at current pos    */
            NULL                 /* Key — not used                        */
        );

        DbgPrint("[KernelDriver] ZwWriteFile NTSTATUS = 0x%08X\n", status);
        DbgPrint("[KernelDriver] Bytes written: %llu\n",
                 (unsigned long long)ioStatusBlock.Information);

        /* Always close the file handle to prevent kernel handle leak */
        ZwClose(fileHandle);
        DbgPrint("[KernelDriver] File handle closed.\n");
    }
    else
    {
        DbgPrint("[KernelDriver] ZwCreateFile FAILED — file was NOT created.\n");
    }

    return status;
}


/* ---------- Driver Unload ---------- */

/*
 * DriverUnload — called when the driver is unloaded (sc stop).
 *
 * This is the kernel equivalent of module_exit() in Linux.
 * The DRIVER_OBJECT pointer is received but not used here
 * (it would be used to clean up device objects, IRPs, etc.
 * in a real driver).
 */
void DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    DbgPrint("==================================================\n");
    DbgPrint("[KernelDriver] Goodbye Kernel!\n");
    DbgPrint("[KernelDriver] Driver has been unloaded successfully.\n");
    DbgPrint("==================================================\n");
}


/* ---------- Driver Entry Point ---------- */

/*
 * DriverEntry — the kernel-mode equivalent of main() / WinMain().
 *
 * This function is called by the I/O Manager when the driver is loaded
 * (via sc start). It receives:
 *   DriverObject  — a kernel object representing this driver instance
 *   RegistryPath  — the driver's registry key (for configuration)
 *
 * Must return STATUS_SUCCESS (0x00000000) for the driver to remain loaded.
 * Any other NTSTATUS causes the I/O Manager to unload the driver immediately.
 *
 * IRQL: Called at PASSIVE_LEVEL (lowest interrupt level — safe for file I/O)
 *
 * Comparison with Linux:
 *   DriverEntry     ↔ module_init()
 *   DriverUnload    ↔ module_exit()
 *   DbgPrint        ↔ printk()
 *   DebugView       ↔ dmesg
 *   sc create/start ↔ insmod
 *   sc stop/delete  ↔ rmmod
 */
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    /* Register the unload function so the driver can be stopped cleanly */
    DriverObject->DriverUnload = DriverUnload;

    /* Print the "Hello Kernel" message — viewed via DebugView */
    DbgPrint("==================================================\n");
    DbgPrint("[KernelDriver] Hello Kernel! Farhan Sadeque\n");
    DbgPrint("[KernelDriver] DriverEntry called successfully.\n");
    DbgPrint("[KernelDriver] RegistryPath: %wZ\n", RegistryPath);
    DbgPrint("==================================================\n");

    /*
     * [EXTREMELY DIFFICULT BONUS] Attempt kernel-mode file creation.
     *
     * This calls ZwCreateFile + ZwWriteFile to create C:\kernel_test.txt
     * If this causes a BSOD, you have still demonstrated kernel-mode
     * execution — the professor explicitly says this counts.
     *
     * Comment out the line below if you want the safe version only.
     */
    DbgPrint("[KernelDriver] Attempting ZwCreateFile (kernel-mode file I/O)...\n");
    KernelWriteTestFile();

    return STATUS_SUCCESS;
}
