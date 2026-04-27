#!/bin/bash
# ============================================
# Task A — Word Counter
# ============================================
# This script compiles and runs the word counter.
# It prompts the user for file names (dynamic input).

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

# Show available text files so the user knows what they can pick
echo "Available text files in $TASK_DIR/:"
ls "$TASK_DIR"/*.txt 2>/dev/null | xargs -n1 basename
echo ""

# Prompt for file names
echo "Enter file names to count words (space-separated),"
echo "or press Enter for defaults (sample1.txt sample2.txt):"
read -rp "> " USER_INPUT

cd "$TASK_DIR"

if [ -z "$USER_INPUT" ]; then
    echo ""
    echo "--- Using default files ---"
    ./word_counter
else
    echo ""
    echo "--- Counting words in: $USER_INPUT ---"
    # shellcheck disable=SC2086
    ./word_counter $USER_INPUT
fi

cd ..
