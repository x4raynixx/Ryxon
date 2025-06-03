@echo off
setlocal enabledelayedexpansion

echo [*] Preparing folders...
if not exist tools mkdir tools
if not exist install mkdir install
if not exist temp mkdir temp

set LLVM_MINGW_DIR=tools\llvm-mingw-20250528-ucrt-x86_64
set COMMON_FLAGS=-std=c++17 -Wall -Wextra -O3 -flto -ffast-math -Isrc
set X86_FLAGS=-march=native
set ARM_FLAGS=-mcpu=cortex-a72
set SRC_FILES=src\*.cpp src\libraries\math\*.cpp src\libraries\colors\*.cpp src\libraries\time\*.cpp src\libraries\system\*.cpp

if not exist %LLVM_MINGW_DIR% (
    echo [*] LLVM-Mingw x86_64 not found, downloading...
    powershell -Command ^
        "$r = Invoke-RestMethod 'https://api.github.com/repos/mstorsjo/llvm-mingw/releases/llvm-mingw-20250528-ucrt-x86_64';" ^
        "$u = ($r.assets | Where-Object { $_.name -like '*ucrt-x86_64.zip' })[0].browser_download_url;" ^
        "Invoke-WebRequest $u -OutFile 'tools\llvm-mingw.zip'"

    echo [*] Extracting LLVM-Mingw...
    powershell -Command "Expand-Archive -Path 'tools\llvm-mingw.zip' -DestinationPath 'tools' -Force"
    for /d %%D in (tools\llvm-mingw-*) do (
        ren "%%D" llvm-mingw
    )
    del tools\llvm-mingw.zip
    echo [✓] LLVM-Mingw x86_64 installed
) else (
    echo [✓] LLVM-Mingw x86_64 already present
)

set PATH=%CD%\%LLVM_MINGW_DIR%\bin;%PATH%

if exist rx.ico (
    echo IDI_ICON1 ICON "rx.ico" > temp\icon_x86_64.rc
    windres --target=pe-x86-64 temp\icon_x86_64.rc temp\icon_x86_64.o

    echo IDI_ICON1 ICON "rx.ico" > temp\icon_x86.rc
    windres --target=pe-i386 temp\icon_x86.rc temp\icon_x86.o

    echo [*] Icon resources compiled
) else (
    echo [-] rx.ico not found – skipping icons
)

echo [*] Building native binary...
if exist temp\icon_x86_64.o (
    g++ %COMMON_FLAGS% %X86_FLAGS% %SRC_FILES% temp\icon_x86_64.o -static -static-libgcc -static-libstdc++ -o install\rx.exe
) else (
    g++ %COMMON_FLAGS% %X86_FLAGS% %SRC_FILES% -static -static-libgcc -static-libstdc++ -o install\rx.exe
)
if %ERRORLEVEL%==0 (
    echo [✓] Built: install\rx.exe
) else (
    echo [X] Failed to build native binary
)

echo [*] Building x86_64...
if exist temp\icon_x86_64.o (
    x86_64-w64-mingw32-g++ %COMMON_FLAGS% %X86_FLAGS% %SRC_FILES% temp\icon_x86_64.o -static -static-libgcc -static-libstdc++ -o install\rx_x86_64.exe
) else (
    x86_64-w64-mingw32-g++ %COMMON_FLAGS% %X86_FLAGS% %SRC_FILES% -static -static-libgcc -static-libstdc++ -o install\rx_x86_64.exe
)
if %ERRORLEVEL%==0 (
    echo [✓] Built: install\rx_x86_64.exe
) else (
    echo [X] Failed to build x86_64
)

echo [*] Building x86...
if exist temp\icon_x86.o (
    i686-w64-mingw32-g++ %COMMON_FLAGS% %X86_FLAGS% %SRC_FILES% temp\icon_x86.o -static -static-libgcc -static-libstdc++ -o install\rx_x86.exe
) else (
    i686-w64-mingw32-g++ %COMMON_FLAGS% %X86_FLAGS% %SRC_FILES% -static -static-libgcc -static-libstdc++ -o install\rx_x86.exe
)
if %ERRORLEVEL%==0 (
    echo [✓] Built: install\rx_x86.exe
) else (
    echo [X] Failed to build x86
)

echo [*] Building ARM64...
aarch64-w64-mingw32-g++ %COMMON_FLAGS% %ARM_FLAGS% %SRC_FILES% -static -static-libgcc -static-libstdc++ -o install\rx_arm64.exe
if %ERRORLEVEL%==0 (
    echo [✓] Built: install\rx_arm64.exe
    if exist rx.ico (
        echo [*] Adding icon to ARM64 executable...
        if exist tools\rcedit.exe (
            tools\rcedit.exe install\rx_arm64.exe --set-icon rx.ico
            if %ERRORLEVEL%==0 (
                echo [✓] Icon added to ARM64 executable
            ) else (
                echo [X] Failed to add icon to ARM64 executable
            )
        ) else (
            echo [-] rcedit.exe not found, skipping icon for ARM64
        )
    )
) else (
    echo [X] Failed to build ARM64
)

echo [*] Testing native build...
echo print("Hello from RX!") > test.rx
if exist install\rx.exe (
    install\rx.exe test.rx
) else (
    echo [X] Native binary not found, skipping test
)
del test.rx >nul 2>nul

rmdir /s /q temp >nul 2>nul

echo [✓] Build process completed!
