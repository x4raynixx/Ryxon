@echo off
setlocal enabledelayedexpansion

>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system" || (
    echo Run as Administrator!
    pause
    exit /b 1
)

set "ARCH=x86"
if /i "%PROCESSOR_ARCHITECTURE%"=="AMD64" set ARCH=x86_64
if /i "%PROCESSOR_ARCHITECTURE%"=="ARM64" set ARCH=arm64

set "OS=windows"

set "URL_BASE=https://raw.githubusercontent.com/x4raynixx/RX-Scripting/master/install"

set "FILE="
if "%OS%"=="windows" (
    set "FILE=rx_%ARCH%.exe"
) else (
    exit /b 1
)

set "INSTALL_DIR=%ProgramFiles%\RX"

if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

set "DEST=%INSTALL_DIR%\rx.exe"

powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%URL_BASE%/%FILE%', '%DEST%')"

if not exist "%DEST%" (
    echo Download failed.
    pause
    exit /b 1
)

for /f "tokens=2*" %%A in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path ^| findstr Path') do (
    set "SYS_PATH=%%B"
)

echo %SYS_PATH% | find /i "%INSTALL_DIR%" >nul
if errorlevel 1 (
    setx /M PATH "%SYS_PATH%;%INSTALL_DIR%"
    if errorlevel 1 (
        echo Failed to update PATH.
        pause
        exit /b 1
    )
)

echo RX installed to %DEST%
echo Restart terminal or log off/on to update PATH.
pause
exit /b 0
