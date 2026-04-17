# ============================================================
# Makefile — Task A: Word Counter
# CSC413/CSE315 — OS Programming Assignment, Spring 2026
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
TARGET  = word_counter
SRC     = word_counter.c

# Default target: build the executable
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Run a quick self-test using this Makefile as the "file" input
test: $(TARGET)
	@echo "--- Test 1: Count words in Makefile (single file) ---"
	./$(TARGET) Makefile
	@echo ""
	@echo "--- Test 2: Count words in multiple files ---"
	./$(TARGET) Makefile $(SRC)
	@echo ""
	@echo "--- Test 3: Include a non-existent file (expect error) ---"
	./$(TARGET) Makefile ghost_file.txt $(SRC)

# Remove build artifacts
clean:
	rm -f $(TARGET)

.PHONY: all test clean
