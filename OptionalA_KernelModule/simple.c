// ============================================================
// Optional A — Simple Linux Kernel Module
// ============================================================
// PROBLEM (from PDF): Write a simple kernel module that prints
// a message when loaded and another message when removed.
//
// HOW WE SOLVE IT:
//   1. Write an init function that runs when the module is loaded.
//   2. Write an exit function that runs when the module is removed.
//   3. Use printk() to print messages to the kernel log.
//   4. Use module_init() and module_exit() to register our functions.
//
// Build:  make
// Load:   sudo insmod simple.ko
// Check:  sudo dmesg | tail -5
// Remove: sudo rmmod simple
// ============================================================

#include <linux/init.h>     // For __init and __exit
#include <linux/kernel.h>   // For printk()
#include <linux/module.h>   // For module_init() and module_exit()

// ======================================================
// STEP 1: The INIT function
// ======================================================
// This function runs when you load the module with:
//   sudo insmod simple.ko
//
// printk() is like printf() but for kernel code.
// KERN_INFO means "this is just an informational message."
// You can see the output by running: sudo dmesg | tail
static int __init simple_init(void)
{
    printk(KERN_INFO "Hello! Kernel module loaded.\n");
    return 0;  // Return 0 means "loaded successfully"
}

// ======================================================
// STEP 2: The EXIT function
// ======================================================
// This function runs when you remove the module with:
//   sudo rmmod simple
static void __exit simple_exit(void)
{
    printk(KERN_INFO "Goodbye! Kernel module removed.\n");
}

// ======================================================
// STEP 3: Register our functions with the kernel
// ======================================================
// These two lines tell the kernel:
//   "When loaded, call simple_init()"
//   "When removed, call simple_exit()"
module_init(simple_init);
module_exit(simple_exit);

// ======================================================
// STEP 4: Module info (required by the kernel)
// ======================================================
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Kernel Module for OS Project");
MODULE_AUTHOR("Farhan Sadeque");
