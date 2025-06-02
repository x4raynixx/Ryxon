@echo off
cls
setlocal enabledelayedexpansion

ver | findstr /i "windows" >nul
if errorlevel 1 (
    color 0C
    echo This installer works only on Windows.
    pause
    exit /b
)

set "arch="
if defined PROCESSOR_ARCHITECTURE (
    set "arch=%PROCESSOR_ARCHITECTURE%"
)

if /i "%arch%"=="AMD64" (
    set "file=rx_x86_64.exe"
    set "archPretty=x86_64 (AMD64)"
) else if /i "%arch%"=="ARM64" (
    set "file=rx_arm64.exe"
    set "archPretty=ARM64"
) else if /i "%arch%"=="x86" (
    set "file=rx_x86.exe"
    set "archPretty=x86"
) else (
    color 0C
    echo Unknown architecture: %arch%
    pause
    exit /b
)

for /f "delims=" %%a in ('ver') do set "osVersion=%%a"

echo ===============================
echo Detected architecture: %archPretty%
echo System: Windows
echo OS version: %osVersion%
echo ===============================

net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Running as administrator...
    powershell -Command "Start-Process -FilePath '%~f0' -Verb runAs -Wait"
    exit /b
)

set "installDir=%ProgramFiles%\RX"
if not exist "%installDir%" (
    mkdir "%installDir%"
)

color 0E
echo Downloading %file%...
curl -L -o "%installDir%\rx.exe" "https://github.com/x4raynixx/RX-Scripting/raw/master/install/%file%"
if errorlevel 1 (
    color 0C
    echo Download failed!
    pause
    exit /b
)
echo Download completed.

echo Updating user PATH...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "[Environment]::SetEnvironmentVariable('Path', (Get-ItemProperty -Path 'HKCU:\Environment' -Name Path).Path + ';%installDir%', 'User')" >nul 2>&1
echo Added %installDir% to user PATH.

echo Associating .rx extension and default application...
assoc .rx=RXFile
ftype RXFile="%installDir%\rx.exe" "%%1"
reg add "HKCR\RXFile" /ve /d "RX Script" /f >nul
reg add "HKCR\RXFile\DefaultIcon" /ve /d "%installDir%\rx.exe,0" /f >nul

echo Creating template for New > RX Script...
set "templateDir=%ProgramData%\Microsoft\Windows\Templates"
if not exist "%templateDir%" mkdir "%templateDir%"
type nul > "%templateDir%\RX Script.rx"

echo Adding RX Script to New menu...
reg add "HKCR\.rx" /ve /d "RXFile" /f >nul
reg add "HKCR\.rx\ShellNew" /v FileName /d "RX Script.rx" /f >nul

cls
color 0A
echo ===============================
echo RX was installed successfully.
echo You can now use the 'rx' command or run .rx files directly.
echo New > RX Script is now available.
echo ===============================
color 7
pause
endlocal
exit /b
