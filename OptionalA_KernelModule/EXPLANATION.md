# 🧩 Optional A — Linux Kernel Module (ELI5 Edition)

## The Story

Imagine your computer is a **building**:
- The **lobby** (user space) is where regular people (programs) hang out. They can use the elevator, the vending machine, but they can't go into the engine room.
- The **engine room** (kernel space) is where the power, plumbing, and wiring are. Only authorized technicians (kernel code) can go in. If a technician messes up, the whole building can collapse!

A **kernel module** is like a **temporary technician** you hire. You don't need to rebuild the building to add them — you just let them in (`insmod`), they do their thing, and then you kick them out (`rmmod`).

Our module does two things:
1. When hired: "Here's a secret number from the engine room: GOLDEN_RATIO_PRIME"
2. When fired: "By the way, the GCD of 3700 and 24 is 4"

---

## 🔧 How to Run It

```bash
cd OptionalA_KernelModule

# Step 1: Build (MUST use make — can't use plain gcc for kernel modules)
make

# Step 2: Clear old kernel messages
sudo dmesg -c

# Step 3: Hire the technician (load the module)
sudo insmod simple.ko

# Step 4: See their work
dmesg
# Expected: "Loading Kernel Module"
#           "GOLDEN_RATIO_PRIME = 11400714819323198485"

# Step 5: Fire the technician (remove the module)
sudo rmmod simple

# Step 6: See their goodbye message
dmesg
# Expected: "Removing Kernel Module"
#           "GCD(3700, 24) = 4"

# Step 7: Verify they left
lsmod | grep simple    # Should show nothing
```

---

## 📖 Code Walkthrough — The Fun Version

### Part 1: Special Engine Room Headers

```c
#include <linux/init.h>     // For module_init() and module_exit()
#include <linux/kernel.h>   // For printk()
#include <linux/module.h>   // For MODULE_LICENSE
#include <linux/hash.h>     // For GOLDEN_RATIO_PRIME (engine room secret!)
#include <linux/gcd.h>      // For gcd() function (engine room tool!)
```

⚠️ **These are NOT normal C headers!** You can't `#include <stdio.h>` in kernel code. The kernel is its own world — no `printf`, no `malloc`, no standard library. Everything has a kernel equivalent:
- `printf()` → `printk()`
- `malloc()` → `kmalloc()`
- `<stdio.h>` → `<linux/kernel.h>`

---

### Part 2: When the Technician Arrives

```c
int simple_init(void) {
    printk(KERN_INFO "Loading Kernel Module\n");
    printk(KERN_INFO "GOLDEN_RATIO_PRIME = %lu\n", GOLDEN_RATIO_PRIME);
    return 0;    // 0 = "I'm in! All good."
}
```

🚪 **`simple_init` = the technician walking into the engine room.** This runs when you type `sudo insmod simple.ko`.

**What is `printk`?** It's `printf` but for the kernel. The output doesn't go to your screen — it goes to a hidden **kernel log**. You read this log with the `dmesg` command.

**What is `KERN_INFO`?** A priority level. The kernel log has levels: EMERGENCY > ALERT > ERROR > WARNING > NOTICE > **INFO** > DEBUG. We use INFO because it's just an informational message.

**What is GOLDEN_RATIO_PRIME?** A number (≈ 11400714819323198485) used inside the kernel for hash tables. The whole point is: **you can only access this value from kernel space.** A normal program can't see it. This proves our code is running inside the kernel!

**Why return 0?** 0 = success. If we return anything else, the module fails to load.

---

### Part 3: When the Technician Leaves

```c
void simple_exit(void) {
    printk(KERN_INFO "Removing Kernel Module\n");
    printk(KERN_INFO "GCD(3700, 24) = %lu\n", gcd(3700, 24));
}
```

👋 **`simple_exit` = the technician leaving.** This runs when you type `sudo rmmod simple`.

**`gcd()` is a kernel function** from `<linux/gcd.h>`. Normal programs would have to write their own GCD. The kernel has one built-in. GCD(3700, 24) = 4. (Because 3700 = 4×925 and 24 = 4×6).

---

### Part 4: The Registration Desk

```c
module_init(simple_init);    // "When module loads, call simple_init"
module_exit(simple_exit);    // "When module unloads, call simple_exit"
```

📋 These macros tell the kernel: *"Hey, when someone loads this module, run `simple_init`. When they remove it, run `simple_exit`."* Without these, the kernel wouldn't know which functions to call.

---

### Part 5: The Paperwork

```c
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module for OS Project Optional A");
MODULE_AUTHOR("Farhan Sadeque");
```

📄 **Why GPL?** Linux requires every kernel module to declare its license. If you don't say `"GPL"`, the kernel marks your module as **"tainted"** (suspicious), and some kernel features become unavailable. It's like not having a badge — security won't let you into certain rooms.

---

### Part 6: The Makefile (Why Can't We Use gcc?)

```makefile
obj-m += simple.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

🔨 **Why not `gcc simple.c`?** Kernel modules aren't normal programs. They must be compiled against YOUR EXACT kernel version. The Makefile says: *"Go to `/lib/modules/your-kernel-version/build/` (where the kernel source lives) and use the kernel's build system (kbuild) to compile `simple.c` into `simple.ko`."*

`.ko` = **K**ernel **O**bject. It's a special format the kernel knows how to load.

---

## 🧠 Concepts Cheat Sheet

| Thing | ELI5 Version |
|-------|-------------|
| Kernel Space | The engine room. Full power. Full danger. |
| User Space | The lobby. Safe but restricted. |
| Kernel Module | A plugin you load/unload without rebooting |
| `printk()` | `printf()` but for the kernel. Output goes to `dmesg`. |
| `insmod` | "Let this technician into the engine room" |
| `rmmod` | "Kick this technician out" |
| `dmesg` | "Show me the engine room's logbook" |
| `lsmod` | "Who's currently working in the engine room?" |
| GPL License | Your entry badge. Without it, some doors stay locked. |
| Kernel Panic | The technician broke something and the building collapsed 💥 |
