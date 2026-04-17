# ============================================================
# Makefile — Task C: Multi-threaded Matrix Adder
# CSC413/CSE315 — OS Programming Assignment, Spring 2026
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2 -D_POSIX_C_SOURCE=199309L
TARGET  = matrix_add
SRC     = matrix_adder.c
LIBS    = -lpthread

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Performance comparison: 1 thread vs 4 threads
perf: $(TARGET)
	@echo "=== Performance Comparison ==="
	@echo "--- 1 thread ---"
	time ./$(TARGET) 1
	@echo ""
	@echo "--- 2 threads ---"
	time ./$(TARGET) 2
	@echo ""
	@echo "--- 4 threads ---"
	time ./$(TARGET) 4
	@echo ""
	@echo "--- 8 threads ---"
	time ./$(TARGET) 8

# Quick correctness test
test: $(TARGET)
	@echo "--- Test: 1 thread ---"
	./$(TARGET) 1
	@echo ""
	@echo "--- Test: 4 threads (with remainder: 1000 rows / 3 = 333 r2) ---"
	./$(TARGET) 3
	@echo ""
	@echo "--- Test: more threads than rows edge case ---"
	./$(TARGET) 1001

clean:
	rm -f $(TARGET)

.PHONY: all perf test clean
