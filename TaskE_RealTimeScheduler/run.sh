#!/bin/bash
# Run script for Task E: Real-Time Scheduler
# Usage: ./run.sh

echo "Compiling rt_scheduler.cpp..."
g++ -o rt_scheduler rt_scheduler.cpp

if [ $? -ne 0 ]; then
    echo "Compilation FAILED!"
    exit 1
fi

echo "Compilation successful!"
echo ""
./rt_scheduler
