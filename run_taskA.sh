#!/bin/bash
# ============================================
# Task A — Word Counter
# ============================================
# This script compiles and runs the Word Counter program.
# It uses fork() and pipe() to count words in sample1.txt and sample2.txt.

TASK_DIR="TaskA_WordCounter"

echo "============================="
echo " Task A: Word Counter"
echo "============================="
echo ""

echo "Compiling word_counter.c..."
gcc -o "$TASK_DIR/word_counter" "$TASK_DIR/word_counter.c"

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

echo "Compilation successful!"
echo ""

# Run from inside the task directory so it can find sample1.txt and sample2.txt
cd "$TASK_DIR"
./word_counter
