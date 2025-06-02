@echo off
fsutil dirty query %systemdrive% >nul 2>&1
if errorlevel 1 (
    powershell -Command "Start-Process -Verb runAs -FilePath '%~f0'"
    exit /b
)

if not exist "C:\Program Files\RX" mkdir "C:\Program Files\RX"
set RXPATH=C:\Program Files\RX

set ARCH=
for /f "tokens=2 delims==" %%a in ('wmic os get osarchitecture /value ^| find "="') do set ARCH=%%a

set DOWNLOAD_URL=https://github.com/x4raynixx/RX-Scripting/raw/master/install/

if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set FILE=rx_x86_64.exe
) else if "%PROCESSOR_ARCHITECTURE%"=="ARM64" (
    set FILE=rx_arm64.exe
) else (
    set FILE=rx_x86.exe
)

curl -L -o "%RXPATH%\rx.exe" "%DOWNLOAD_URL%%FILE%"

reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_EXPAND_SZ /d "%Path%;%RXPATH%" /f

assoc .rx=RXFile
ftype RXFile="%RXPATH%\rx.exe" "%%1"

echo RX installed successfully
pause
