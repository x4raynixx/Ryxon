@echo off
cls
echo ===============================
echo     RX INSTALLER
echo ===============================

setlocal

rem Detect OS
ver | findstr /i "windows" >nul
if errorlevel 1 (
    color 0C
    echo This installer works only on Windows.
    pause
    exit /b
)

rem Detect architecture
set "arch="
if defined PROCESSOR_ARCHITECTURE (
    set "arch=%PROCESSOR_ARCHITECTURE%"
)

if "%arch%"=="AMD64" (
    set "file=rx_x86_64.exe"
    set "archPretty=x86_64 (AMD64)"
) else if "%arch%"=="ARM64" (
    set "file=rx_arm64.exe"
    set "archPretty=ARM64"
) else if "%arch%"=="x86" (
    set "file=rx_x86.exe"
    set "archPretty=x86"
) else (
    color 0C
    echo Unsupported architecture: %arch%
    pause
    exit /b
)

rem Detect OS version (basic)
for /f "tokens=4-5 delims=[.] " %%a in ('ver') do (
    set "osVersion=%%a.%%b"
)

echo ===============================
echo Detected Architecture: %archPretty%
echo System: Windows
echo OS Version: %osVersion%
echo ===============================

net session >nul 2>&1
if %errorLevel% neq 0 (
    powershell -Command "Start-Process -FilePath '%~f0' -Verb runAs -Wait"
    exit /b
)

set "installDir=%ProgramFiles%\RX"
if not exist "%installDir%" mkdir "%installDir%"

echo Downloading RX for architecture: %archPretty%
color 0E
curl -L -o "%installDir%\rx.exe" "https://github.com/x4raynixx/RX-Scripting/raw/master/install/%file%"

echo Updating system PATH...
color 0A
setx PATH "%PATH%;%installDir%" >nul

assoc .rx=RXFile
ftype RXFile="%installDir%\rx.exe" "%%1"

cls
color 0A
echo ===============================
echo RX installed successfully.
echo ===============================
pause
endlocal
