# Optional A — Linux Kernel Module: Viva Preparation

---

## 🧠 What Is The Problem?

The assignment asks: *"Create a Linux kernel module that prints GOLDEN_RATIO_PRIME when loaded, and the GCD of 3700 and 24 when unloaded."*

This teaches you how to write code that runs INSIDE the operating system kernel itself — not as a normal user program, but as a piece of the OS.

This tests your understanding of:
- The difference between user space and kernel space
- How kernel modules extend the OS without rebooting
- How `printk()` differs from `printf()`
- How to build, load, and unload a kernel module

---

## 💬 How To Explain The Problem (Your Opening Line)

> *"The problem is to write a basic Linux kernel module — a piece of code that loads directly into the running kernel. When I insert it using `insmod`, it prints the GOLDEN_RATIO_PRIME constant from the kernel's hash library. When I remove it using `rmmod`, it prints the GCD of 3700 and 24 using the kernel's own math function. Both outputs are only visible in the kernel log, viewable with `dmesg`."*

---

## ⚙️ How Your Code Solves It — Step By Step

### Step 1 — Two Functions: init and exit
- `simple_init()` → called when the module is loaded (`sudo insmod simple.ko`)
- `simple_exit()` → called when the module is removed (`sudo rmmod simple`)
- These are registered with the kernel using `module_init()` and `module_exit()` macros

### Step 2 — Printing GOLDEN_RATIO_PRIME
- Comes from `#include <linux/hash.h>` — a kernel-only header
- Not accessible in user-space programs at all
- Printed with `printk(KERN_INFO "GOLDEN_RATIO_PRIME = %lu\n", GOLDEN_RATIO_PRIME)`
- On 64-bit systems: value is `11400714819323198485`

### Step 3 — Printing GCD of 3700 and 24
- Comes from `#include <linux/gcd.h>` — another kernel-only header
- `gcd(3700, 24)` returns `4` (since both 3700 and 24 are divisible by 4)
- Printed in the exit function with `printk()`

### Step 4 — Building with Make
```
make
```
- The Makefile uses the running kernel's build system at `/lib/modules/$(uname -r)/build`
- Produces `simple.ko` — the compiled kernel module object file

### Step 5 — Load, Check, Unload
```bash
sudo insmod simple.ko    # Load: triggers simple_init()
sudo dmesg | tail -5     # See: "Loading Kernel Module" + GOLDEN_RATIO_PRIME
sudo rmmod simple        # Unload: triggers simple_exit()
sudo dmesg | tail -5     # See: GCD result + "Removing Kernel Module"
```

---

## 🔥 Challenges & What You'd Say You Faced

> *"The biggest challenge was understanding the build process. You don't compile a kernel module like a normal C program with `gcc simple.c`. Instead, you use a special Makefile that invokes the kernel's own build infrastructure. The `obj-m += simple.o` line tells the kernel build system to compile simple.c as a module."*

> *"I also had to understand that `printf()` doesn't exist in kernel space. The kernel has its own function called `printk()` which writes to an internal ring buffer (the kernel log). You can only read this buffer with `sudo dmesg`. The priority prefix `KERN_INFO` is required — without it, the kernel wouldn't know how urgently to treat the message."*

> *"Understanding `__init` and `__exit` was also new. These are macros that tell the kernel build system to put these functions in special memory sections. After `insmod` loads the module and calls `simple_init()`, the kernel frees the memory used by `__init` functions — they're never needed again. This saves RAM."*

---

## 🤔 Choices You Made & Why

| Choice | Why |
|--------|-----|
| `static int __init` for init function | `static` = not exported outside module; `__init` = freed after use |
| `static void __exit` for exit function | `void` because exit functions cannot return error codes |
| `KERN_INFO` log level | "Informational" severity — appropriate for non-error output |
| `%lu` format in printk | GOLDEN_RATIO_PRIME and gcd() both return `unsigned long` |
| `MODULE_LICENSE("GPL")` | Required — kernel will warn/fail to export symbols without a valid license |
| `module_init()` and `module_exit()` | Tell the kernel which functions to call on load and unload |

---

## 📚 Concepts You Need To Know

### User Space vs Kernel Space

| User Space | Kernel Space |
|------------|-------------|
| Normal programs (your apps, terminal) | The OS kernel itself |
| Protected — can't access hardware directly | Has full hardware access |
| Uses system calls to ask kernel for services | Runs system calls |
| Crashes are isolated — only that program dies | A crash can take down the ENTIRE system (kernel panic) |
| Uses printf(), malloc(), fopen() | Uses printk(), kmalloc(), filp_open() |

> **Analogy**: User space is like a customer in a bank — you fill out a form and ask the teller (kernel) to do things for you. Kernel space is the teller — they have access to the vault, the systems, everything. If the teller makes a mistake, the whole bank stops.

### Kernel Module
- A piece of code that can be dynamically loaded into and removed from the running kernel
- No reboot required (unlike recompiling the kernel itself)
- Used for: device drivers, file systems, network protocols
- The `.ko` extension = **K**ernel **O**bject file
- Commands:
  - `insmod simple.ko` — insert module
  - `rmmod simple` — remove module
  - `lsmod` — list loaded modules
  - `modinfo simple.ko` — show module metadata

### `printk()` vs `printf()`
- `printf()`: sends text to standard output (your terminal) — only works in user space
- `printk()`: sends text to the **kernel log buffer** — only works in kernel space
- View kernel log: `sudo dmesg` or `sudo dmesg | tail -20`
- Clear kernel log: `sudo dmesg -c`
- `KERN_INFO` prefix means "informational" — other levels: `KERN_WARNING`, `KERN_ERR`, `KERN_DEBUG`

### GOLDEN_RATIO_PRIME
- A constant defined in `<linux/hash.h>`
- Its value ≈ 2^64 × (golden ratio − 1) on 64-bit systems
- The golden ratio is approximately 1.618...
- Used in the kernel's hash tables for distributing keys evenly
- On 64-bit: `11400714819323198485`
- On 32-bit: `2654435761`

### GCD (Greatest Common Divisor)
- GCD(a, b) = the largest number that divides both a and b without a remainder
- Example: GCD(3700, 24)
  - 3700 = 4 × 925
  - 24   = 4 × 6
  - GCD  = 4
- The kernel's `gcd()` function in `<linux/gcd.h>` computes this
- It uses the **Euclidean algorithm**: repeatedly replace (a, b) with (b, a mod b) until b=0

### The Makefile
```makefile
obj-m += simple.o
all:
    make -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) modules
clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) clean
```
- `obj-m += simple.o` → "compile simple.c as a MODULE (not built-in)"
- `-C /lib/modules/$(uname -r)/build` → use the current kernel's build system
- `M=$(CURDIR)` → "build files are in my current directory"
- `$(shell uname -r)` → substitutes the running kernel version string at build time

### `module_init()` and `module_exit()`
- These are **macros** (not functions) — they expand into code that registers your functions
- `module_init(simple_init)` tells the kernel: "when someone runs insmod, call simple_init()"
- `module_exit(simple_exit)` tells the kernel: "when someone runs rmmod, call simple_exit()"
- Without these, the kernel doesn't know what to call — the module would load but do nothing

---

## ✅ Quick Viva Q&A

**Q: What is the difference between a kernel module and a normal program?**
> A normal program runs in user space — it has limited privileges and communicates with the kernel via system calls. A kernel module runs IN the kernel itself, in kernel space, with full hardware and memory access. It can call any kernel function directly. A bug in a kernel module can crash the entire system, whereas a bug in a normal program only crashes that program.

**Q: Why do we use `printk()` instead of `printf()`?**
> `printf()` is a C standard library function that writes to standard output — it only works in user space where there's a terminal to print to. Inside the kernel, there's no terminal or C library. `printk()` writes to the kernel's internal log ring buffer, which can be read with `sudo dmesg`.

**Q: How do you load and unload the module?**
> Load: `sudo insmod simple.ko` (the `.ko` extension is the compiled kernel object). Unload: `sudo rmmod simple` (no extension needed). Check it's loaded: `lsmod | grep simple`. View output: `sudo dmesg | tail -5`.

**Q: What does `__init` mean and why do we use it?**
> `__init` is a macro that marks the function for placement in a special memory section. After the kernel calls `simple_init()` during `insmod`, it frees that memory — the function is never needed again. This is an optimization to save kernel RAM, since init code is one-time use.

**Q: What happens if you forget `MODULE_LICENSE("GPL")`?**
> The module will compile but you'll get kernel warnings: "module is not GPL compatible" and "module tainted kernel." More critically, some kernel functions are only exported to GPL modules — without the license declaration, you can't use them and the module will fail to load or link.

**Q: What is GOLDEN_RATIO_PRIME and why does the kernel have it?**
> It's a large prime number close to 2^64 × (golden ratio − 1). It's used in the kernel's hash table implementations. When you hash a key, multiplying by GOLDEN_RATIO_PRIME distributes the results uniformly across hash buckets, reducing collisions. It's only available in kernel space via `<linux/hash.h>`.

**Q: What is GCD(3700, 24)?**
> The answer is 4. Both 3700 and 24 are divisible by 4 (3700 = 4×925, 24 = 4×6). The Euclidean algorithm: gcd(3700, 24) → gcd(24, 3700 mod 24=4) → gcd(4, 24 mod 4=0) → 4.
