@echo off
setlocal enabledelayedexpansion

echo Building...

if not exist temp mkdir temp
if not exist install mkdir install

set PLATFORM=windows

set COMMON_FLAGS=-std=c++17 -Wall -Wextra -O2 -Isrc

echo Generating icon resource...
if exist rx.ico (
    echo IDI_ICON1 ICON "rx.ico" > temp\icon.rc
    windres -i temp\icon.rc -o temp\icon.o
) else (
    echo No icon.ico found, skipping icon...
)

echo Building for Windows...
if exist temp\icon.o (
    g++ %COMMON_FLAGS% src\*.cpp temp\icon.o -o install\rx.exe
) else (
    g++ %COMMON_FLAGS% src\*.cpp -o install\rx.exe
)
if %ERRORLEVEL% NEQ 0 (
    echo Build failed...
    exit /b %ERRORLEVEL%
)
echo Built: install\rx.exe

if "%1"=="all" (
    echo Building for all platforms...

    where x86_64-w64-mingw32-g++ >nul 2>nul
    if %ERRORLEVEL%==0 (
        echo Building for Windows...
        if exist temp\icon.o (
            x86_64-w64-mingw32-g++ %COMMON_FLAGS% src\*.cpp temp\icon.o -o install\rx_cross.exe
        ) else (
            x86_64-w64-mingw32-g++ %COMMON_FLAGS% src\*.cpp -o install\rx_cross.exe
        )
        if %ERRORLEVEL%==0 (
            echo ✓ Windows cross-build complete
        )
    )
)

echo Testing the build...
echo print("Hello from RX!") > test.rx

if exist install\rx.exe (
    echo Running test...
    install\rx.exe test.rx
)

del test.rx
rmdir /s /q temp
echo Build finished successfully
