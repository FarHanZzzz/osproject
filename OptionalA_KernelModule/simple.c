#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hash.h>   // For GOLDEN_RATIO_PRIME
#include <linux/gcd.h>    // For gcd()

/* This function is called when the module is loaded. */
int simple_init(void)
{
    printk(KERN_INFO "========================================\n");
    printk(KERN_INFO "Loading Kernel Module\n");
    printk(KERN_INFO "GOLDEN_RATIO_PRIME = %lu\n", GOLDEN_RATIO_PRIME);
    printk(KERN_INFO "========================================\n");
    return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void)
{
    printk(KERN_INFO "========================================\n");
    printk(KERN_INFO "Removing Kernel Module\n");
    printk(KERN_INFO "GCD(3700, 24) = %lu\n", gcd(3700, 24));
    printk(KERN_INFO "========================================\n");
}

/* Macros for registering module entry and exit points. */
module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module for OS Project Optional A");
MODULE_AUTHOR("Farhan Sadeque");
