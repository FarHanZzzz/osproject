#!/bin/bash
# ============================================
# Optional A — Kernel Module
# ============================================
# This script builds, loads, and unloads the kernel module.
# NOTE: You need sudo (root) access to load kernel modules!
# NOTE: You need linux-headers installed for your kernel version.

TASK_DIR="OptionalA_KernelModule"

echo "==============================="
echo " Optional A: Kernel Module"
echo "==============================="
echo ""

echo "Step 1: Building the kernel module..."
make -C "$TASK_DIR"

if [ $? -ne 0 ]; then
    echo ""
    echo "Build FAILED!"
    echo "Make sure you have linux-headers installed:"
    echo "  sudo apt install linux-headers-\$(uname -r)"
    exit 1
fi

echo ""
echo "Build successful!"
echo ""
echo "====================================="
echo " HOW TO USE THE KERNEL MODULE"
echo "====================================="
echo ""
echo "Step 2: Load the module (requires sudo):"
echo "  sudo insmod $TASK_DIR/simple.ko"
echo ""
echo "Step 3: Check the kernel log to see the output:"
echo "  sudo dmesg | tail -5"
echo ""
echo "Step 4: Remove the module:"
echo "  sudo rmmod simple"
echo ""
echo "Step 5: Check the log again to see the exit message:"
echo "  sudo dmesg | tail -5"
echo ""
