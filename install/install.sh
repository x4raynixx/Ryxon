#!/bin/bash

set -e

OS=""
ARCH=""

uname_os=$(uname -s)
uname_arch=$(uname -m)

case "$uname_os" in
    Linux) OS="linux" ;;
    Darwin) OS="macos" ;;
    *) echo "Unsupported OS: $uname_os"; exit 1 ;;
esac

case "$uname_arch" in
    x86_64|amd64) ARCH="x86_64" ;;
    aarch64|arm64) ARCH="arm64" ;;
    *) echo "Unsupported architecture: $uname_arch"; exit 1 ;;
esac

if [[ "$PREFIX" == *"com.termux"* ]]; then
    echo "Detected Termux environment. Architecture: $ARCH"
    pkg update -y
    pkg install -y git curl clang make build-essential
    rm -rf rx_temp && mkdir rx_temp && cd rx_temp
    curl -fsSL https://api.github.com/repos/x4raynixx/RX-Scripting/contents/src | grep 'download_url' | cut -d '"' -f4 | while read url; do curl -fsSL "$url" -O; done
    clang++ -std=c++17 -Wall -Wextra -O2 *.cpp -o rx
    mkdir -p ~/.local/bin
    mv rx ~/.local/bin/rx
    chmod +x ~/.local/bin/rx
    grep -qxF 'export PATH="$HOME/.local/bin:$PATH"' ~/.bashrc || echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
    grep -qxF 'export PATH="$HOME/.local/bin:$PATH"' ~/.zshrc || echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.zshrc
    source ~/.bashrc 2>/dev/null || true
    source ~/.zshrc 2>/dev/null || true
    echo "Installation complete! You can now run 'rx'."
    cd .. && rm -rf rx_temp
    exit 0
fi

FILE_NAME="rx_${OS}_${ARCH}"
URL="https://files-hosterino.vercel.app/${FILE_NAME}"
TMPFILE=$(mktemp)

if ! curl -fsSL "$URL" -o "$TMPFILE"; then
    if [[ "$OS" != "linux" ]]; then rm -f "$TMPFILE"; exit 1; fi
    if [[ "$ARCH" == "arm64" ]] && grep -qi "raspberry" /proc/cpuinfo 2>/dev/null; then
        sudo apt update && sudo apt upgrade -y
        sudo apt install -y build-essential libc6 libstdc++6
    fi
    if ! command -v g++ &>/dev/null; then
        sudo apt update && sudo apt install -y build-essential
    fi
    rm -rf rx_temp && mkdir rx_temp && cd rx_temp
    curl -fsSL https://api.github.com/repos/x4raynixx/RX-Scripting/contents/src | grep 'download_url' | cut -d '"' -f4 | while read url; do curl -fsSL "$url" -O; done
    g++ -std=c++17 -Wall -Wextra -O2 *.cpp -o rx_build
    sudo mv rx_build /usr/local/bin/rx
    sudo chmod +x /usr/local/bin/rx
    cd .. && rm -rf rx_temp
    rm -f "$TMPFILE"
    echo "Build and installation complete! You can now run 'rx'."
    exit 0
fi

if [[ ! -s "$TMPFILE" ]]; then
    rm -f "$TMPFILE"
    exit 1
fi

sudo mv "$TMPFILE" /usr/local/bin/rx
sudo chmod +x /usr/local/bin/rx
echo "Installation complete! You can now run 'rx' from anywhere."
echo "Restart terminal or run: source ~/.bashrc or source ~/.zshrc to apply changes"