# ============================================================
# Makefile — Task E: Real-Time Scheduler
# CSC413/CSE315 — OS Programming Assignment, Spring 2026
# ============================================================

CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
TARGET   = rt_scheduler
SRC      = rt_scheduler.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Run with "no" to skip interactive prompt
run: $(TARGET)
	echo "no" | ./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
