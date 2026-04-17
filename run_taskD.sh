#!/bin/bash
set -e
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR/TaskD_MLQScheduler"

echo "================================================================================"
echo ">>> [TASK D] Building and Running Time-Sliced MLQ Scheduler"
echo "================================================================================"

make clean > /dev/null 2>&1
make > /dev/null 2>&1

echo "Command: ./mlq_scheduler"
echo "--------------------------------------------------------"
./mlq_scheduler
echo ""
