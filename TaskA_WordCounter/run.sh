#!/bin/bash
# Run script for Task A: Word Counter
# Usage: ./run.sh

echo "Compiling word_counter.c..."
gcc -o word_counter word_counter.c

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

echo "Compilation successful!"
echo ""
./word_counter
