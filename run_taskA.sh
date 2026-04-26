#!/bin/bash
# ============================================
# Task A — Word Counter
# ============================================
# This script compiles and runs the word counter.
# It demonstrates dynamic file input and error reporting.

TASK_DIR="TaskA_WordCounter"

echo "==============================="
echo " Task A: Word Counter"
echo "==============================="
echo ""

echo "Compiling..."
gcc -o "$TASK_DIR/word_counter" "$TASK_DIR/word_counter.c"

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

echo "Build successful!"
echo ""

echo "--- Test 1: Default files (no arguments) ---"
cd "$TASK_DIR"
./word_counter
echo ""

echo "--- Test 2: Custom files with error reporting ---"
./word_counter sample1.txt sample2.txt nonexistent.txt
cd ..
