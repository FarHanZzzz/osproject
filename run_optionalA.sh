#!/bin/bash
set -e
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR/OptionalA_KernelModule"

echo "================================================================================"
echo ">>> [OPTIONAL A] Building and Running Linux Kernel Module"
echo "================================================================================"

# Compile the module
make clean > /dev/null 2>&1 || true
make > /dev/null 2>&1

echo "[1/4] Module successfully compiled (simple.ko created)."
echo ""
echo "NOTE: The next steps require Administrator privileges."
echo "Please type your computer's password if prompted."
echo ""

# Load the module
echo "[2/4] Loading module into the Kernel (sudo insmod simple.ko)..."
sudo insmod simple.ko

echo ""
echo "--- KERNEL RING BUFFER OUTPUT (dmesg) ---"
sudo dmesg | tail -5
echo "-----------------------------------------"
echo ""

# Remove the module
echo "[3/4] Removing module from the Kernel (sudo rmmod simple)..."
sudo rmmod simple

echo ""
echo "--- KERNEL RING BUFFER OUTPUT (dmesg) ---"
sudo dmesg | tail -5
echo "-----------------------------------------"
echo ""

echo "[4/4] Done! Optional A test completed successfully."
echo "================================================================================"
