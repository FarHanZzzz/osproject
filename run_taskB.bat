@echo off
setlocal
cd /d "%~dp0TaskB_ElusiveCursor"

echo ================================================================================
echo >>> [TASK B] Building and Running The Elusive Cursor (Win32 API)
echo ================================================================================
echo.

REM Set up MSVC environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
if errorlevel 1 (
    echo ERROR: Visual Studio Build Tools not found.
    echo Please install VS2019 or VS2022 Build Tools.
    exit /b 1
)

REM Clean previous build
del /f elusive_cursor.exe elusive_cursor.obj 2>nul

REM Compile
echo [1/3] Compiling elusive_cursor.c with MSVC...
cl /nologo /W4 elusive_cursor.c user32.lib gdi32.lib /Fe:elusive_cursor.exe >nul 2>&1
if errorlevel 1 (
    echo ERROR: Compilation failed!
    exit /b 1
)
echo       Compiled successfully: elusive_cursor.exe
echo.

REM Run
echo [2/3] Launching elusive_cursor.exe...
echo       The cursor will start jittering in +-25 pixels every 400ms.
echo.
echo       *** Press Ctrl+Shift+Q to stop the program. ***
echo.
start "" elusive_cursor.exe

REM Wait and confirm
timeout /t 2 >nul
tasklist /fi "imagename eq elusive_cursor.exe" /nh 2>nul | find /i "elusive_cursor" >nul
if errorlevel 1 (
    echo ERROR: Process failed to start!
    exit /b 1
)
echo       Process is running. Your cursor should be jittering now.
echo.

echo [3/3] Waiting for you to press Ctrl+Shift+Q to stop the program...
echo       (Or close this window and stop it from Task Manager)
echo.

:waitloop
tasklist /fi "imagename eq elusive_cursor.exe" /nh 2>nul | find /i "elusive_cursor" >nul
if not errorlevel 1 (
    timeout /t 1 >nul
    goto waitloop
)

echo       Program exited cleanly.
echo ================================================================================
echo >>> [TASK B] Test completed successfully!
echo ================================================================================
endlocal
