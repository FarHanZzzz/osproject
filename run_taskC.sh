#!/bin/bash
# ============================================
# Task C — Parallel Matrix Adder
# ============================================
# This script compiles and runs the matrix adder.
# It demonstrates dynamic threading and performance comparison.

TASK_DIR="TaskC_MatrixAdder"

echo "==============================="
echo " Task C: Parallel Matrix Adder"
echo "==============================="
echo ""

echo "Compiling..."
g++ -x c++ -std=c++11 -o "$TASK_DIR/matrix_add" "$TASK_DIR/matrix_adder.c" -pthread

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

echo "Build successful!"
echo ""

echo "--- Run with 4 threads (default) ---"
./"$TASK_DIR"/matrix_add
echo ""

echo "--- Run with 3 threads (remainder handling demo) ---"
./"$TASK_DIR"/matrix_add 3
