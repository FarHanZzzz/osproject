# ============================================================
# Makefile — Task D: Time-Sliced MLQ Scheduler
# CSC413/CSE315 — OS Programming Assignment, Spring 2026
# ============================================================

CXX     = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
TARGET  = mlq_scheduler
SRC     = mlq_scheduler.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
