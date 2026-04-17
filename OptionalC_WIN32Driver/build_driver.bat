@echo off
REM ============================================================
REM  Optional C — WIN32 Kernel-Mode Driver Build Script
REM  CSC413/CSE315 — OS Programming Assignment, Spring 2026
REM
REM  Prerequisites:
REM    - Visual Studio 2019/2022 Build Tools
REM    - Windows Driver Kit (WDK) 10.0.19041
REM
REM  Usage:
REM    Open a regular Command Prompt and run:
REM      build_driver.bat
REM ============================================================

echo === Optional C — Building Kernel-Mode Driver ===
echo.

REM Set up the MSVC x64 environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
if errorlevel 1 (
    echo ERROR: Could not find Visual Studio Build Tools.
    echo Install VS2019 or VS2022 Build Tools first.
    exit /b 1
)

set WDK_INC=C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0
set WDK_LIB=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0

REM Step 1: Compile driver.c to driver.obj
echo [1/2] Compiling driver.c ...
cl /nologo /W4 /WX- /kernel /GS- /Gz /Oi ^
   /D _WIN64 /D _AMD64_ /D AMD64 ^
   /D NTDDI_VERSION=0x0A000007 ^
   /I "%WDK_INC%\km" ^
   /I "%WDK_INC%\shared" ^
   /I "%WDK_INC%\um" ^
   /c driver.c /Fo:driver.obj
if errorlevel 1 (
    echo ERROR: Compilation failed!
    exit /b 1
)
echo    Compilation successful.
echo.

REM Step 2: Link driver.obj to driver.sys
echo [2/2] Linking driver.sys ...
link /nologo /DRIVER /SUBSYSTEM:NATIVE /ENTRY:DriverEntry ^
     /OUT:driver.sys ^
     /LIBPATH:"%WDK_LIB%\km\x64" ^
     /LIBPATH:"%WDK_LIB%\um\x64" ^
     ntoskrnl.lib hal.lib wmilib.lib ^
     driver.obj
if errorlevel 1 (
    echo ERROR: Linking failed!
    exit /b 1
)
echo    Linking successful.
echo.

echo === Build Complete! ===
echo Output: driver.sys (%~dp0driver.sys)
echo.
echo === Next Steps ===
echo 1. Disable Driver Signature Enforcement:
echo    Shift+Click Restart ^> Troubleshoot ^> Advanced Options ^>
echo    Startup Settings ^> Restart ^> Press F7
echo.
echo 2. Open DebugView as Administrator:
echo    Enable Capture ^> Capture Kernel
echo.
echo 3. Open an Admin Command Prompt and run:
echo    sc create myDriver binPath= "%~dp0driver.sys" type= kernel
echo    sc start myDriver
echo    sc stop myDriver
echo    sc delete myDriver
echo.
