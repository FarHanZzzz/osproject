#!/bin/bash
# Run script for Task C: Parallel Matrix Adder
# Usage: ./run.sh

echo "Compiling matrix_adder.c..."
gcc -o matrix_add matrix_adder.c -lpthread

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

echo "Compilation successful!"
echo ""
./matrix_add
