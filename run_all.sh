#!/bin/bash

# ==============================================================================
# Master Run Script for OS Programming Assignment (Spring 2026)
# This script compiles and runs all the Linux-compatible tasks sequentially.
# ==============================================================================

# Stop the script if any command fails
set -e

# Get the absolute path of the directory this script is in
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"

echo "================================================================================"
echo "          STARTING OS PROJECT DEMONSTRATION SCRIPT"
echo "================================================================================"
echo ""

# ------------------------------------------------------------------------------
# TASK A - Word Counter
# ------------------------------------------------------------------------------
echo ">>> [TASK A] Building and Running Word Counter (fork + pipe IPC)"
cd "$PROJECT_DIR/TaskA_WordCounter"
make clean > /dev/null 2>&1
make > /dev/null 2>&1

echo "Command: ./word_counter word_counter.c Makefile"
echo "--------------------------------------------------------"
./word_counter word_counter.c Makefile
echo ""

# ------------------------------------------------------------------------------
# TASK C - Matrix Adder
# ------------------------------------------------------------------------------
echo ">>> [TASK C] Building and Running Parallel Matrix Adder (pthreads)"
cd "$PROJECT_DIR/TaskC_MatrixAdder"
make clean > /dev/null 2>&1
make > /dev/null 2>&1

echo "Command: ./matrix_add 4 (Using 4 threads for 1000x1000 matrix)"
echo "--------------------------------------------------------"
./matrix_add 4
echo ""

# ------------------------------------------------------------------------------
# TASK D - MLQ Scheduler
# ------------------------------------------------------------------------------
echo ">>> [TASK D] Building and Running Time-Sliced MLQ Scheduler"
cd "$PROJECT_DIR/TaskD_MLQScheduler"
make clean > /dev/null 2>&1
make > /dev/null 2>&1

echo "Command: ./mlq_scheduler"
echo "--------------------------------------------------------"
./mlq_scheduler
echo ""

# ------------------------------------------------------------------------------
# TASK E - Real-Time Scheduler
# ------------------------------------------------------------------------------
echo ">>> [TASK E] Building and Running Real-Time Scheduler (RM & EDF)"
cd "$PROJECT_DIR/TaskE_RealTimeScheduler"
make clean > /dev/null 2>&1
make > /dev/null 2>&1

echo "Command: ./rt_scheduler (Piping 'no' to skip interactive prompt)"
echo "--------------------------------------------------------"
echo "no" | ./rt_scheduler
echo ""

# ------------------------------------------------------------------------------
# END
# ------------------------------------------------------------------------------
echo "================================================================================"
echo "          ALL COMPATIBLE LINUX TASKS COMPLETED SUCCESSFULLY!"
echo "================================================================================"
echo "Note: Task B (Elusive Cursor) and Optional Tasks were skipped because they"
echo "require a Windows environment or root/sudo privileges (Kernel Module)."
echo "================================================================================"
