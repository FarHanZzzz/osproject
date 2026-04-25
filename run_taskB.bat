@echo off
setlocal
cd /d "%~dp0TaskB_ElusiveCursor"

echo ================================================================================
echo    [TASK B] Building and Running The Elusive Cursor (Win32 API)
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
echo [1/2] Compiling elusive_cursor.c with MSVC...
cl /nologo /W4 elusive_cursor.c user32.lib gdi32.lib /Fe:elusive_cursor.exe
if errorlevel 1 (
    echo ERROR: Compilation failed!
    exit /b 1
)
echo.
echo       Compiled successfully: elusive_cursor.exe
echo.

REM Launch
echo [2/2] Launching elusive_cursor.exe...
echo.
echo       The cursor will start jittering +-25 pixels every 400ms.
echo       Press Ctrl+Shift+Q to stop it cleanly.
echo.
start "" "%~dp0TaskB_ElusiveCursor\elusive_cursor.exe"
echo       Process launched! Check your cursor.
echo.
echo ================================================================================
echo    [TASK B] Done! Press Ctrl+Shift+Q to kill the cursor prank.
echo ================================================================================
endlocal
