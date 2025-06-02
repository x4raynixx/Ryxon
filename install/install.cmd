@echo off
net session >nul 2>&1
if %errorLevel% neq 0 (
    powershell -Command "Start-Process -FilePath '%~f0' -Verb runAs -Wait"
    exit /b
)

set "arch="
if defined PROCESSOR_ARCHITECTURE (
    set "arch=%PROCESSOR_ARCHITECTURE%"
)

if "%arch%"=="AMD64" (
    set "file=rx_x86_64.exe"
) else if "%arch%"=="ARM64" (
    set "file=rx_arm64.exe"
) else if "%arch%"=="x86" (
    set "file=rx_x86.exe"
) else (
    color 0C
    echo Unsupported architecture: %arch%
    pause
    exit /b
)

set "installDir=%ProgramFiles%\RX"
if not exist "%installDir%" mkdir "%installDir%"

echo Downloading RX for architecture: %arch%
color 0E
curl -L -o "%installDir%\rx.exe" "https://github.com/x4raynixx/RX-Scripting/raw/master/install/%file%"

echo Updating system PATH...
color 0A
setx PATH "%PATH%;%installDir%" >nul

assoc .rx=RXFile
ftype RXFile="%installDir%\rx.exe" "%%1"

cls
color 0A
cls
echo RX installed successfully.
pause
