# 🧩 Optional A — Kernel Module (ELI5 Edition)

> **Quick Overview:** This is a Linux kernel module that prints `GOLDEN_RATIO_PRIME` when loaded and `GCD(3700, 24)` when removed. It demonstrates kernel space programming, `printk()`, and dynamic kernel extensions. **Key OS concept: Kernel Space vs User Space.**

---

## 🎯 The Big Picture

```
You type: sudo insmod simple.ko
  → simple_init() runs
  → Prints: "Loading Kernel Module"
  → Prints: "GOLDEN_RATIO_PRIME = <value>"

You type: sudo dmesg | tail -5
  → You see both messages

You type: sudo rmmod simple
  → simple_exit() runs
  → Prints: "GCD of 3700 and 24 = 4"
  → Prints: "Removing Kernel Module"
```

---

## 🔧 How to Run It

```bash
# Step 1: Build the module
make

# Step 2: Load it into the kernel (needs sudo!)
sudo insmod simple.ko

# Step 3: See the messages in the kernel log
sudo dmesg | tail -5

# Step 4: Remove the module
sudo rmmod simple

# Step 5: See the exit messages
sudo dmesg | tail -5
```

**Prerequisites:** You need Linux kernel headers installed:
```bash
sudo apt install linux-headers-$(uname -r)
```

---

## 📖 Code Walkthrough

### Part 1: Kernel-Only Headers

```c
#include <linux/init.h>    // For __init and __exit
#include <linux/kernel.h>  // For printk()
#include <linux/module.h>  // For module_init() and module_exit()
#include <linux/hash.h>    // For GOLDEN_RATIO_PRIME (kernel-only!)
#include <linux/gcd.h>     // For gcd() (kernel-only!)
```

📚 These are NOT normal C headers. You can't use `<stdio.h>` or `printf()` in kernel code. The kernel has its own separate API.

---

### Part 2: The Init Function (with GOLDEN_RATIO_PRIME)

```c
static int __init simple_init(void) {
    printk(KERN_INFO "Loading Kernel Module\n");
    printk(KERN_INFO "GOLDEN_RATIO_PRIME = %lu\n", GOLDEN_RATIO_PRIME);
    return 0;
}
```

🔑 **`GOLDEN_RATIO_PRIME`** is a constant defined in `<linux/hash.h>`. It's used internally by the kernel for hash table calculations. This value is only accessible from kernel space — user-space programs can't see it!

---

### Part 3: The Exit Function (with GCD)

```c
static void __exit simple_exit(void) {
    printk(KERN_INFO "GCD of 3700 and 24 = %lu\n", gcd(3700, 24));
    printk(KERN_INFO "Removing Kernel Module\n");
}
```

🧮 **`gcd()`** from `<linux/gcd.h>` computes the Greatest Common Divisor. `gcd(3700, 24) = 4`. This function is part of the kernel's math library — not available in user space.

---

### Part 4: Registration and Metadata

```c
module_init(simple_init);   // "Call simple_init() when loaded"
module_exit(simple_exit);   // "Call simple_exit() when removed"

MODULE_LICENSE("GPL");      // Required — kernel marks non-GPL as "tainted"
MODULE_DESCRIPTION("Simple Kernel Module for OS Project");
MODULE_AUTHOR("Farhan Sadeque");
```

---

## OS Concepts Covered

### Kernel Space vs User Space
- **User Space:** Where normal programs run. Protected from each other. A crash kills only that program.
- **Kernel Space:** Where the OS core runs. Full hardware access. A crash here freezes the entire system.

### Dynamic Kernel Extensions (LKMs)
- Linux allows adding/removing code from the running kernel without rebooting
- `insmod` loads a module, `rmmod` removes it
- This is how device drivers, filesystems, and network protocols are added dynamically

### printk vs printf
- `printf()` → user space, output to terminal
- `printk()` → kernel space, output to kernel ring buffer (viewable via `dmesg`)

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| User Space | Safe zone where normal programs live. Crashes only kill that program. |
| Kernel Space | Danger zone where the OS runs. Crashes = system freeze. |
| Kernel Module | Code plugged into the running kernel without rebooting. |
| `printk()` | Kernel's `printf()`. Output goes to kernel log (`dmesg`). |
| `GOLDEN_RATIO_PRIME` | Kernel-only constant used for hash calculations. |
| `gcd()` | Kernel-only function to compute greatest common divisor. |
| `insmod` | Load a `.ko` module into the kernel. Needs sudo. |
| `rmmod` | Remove a module from the kernel. Needs sudo. |
| `dmesg` | View the kernel log where `printk()` messages appear. |
| `__init` / `__exit` | Annotations that let the kernel free memory after one-time use. |
| `MODULE_LICENSE("GPL")` | Required. Without it, kernel marks the module as "tainted." |
