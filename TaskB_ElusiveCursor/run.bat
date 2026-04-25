@echo off
REM Run script for Task B: Elusive Cursor (Windows only)
REM Usage: run.bat

echo Compiling elusive_cursor.c...
gcc -o elusive_cursor.exe elusive_cursor.c -lgdi32 -luser32 -mwindows

if %errorlevel% neq 0 (
    echo Compilation FAILED!
    exit /b 1
)

echo Compilation successful!
echo.
echo Starting Elusive Cursor... Press Ctrl+Shift+Q to stop.
elusive_cursor.exe
