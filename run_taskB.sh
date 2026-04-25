#!/bin/bash
# ============================================
# Task B — Elusive Cursor
# ============================================
# NOTE: This task is Windows-only! It uses Win32 API (windows.h).
# It CANNOT be compiled or run on Linux.
#
# To run this on Windows, open Command Prompt and type:
#   gcc -o TaskB_ElusiveCursor\elusive_cursor.exe TaskB_ElusiveCursor\elusive_cursor.c -lgdi32 -luser32 -mwindows
#   TaskB_ElusiveCursor\elusive_cursor.exe
#
# Press Ctrl+Shift+Q to stop the cursor from jumping.

echo "============================="
echo " Task B: Elusive Cursor"
echo "============================="
echo ""
echo "WARNING: This task uses Windows API (windows.h)."
echo "It can ONLY be compiled and run on Windows."
echo ""
echo "To compile on Windows (using MinGW GCC):"
echo "  gcc -o elusive_cursor.exe TaskB_ElusiveCursor/elusive_cursor.c -lgdi32 -luser32 -mwindows"
echo ""
echo "To run on Windows:"
echo "  elusive_cursor.exe"
echo ""
echo "Press Ctrl+Shift+Q to stop the program."
