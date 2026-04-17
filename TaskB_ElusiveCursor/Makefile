# Task B — Elusive Cursor Makefile
# CSC413/CSE315 — OS Programming Assignment, Spring 2026
#
# ===============================================================
# Option 1: MinGW/MSYS2 (if you have gcc on Windows)
#   $ make
#
# Option 2: MSVC (Visual Studio Build Tools)
#   Open "Developer Command Prompt" or "x64 Native Tools" and run:
#   $ nmake /f Makefile.msvc
#   Or simply:
#   $ cl /W4 elusive_cursor.c user32.lib gdi32.lib /Fe:elusive_cursor.exe
# ===============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -mwindows
TARGET  = elusive_cursor.exe
SRC     = elusive_cursor.c
LIBS    = -lgdi32 -luser32

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	del /f $(TARGET) 2>nul

.PHONY: all clean
