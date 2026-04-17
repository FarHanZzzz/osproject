#!/bin/bash
set -e
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR/TaskA_WordCounter"

echo "================================================================================"
echo ">>> [TASK A] Building and Running Word Counter (fork + pipe IPC)"
echo "================================================================================"

make clean > /dev/null 2>&1
make > /dev/null 2>&1

echo "Command: ./word_counter word_counter.c Makefile"
echo "--------------------------------------------------------"
./word_counter word_counter.c Makefile
echo ""
