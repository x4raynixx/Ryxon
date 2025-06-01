#!/bin/bash

set -e

echo "Building..."
mkdir -p temp bin

case "$(uname -s)" in
    Linux*)     PLATFORM=linux;;
    Darwin*)    PLATFORM=macos;;
    *)          PLATFORM=unknown;;
esac

echo "Detected platform: $PLATFORM"

COMMON_FLAGS="-std=c++17 -Wall -Wextra -O2 -I./src"

case $PLATFORM in
    linux)
        echo "Building for Linux..."
        g++ $COMMON_FLAGS src/*.cpp -o bin/rx.lnx
        echo "Built: bin/rx.lnx"
        ;;
    macos)
        echo "Building for macOS..."
        clang++ $COMMON_FLAGS src/*.cpp -o bin/rx.mac
        echo "Built: bin/rx.mac"
        ;;
    *)
        echo "Unknown platform. Attempting generic build..."
        g++ $COMMON_FLAGS src/*.cpp -o bin/rx
        echo "Built: bin/rx"
        ;;
esac

if [ "$1" = "all" ]; then
    echo "Building for all platforms..."
    
    if command -v g++ &> /dev/null; then
        echo "Building for Linux..."
        g++ $COMMON_FLAGS src/*.cpp -o bin/rx_linux
        echo "✓ Linux build complete"
    fi
    
    if command -v clang++ &> /dev/null; then
        echo "Building for macOS..."
        clang++ $COMMON_FLAGS src/*.cpp -o bin/rx_macos
        echo "✓ macOS build complete"
    fi
    
    echo "Cross-platform build complete!"
fi

echo "Testing the build..."
echo 'print("Hello from RX!")' > temp/test.rx

case $PLATFORM in
    linux)
        if [ -f "bin/rx_linux" ]; then
            echo "Running test..."
            ./bin/rx_linux temp/test.rx
        fi
        ;;
    macos)
        if [ -f "bin/rx_macos" ]; then
            echo "Running test..."
            ./bin/rx_macos temp/test.rx
        fi
        ;;
    *)
        if [ -f "bin/rx" ]; then
            echo "Running test..."
            ./bin/rx temp/test.rx
        fi
        ;;
esac

rm -rf temp
echo "Build finished successfully"
