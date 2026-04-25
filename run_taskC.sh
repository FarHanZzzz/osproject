#!/bin/bash
# ============================================
# Task C — Parallel Matrix Adder
# ============================================
# This script compiles and runs the Matrix Adder program.
# It uses 4 pthreads to add two 1000x1000 matrices in parallel.

TASK_DIR="TaskC_MatrixAdder"

echo "============================="
echo " Task C: Parallel Matrix Adder"
echo "============================="
echo ""

echo "Compiling matrix_adder.c..."
gcc -o "$TASK_DIR/matrix_add" "$TASK_DIR/matrix_adder.c" -lpthread

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

echo "Compilation successful!"
echo ""
./"$TASK_DIR/matrix_add"
