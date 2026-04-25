#!/bin/bash
# ============================================
# Task D — MLQ Scheduler
# ============================================
# This script compiles and runs the Multilevel Queue Scheduler.
# It simulates Q1 (Round Robin), Q2 (SJF), Q3 (FCFS).

TASK_DIR="TaskD_MLQScheduler"

echo "============================="
echo " Task D: MLQ Scheduler"
echo "============================="
echo ""

echo "Compiling mlq_scheduler.cpp..."
g++ -o "$TASK_DIR/mlq_scheduler" "$TASK_DIR/mlq_scheduler.cpp"

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

echo "Compilation successful!"
echo ""
./"$TASK_DIR/mlq_scheduler"
