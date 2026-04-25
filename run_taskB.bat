@echo off
setlocal

REM Root runner for Task B: Elusive Cursor (Windows only)
set "SCRIPT_DIR=%~dp0"
set "EXE=%SCRIPT_DIR%TaskB_ElusiveCursor\elusive_cursor.exe"

echo =============================
echo  Task B: Elusive Cursor
echo =============================
echo.

if not exist "%EXE%" (
    echo Executable not found: %EXE%
    echo Build it first, then run this script again.
    exit /b 1
)

echo Starting Elusive Cursor... Press Ctrl+Shift+Q to stop.
"%EXE%"