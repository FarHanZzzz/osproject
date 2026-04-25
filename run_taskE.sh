#!/bin/bash
# ============================================
# Task E — Real-Time Scheduler (RM vs EDF)
# ============================================
# This script compiles and runs the Real-Time Scheduler.
# It simulates Rate Monotonic and Earliest Deadline First.

TASK_DIR="TaskE_RealTimeScheduler"

echo "============================="
echo " Task E: Real-Time Scheduler"
echo "============================="
echo ""

echo "Compiling rt_scheduler.cpp..."
g++ -o "$TASK_DIR/rt_scheduler" "$TASK_DIR/rt_scheduler.cpp"

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

echo "Compilation successful!"
echo ""
./"$TASK_DIR/rt_scheduler"
