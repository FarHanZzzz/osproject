#!/bin/bash
set -e
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR/TaskC_MatrixAdder"

echo "================================================================================"
echo ">>> [TASK C] Building and Running Parallel Matrix Adder (pthreads)"
echo "================================================================================"

make clean > /dev/null 2>&1
make > /dev/null 2>&1

echo "Command: ./matrix_add 4 (Using 4 threads for 1000x1000 matrix)"
echo "--------------------------------------------------------"
./matrix_add 4
echo ""
