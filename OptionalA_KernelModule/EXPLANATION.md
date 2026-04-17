# Optional A — Linux Kernel Module
## How the Code Works — Complete Explanation

> **Source:** `simple.c` | **Language:** C | **OS Concepts:** Kernel Space vs User Space, Dynamic Kernel Extensions, Ring Buffer

---

## What This Program Does (And Why There is No UI)

This program creates a **Linux Kernel Module**. 

Unlike normal programs (like web browsers or your Word Counter task), a kernel module does not run in "User Space" and it does not have a UI. It doesn't even have a standard terminal output!

It runs entirely in **Kernel Space (Ring 0)**. This means it has absolute, unrestricted access to the computer's hardware. Because it runs deep inside the operating system itself, it cannot use `printf()` to print to the screen. Instead, it uses `printk()` to silently log messages directly to the kernel's internal ring buffer.

We only see the output when we use the `dmesg` command to read the kernel's hidden logs.

---

## How It Works — Step by Step

### 1. The Headers (Lines 1–5)

```c
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hash.h>
#include <linux/gcd.h>
```

We include special `<linux/...>` headers. These are not standard C library headers. They give us access to internal operating system functions, such as kernel hashing constants (`GOLDEN_RATIO_PRIME`) and kernel math utilities (`gcd()`).

### 2. The Initialization Function (Lines 7–15)

```c
int simple_init(void) {
    printk(KERN_INFO "Loading Kernel Module\n");
    printk(KERN_INFO "GOLDEN_RATIO_PRIME = %lu\n", GOLDEN_RATIO_PRIME);
    return 0;
}
```

This is the equivalent of `main()` for a kernel module, but it is only called exactly once: the moment you run `sudo insmod simple.ko`.
- We use `printk` instead of `printf`.
- `KERN_INFO` sets the priority/severity of the log message.
- We print `GOLDEN_RATIO_PRIME`, a 64-bit constant derived from the mathematical golden ratio, used internally by Linux to optimize its hash tables.
- It returns `0` to tell the OS that the module loaded successfully.

### 3. The Exit Function (Lines 17–24)

```c
void simple_exit(void) {
    printk(KERN_INFO "Removing Kernel Module\n");
    printk(KERN_INFO "GCD(3700, 24) = %lu\n", gcd(3700, 24));
}
```

This function is called exactly once: the moment you run `sudo rmmod simple`. 
It calculates the Greatest Common Divisor of 3700 and 24 using the kernel's built-in `gcd()` function (which evaluates to 4) and logs it. It acts as the cleanup phase.

### 4. Registering the Hooks (Lines 26–28)

```c
module_init(simple_init);
module_exit(simple_exit);
```

The kernel needs to know which functions to call when loading and unloading. These macros register our custom `simple_init` and `simple_exit` functions into the kernel's module table.

### 5. Module Metadata (Lines 30–32)

```c
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module for OS Project Optional A");
MODULE_AUTHOR("Farhan Sadeque");
```

This metadata is required. If `MODULE_LICENSE("GPL")` is missing, the kernel will complain that you are loading a "tainted" or proprietary module and may refuse to load it or disable certain features.

---

## Key OS Concepts

| Concept | Explanation |
|---------|-------------|
| **Kernel Space vs User Space** | User space programs are sandboxed. Kernel space programs have full hardware access. A crash in user space closes the app. A crash in kernel space (like a null pointer in this module) causes a **Kernel Panic** (the Linux Blue Screen of Death). |
| **Dynamic Kernel Extensions** | In the old days, adding new OS features required recompiling the whole kernel and rebooting. Kernel modules (`.ko` files) allow us to dynamically inject new code into a live, running kernel without rebooting. |
| **`printk` and the Ring Buffer** | Kernel code cannot write to your terminal window. It writes to a circular memory buffer in RAM. `dmesg` is the tool used to read that buffer. |

---

## Test Results

When the commands are run, the module behaves exactly as expected:

1. `$ sudo insmod simple.ko`
   - Automatically executes `simple_init`.
   - Logs `GOLDEN_RATIO_PRIME` invisibly.

2. `$ sudo rmmod simple`
   - Automatically executes `simple_exit`.
   - Logs `GCD(3700, 24) = 4` invisibly.

3. `$ sudo dmesg | tail -5`
   - Reveals the hidden kernel logs, proving both the load and unload functions executed properly.

---

## Viva Questions & Answers

**Q: Why didn't anything print on the screen when I ran `insmod`?**
A: Because kernel modules do not have standard output (`stdout`) or a user interface. They run in Ring 0 (Kernel Space). The `printk()` function writes directly to the kernel ring buffer, which must be manually read using the `dmesg` command.

**Q: What does the `module_init()` macro actually do?**
A: It tells the kernel exactly which function should serve as the entry point. When `insmod` is called, the kernel looks at the pointer registered by `module_init` and executes that function to start the module.

**Q: What would happen if you wrote `int *ptr = NULL; *ptr = 10;` inside the `simple_init` function?**
A: Because this code runs in Kernel Space, memory protections do not apply in the same way. A null pointer dereference here wouldn't just cause a "Segmentation Fault"—it would likely cause a total system crash known as a Kernel Panic, requiring a hard reboot of the machine.

**Q: Why do we have to compile this with a special Makefile instead of just running `gcc simple.c`?**
A: A kernel module must be linked against the exact kernel headers matching your running operating system version. The special Makefile uses `make -C /lib/modules/$(uname -r)/build` to invoke the kernel's own internal build system to compile our module correctly.
