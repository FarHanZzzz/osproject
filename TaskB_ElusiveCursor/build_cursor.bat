@echo off
REM ============================================================
REM  Task B — Elusive Cursor Build Script (MSVC)
REM  CSC413/CSE315 — OS Programming Assignment, Spring 2026
REM
REM  Usage:
REM    Open a regular Command Prompt and run:
REM      build_cursor.bat
REM ============================================================

echo === Task B — Building Elusive Cursor ===
echo.

REM Set up the MSVC x64 environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
if errorlevel 1 (
    echo ERROR: Could not find Visual Studio Build Tools.
    exit /b 1
)

echo Compiling elusive_cursor.c ...
cl /nologo /W4 elusive_cursor.c user32.lib gdi32.lib /Fe:elusive_cursor.exe
if errorlevel 1 (
    echo ERROR: Compilation failed!
    exit /b 1
)

echo.
echo === Build Complete! ===
echo Output: elusive_cursor.exe
echo.
echo === How to Run ===
echo 1. Double-click elusive_cursor.exe (or run from terminal)
echo 2. The cursor will start jumping every 400ms
echo 3. Press Ctrl+Shift+Q to stop it cleanly
echo.
