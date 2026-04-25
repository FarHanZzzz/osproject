#!/bin/bash
# Run script for Task D: MLQ Scheduler
# Usage: ./run.sh

echo "Compiling mlq_scheduler.cpp..."
g++ -o mlq_scheduler mlq_scheduler.cpp

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

echo "Compilation successful!"
echo ""
./mlq_scheduler
