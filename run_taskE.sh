#!/bin/bash
set -e
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR/TaskE_RealTimeScheduler"

echo "================================================================================"
echo ">>> [TASK E] Building and Running Real-Time Scheduler (RM & EDF)"
echo "================================================================================"

make clean > /dev/null 2>&1
make > /dev/null 2>&1

echo "Command: ./rt_scheduler (Piping 'no' to skip interactive prompt)"
echo "--------------------------------------------------------"
echo "no" | ./rt_scheduler
echo ""
