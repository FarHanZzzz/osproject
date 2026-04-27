// ============================================================
// Optional A — Simple Linux Kernel Module
// ============================================================
//
// THE PROBLEM:
//   The assignment asks us to:
//   1. Write a kernel module that loads and unloads from the Linux kernel.
//   2. On load  (insmod): print the GOLDEN_RATIO_PRIME constant.
//   3. On unload (rmmod): print the GCD of 3700 and 24.
//
// WHAT IS A KERNEL MODULE?
//   Normal programs run in "user space" — they can't directly access
//   hardware or kernel internals. A kernel module runs in "kernel space"
//   — it IS part of the OS kernel while loaded, with full hardware access.
//   The advantage: you can add/remove functionality from a running kernel
//   WITHOUT rebooting (e.g., device drivers, file systems).
//
// KEY DIFFERENCE: printf() vs printk()
//   User space programs use printf() which sends output to the terminal.
//   Kernel space uses printk() which sends output to the KERNEL LOG BUFFER.
//   To view kernel log output, run: sudo dmesg | tail -10
//   KERN_INFO is a priority level ("informational message").
//
// HOW TO BUILD AND USE:
//   Build:    make                         (compiles simple.c into simple.ko)
//   Load:     sudo insmod simple.ko        (inserts module into running kernel)
//   View:     sudo dmesg | tail -5         (see the load message + GOLDEN_RATIO_PRIME)
//   Unload:   sudo rmmod simple            (removes module from kernel)
//   View:     sudo dmesg | tail -5         (see the unload message + GCD result)
//   Clean:    sudo dmesg -c               (clear the kernel log buffer)
//
// THE TWO KERNEL-ONLY FEATURES THIS MODULE DEMONSTRATES:
//   1. GOLDEN_RATIO_PRIME — a constant from <linux/hash.h> used in kernel
//      hash tables. Only available inside the kernel, not in user programs.
//   2. gcd(a, b) — a function from <linux/gcd.h>. The kernel has its own
//      math functions separate from the C standard library.
// ============================================================

#include <linux/init.h>   // Provides the __init and __exit macros.
                          // __init tells the kernel to free this function's
                          // memory after the module is loaded (saves RAM).
                          // __exit tells the kernel this is the cleanup function.

#include <linux/kernel.h> // Provides printk() — the kernel's version of printf().
                          // Output goes to the kernel log buffer, not the terminal.

#include <linux/module.h> // Provides module_init(), module_exit(), and the
                          // MODULE_LICENSE/DESCRIPTION/AUTHOR macros.
                          // This header is REQUIRED for every kernel module.

#include <linux/hash.h>   // Provides GOLDEN_RATIO_PRIME — a special constant
                          // used in the kernel's hash table implementations.
                          // Only available in kernel space, not user programs.

#include <linux/gcd.h>    // Provides gcd(a, b) — computes the greatest common
                          // divisor of two unsigned long integers.
                          // This is the kernel's own math utility function.

// ======================================================
// MODULE INIT FUNCTION — called when the module is loaded
// ======================================================
// The __init macro marks this function so the kernel can discard it
// from memory after loading (optimization — saves kernel RAM).
// Must return 0 on success, or a negative error code on failure.
// ======================================================
static int __init simple_init(void) {
    // printk() sends text to the kernel log buffer (view with: sudo dmesg)
    // KERN_INFO is a log level meaning "informational" (not an error/warning)
    // "\n" is needed because printk doesn't add a newline automatically
    printk(KERN_INFO "Loading Kernel Module\n");

    // GOLDEN_RATIO_PRIME is defined in <linux/hash.h>
    // It is a large prime number close to 2^64 * (golden ratio - 1)
    // Used internally for hash table indexing.
    // %lu = format specifier for unsigned long integer
    // On a 64-bit system this value is: 11400714819323198485
    // On a 32-bit system this value is: 2654435761
    printk(KERN_INFO "GOLDEN_RATIO_PRIME = %lu\n", GOLDEN_RATIO_PRIME);

    return 0; // 0 means the module loaded successfully
              // Any other value means failure (module won't load)
}

// ======================================================
// MODULE EXIT FUNCTION — called when the module is removed
// ======================================================
// The __exit macro marks this as the cleanup function.
// The kernel calls this when you run: sudo rmmod simple
// Return type is void — exit functions cannot fail/return errors.
// ======================================================
static void __exit simple_exit(void) {
    // gcd(3700, 24) uses the kernel's built-in GCD function.
    // Math: 3700 = 4 * 925, 24 = 4 * 6. The GCD is 4.
    // We cast to unsigned long to match the function signature in <linux/gcd.h>
    // which expects: unsigned long gcd(unsigned long a, unsigned long b)
    printk(KERN_INFO "GCD of 3700 and 24 = %lu\n", gcd(3700, 24));

    printk(KERN_INFO "Removing Kernel Module\n");
    // No return value — void function
}

// ======================================================
// REGISTER ENTRY AND EXIT POINTS WITH THE KERNEL
// ======================================================
// module_init(func) — tells the kernel: "call func when insmod runs"
// module_exit(func) — tells the kernel: "call func when rmmod runs"
// Without these two lines, the kernel doesn't know which functions
// to call on load/unload.
module_init(simple_init);
module_exit(simple_exit);

// ======================================================
// MODULE METADATA (required — compilation fails without LICENSE)
// ======================================================
// MODULE_LICENSE: kernel will warn if "GPL" is not set because many
// kernel functions are only exported to GPL-compatible modules.
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Kernel Module for OS Project"); // Shown in modinfo
MODULE_AUTHOR("Student");                                   // Shown in modinfo
