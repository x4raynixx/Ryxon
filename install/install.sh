#!/bin/bash
set -e

OS=""
ARCH=""

uname_os=$(uname -s)
uname_arch=$(uname -m)

case "$uname_os" in
    Linux) OS="linux" ;;
    Darwin) OS="macos" ;;
    CYGWIN*|MINGW*|MSYS*) OS="windows" ;; 
    *) echo "Unsupported OS: $uname_os"; exit 1 ;;
esac

case "$uname_arch" in
    x86_64|amd64) ARCH="x86_64" ;;
    aarch64|arm64) ARCH="arm64" ;;
    i686|i386) ARCH="x86" ;; 
    *) echo "Unsupported architecture: $uname_arch"; exit 1 ;;
esac

if [ -n "$PREFIX" ] && [[ "$PREFIX" == *"/data/data/"* ]]; then
    ENV_TERMUX=true
else
    ENV_TERMUX=false
fi

if [[ "$OS" == "windows" ]]; then
    FILE_NAME="rx.exe"
else
    FILE_NAME="rx_${OS}_${ARCH}"
fi

BASE_URL="https://raw.githubusercontent.com/x4raynixx/RX-Scripting/master/install"
DOWNLOAD_URL="$BASE_URL/$FILE_NAME"

TMPFILE=$(mktemp)

echo "Detecting environment..."
if $ENV_TERMUX; then
    echo "Termux environment detected. Architecture: $ARCH"
else
    echo "Detected OS: $OS, Architecture: $ARCH"
fi

echo "Downloading $FILE_NAME from GitHub..."
if ! curl -fsSL "$DOWNLOAD_URL" -o "$TMPFILE"; then
    echo "Failed to download binary from $DOWNLOAD_URL"
    rm -f "$TMPFILE"
    exit 1
fi

if [[ ! -s "$TMPFILE" ]]; then
    echo "Downloaded file is empty. Exiting."
    rm -f "$TMPFILE"
    exit 1
fi

if $ENV_TERMUX; then
    INSTALL_PATH="$HOME/.local/bin/rx"
    mkdir -p "$(dirname "$INSTALL_PATH")"
    mv "$TMPFILE" "$INSTALL_PATH"
    chmod +x "$INSTALL_PATH"
    export PATH="$HOME/.local/bin:$PATH"
    echo "Added ~/.local/bin to PATH."
    echo "Installation complete! Restart Termux or run: source ~/.bashrc or source ~/.zshrc"
else
    sudo mv "$TMPFILE" /usr/local/bin/rx
    sudo chmod +x /usr/local/bin/rx
    echo "Installation complete! You can now run 'rx' from anywhere."
fi