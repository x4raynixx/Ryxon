#!/bin/bash

set -e

echo "Select build target:"
echo "1) Linux x86_64 (gcc)"
echo "2) Linux ARM (aarch64, gcc)"
echo "3) macOS x86_64 (clang)"
echo "4) macOS ARM (aarch64, clang)"
echo "5) Build for all platforms"
read -rp "Enter number (1-5): " choice

COMMON_FLAGS="-std=c++17 -Wall -Wextra -O2 -I./src"

install_if_missing() {
    cmd=$1
    pkg=$2
    if ! command -v "$cmd" &> /dev/null; then
        echo "Installing missing package: $pkg"
        if [[ "$OSTYPE" == "linux-gnu"* ]]; then
            sudo apt update
            sudo apt install -y "$pkg"
        else
            echo "Manual install required for $pkg"
        fi
    fi
}

build_linux_x86_64() {
    install_if_missing g++ g++
    echo "Building Linux x86_64..."
    g++ $COMMON_FLAGS src/*.cpp -o install/rx_linux_x86_64
    echo "Built: install/rx_linux_x86_64"
}

build_linux_arm() {
    install_if_missing aarch64-linux-gnu-g++ g++-aarch64-linux-gnu
    echo "Building Linux ARM (aarch64)..."
    aarch64-linux-gnu-g++ $COMMON_FLAGS src/*.cpp -o install/rx_linux_arm64
    echo "Built: install/rx_linux_arm64"
}

build_macos_x86_64() {
    if [[ "$(uname -s)" != "Darwin" ]]; then
        echo "Cannot build macOS x86_64: not running on macOS."
        return
    fi
    install_if_missing clang++ clang
    echo "Building macOS x86_64..."
    clang++ $COMMON_FLAGS src/*.cpp -o install/rx_macos_x86_64
    echo "Built: install/rx_macos_x86_64"
}

build_macos_arm() {
    if [[ "$(uname -s)" != "Darwin" ]]; then
        echo "Cannot build macOS ARM: not running on macOS."
        return
    fi
    install_if_missing clang++ clang
    echo "Building macOS ARM (aarch64)..."
    clang++ $COMMON_FLAGS -target arm64-apple-macos11 -stdlib=libc++ -isysroot $(xcrun --sdk macosx --show-sdk-path) src/*.cpp -o install/rx_macos_arm64
    echo "Built: install/rx_macos_arm64"
}

run_test() {
    ARCH=$(uname -m)
    TARGET=$1
    EXPECTED_ARCH=$2

    echo "Testing build: $TARGET"

    if [[ "$ARCH" != "$EXPECTED_ARCH" ]]; then
        echo "Cannot run $TARGET: current arch ($ARCH) does not match target arch ($EXPECTED_ARCH). Skipping test."
        return
    fi

    if [ -f "$TARGET" ]; then
        "$TARGET" temp/test.rx || echo "Test failed :("
    else
        echo "File $TARGET does not exist, skipping test."
    fi
}

mkdir -p install temp
echo 'print("Hello from RX!")' > temp/test.rx

case $choice in
    1)
        build_linux_x86_64
        run_test ./install/rx_linux_x86_64 x86_64
        ;;
    2)
        build_linux_arm
        run_test ./install/rx_linux_arm64 aarch64
        ;;
    3)
        build_macos_x86_64
        run_test ./install/rx_macos_x86_64 x86_64
        ;;
    4)
        build_macos_arm
        run_test ./install/rx_macos_arm64 arm64
        ;;
    5)
        build_linux_x86_64
        build_linux_arm
        build_macos_x86_64
        build_macos_arm

        run_test ./install/rx_linux_x86_64 x86_64
        run_test ./install/rx_linux_arm64 aarch64
        run_test ./install/rx_macos_x86_64 x86_64
        run_test ./install/rx_macos_arm64 arm64
        ;;
    *)
        echo "Invalid choice."
        exit 1
        ;;
esac

rm -rf temp
echo "Build finished."
