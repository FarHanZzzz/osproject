@echo off
setlocal

REM Root runner for Task B: Elusive Cursor (Windows only)
set "SCRIPT_DIR=%~dp0"
set "EXE=%SCRIPT_DIR%TaskB_ElusiveCursor\elusive_cursor.exe"
set "SRC=%SCRIPT_DIR%TaskB_ElusiveCursor\elusive_cursor.c"

echo =============================
echo  Task B: Elusive Cursor
echo =============================
echo.

if not exist "%EXE%" (
    echo Executable not found: %EXE%
    echo Building now...

    where gcc >nul 2>nul
    if errorlevel 1 (
        echo ERROR: gcc not found in PATH.
        echo Install MinGW-w64 and make sure gcc is available, then run again.
        exit /b 1
    )

    gcc -o "%EXE%" "%SRC%" -lgdi32 -luser32 -mwindows
    if errorlevel 1 (
        echo Build failed.
        exit /b 1
    )

    echo Build successful.
    echo.
)

echo Starting Elusive Cursor... Press Ctrl+Shift+Q to stop.
"%EXE%"