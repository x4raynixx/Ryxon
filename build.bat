@echo off
setlocal enabledelayedexpansion

echo Building...

if not exist temp mkdir temp
if not exist bin mkdir bin

set PLATFORM=windows

set COMMON_FLAGS=-std=c++17 -Wall -Wextra -O2 -Isrc

echo Building for Windows...
g++ %COMMON_FLAGS% src\*.cpp -o bin\rx.exe
if %ERRORLEVEL% NEQ 0 (
    echo Build failed...
    exit /b %ERRORLEVEL%
)
echo Built: bin\rx.exe

if "%1"=="all" (
    echo Building for all platforms...

    where x86_64-w64-mingw32-g++ >nul 2>nul
    if %ERRORLEVEL%==0 (
        echo Building for Windows...
        x86_64-w64-mingw32-g++ %COMMON_FLAGS% src\*.cpp -o bin\rx_cross.exe
        if %ERRORLEVEL%==0 (
            echo ✓ Windows cross-build complete
        )
    )
)

echo Testing the build...
echo print("Hello from RX!") > test.rx

if exist bin\rx.exe (
    echo Running test...
    bin\rx.exe test.rx
)

del test.rx
rmdir /s /q temp
echo Build finished successfully
