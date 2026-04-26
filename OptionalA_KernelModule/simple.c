// ============================================================
// Optional A — Simple Linux Kernel Module
// ============================================================
// PROBLEM (from PDF):
//   1. Print GOLDEN_RATIO_PRIME when the module is loaded.
//   2. Print GCD of 3700 and 24 when the module is removed.
//   3. Basic load/unload with printk messages.
//
// Build:  make
// Load:   sudo insmod simple.ko
// Check:  sudo dmesg | tail -5
// Remove: sudo rmmod simple
// Check:  sudo dmesg | tail -5
// ============================================================

#include <linux/init.h>   // For __init and __exit
#include <linux/kernel.h> // For printk()
#include <linux/module.h> // For module_init() and module_exit()
#include <linux/hash.h>   // For GOLDEN_RATIO_PRIME
#include <linux/gcd.h>    // For gcd()

// ======================================================
// INIT function — runs when module is loaded (insmod)
// ======================================================
static int __init simple_init(void) {
    printk(KERN_INFO "Loading Kernel Module\n");

    // Print the value of GOLDEN_RATIO_PRIME (from linux/hash.h)
    // This is a kernel-only constant used for hash table calculations
    printk(KERN_INFO "GOLDEN_RATIO_PRIME = %lu\n", GOLDEN_RATIO_PRIME);

    return 0; // 0 = loaded successfully
}

// ======================================================
// EXIT function — runs when module is removed (rmmod)
// ======================================================
static void __exit simple_exit(void) {
    // Print the GCD of 3700 and 24 using the kernel's gcd() function
    // gcd(3700, 24) = 4
    printk(KERN_INFO "GCD of 3700 and 24 = %lu\n", gcd(3700, 24));

    printk(KERN_INFO "Removing Kernel Module\n");
}

// Register entry and exit points
module_init(simple_init);
module_exit(simple_exit);

// Module metadata (required by kernel)
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Kernel Module for OS Project");
MODULE_AUTHOR("Farhan Sadeque");
