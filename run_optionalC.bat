@echo off
setlocal
cd /d "%~dp0OptionalC_WIN32Driver"

echo ================================================================================
echo >>> [OPTIONAL C] Building WIN32 Kernel-Mode Driver
echo ================================================================================
echo.

REM Set up MSVC environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
if errorlevel 1 (
    echo ERROR: Visual Studio Build Tools not found.
    echo Please install VS2019 or VS2022 Build Tools.
    exit /b 1
)

set WDK_INC=C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0
set WDK_LIB=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0

REM Verify WDK is installed
if not exist "%WDK_INC%\km\ntddk.h" (
    echo ERROR: Windows Driver Kit (WDK) not found!
    echo Expected: %WDK_INC%\km\ntddk.h
    echo.
    echo Install WDK with:
    echo   winget install Microsoft.WindowsWDK.10.0.19041
    exit /b 1
)

REM Clean previous build
del /f driver.obj driver.sys 2>nul

REM Step 1: Compile
echo [1/2] Compiling driver.c (kernel mode)...
cl /nologo /W4 /WX- /kernel /GS- /Gz /Oi ^
   /D _WIN64 /D _AMD64_ /D AMD64 ^
   /D NTDDI_VERSION=0x0A000007 ^
   /I "%WDK_INC%\km" ^
   /I "%WDK_INC%\shared" ^
   /I "%WDK_INC%\um" ^
   /c driver.c /Fo:driver.obj >nul 2>&1
if errorlevel 1 (
    echo ERROR: Compilation failed!
    exit /b 1
)
echo       Compiled successfully: driver.obj
echo.

REM Step 2: Link
echo [2/2] Linking driver.sys (NATIVE subsystem)...
link /nologo /DRIVER /SUBSYSTEM:NATIVE /ENTRY:DriverEntry ^
     /OUT:driver.sys ^
     /LIBPATH:"%WDK_LIB%\km\x64" ^
     /LIBPATH:"%WDK_LIB%\um\x64" ^
     ntoskrnl.lib hal.lib wmilib.lib ^
     driver.obj >nul 2>&1
if errorlevel 1 (
    echo ERROR: Linking failed!
    exit /b 1
)
echo       Linked successfully: driver.sys
echo.

echo ================================================================================
echo >>> [OPTIONAL C] Build completed successfully!
echo ================================================================================
echo.
echo   Output: %cd%\driver.sys
echo.
echo ================================================================================
echo >>> HOW TO LOAD AND TEST THE DRIVER
echo ================================================================================
echo.
echo   STEP 1: Disable Driver Signature Enforcement
echo           Shift + Click "Restart" in the Start Menu
echo           Troubleshoot ^> Advanced Options ^> Startup Settings ^> Restart
echo           Press F7: "Disable driver signature enforcement"
echo.
echo   STEP 2: Open DebugView (Sysinternals) as Administrator
echo           Download: https://learn.microsoft.com/en-us/sysinternals/downloads/debugview
echo           Enable: Capture ^> Capture Kernel
echo           Enable: Capture ^> Enable Verbose Kernel Output
echo.
echo   STEP 3: Open an Administrator Command Prompt and run:
echo.
echo           sc create myDriver binPath= "%cd%\driver.sys" type= kernel
echo           sc start myDriver
echo.
echo           (Check DebugView for "Hello Kernel! Farhan Sadeque")
echo           (Check C:\kernel_test.txt for kernel-mode file output)
echo.
echo           sc stop myDriver
echo.
echo           (Check DebugView for "Goodbye Kernel!")
echo.
echo           sc delete myDriver
echo.
echo   WARNING: Test in a Virtual Machine! A kernel bug = Blue Screen of Death.
echo ================================================================================
endlocal
