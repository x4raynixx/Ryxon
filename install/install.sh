#!/bin/bash set -e

OS="" ARCH=""

uname_os=$(uname -s) uname_arch=$(uname -m)

case "$uname_os" in Linux) OS="linux" ;; Darwin) OS="macos" ;; CYGWIN*|MINGW*|MSYS*) OS="windows" ;; *) echo "Unsupported OS: $uname_os"; exit 1 ;; esac

case "$uname_arch" in x86_64|amd64) ARCH="x86_64" ;; aarch64|arm64) ARCH="arm64" ;; i686|i386) ARCH="x86" ;; *) echo "Unsupported architecture: $uname_arch"; exit 1 ;; esac

if [ -n "$PREFIX" ] && [[ "$PREFIX" == "/data/data/" ]]; then ENV_TERMUX=true else ENV_TERMUX=false fi

BASE_URL="https://raw.githubusercontent.com/x4raynixx/RX-Scripting/master/install"

if $ENV_TERMUX; then echo "Detecting environment..." echo "Termux environment detected. Architecture: $ARCH"

pkg update -y && pkg install -y git clang build-essential
git clone https://github.com/x4raynixx/RX-Scripting
cd RX-Scripting/src
clang++ -std=c++17 -O2 *.cpp -o rx
mkdir -p $HOME/.local/bin
mv rx $HOME/.local/bin/rx
chmod +x $HOME/.local/bin/rx
echo "export PATH=\"$HOME/.local/bin:\$PATH\"" >> $HOME/.bashrc
echo "Added ~/.local/bin to PATH."
echo "Installation complete! Restart Termux or run: source ~/.bashrc or source ~/.zshrc"
exit 0

fi

if [[ "$OS" == "windows" ]]; then FILE_NAME="rx.exe" else FILE_NAME="rx_${OS}_${ARCH}" fi

DOWNLOAD_URL="$BASE_URL/$FILE_NAME" TMPFILE=$(mktemp)

echo "Detecting environment..." echo "Detected OS: $OS, Architecture: $ARCH" echo "Downloading $FILE_NAME from GitHub..."

if ! curl -fsSL "$DOWNLOAD_URL" -o "$TMPFILE"; then echo "Failed to download binary from $DOWNLOAD_URL" rm -f "$TMPFILE" exit 1 fi

if [[ ! -s "$TMPFILE" ]]; then echo "Downloaded file is empty. Exiting." rm -f "$TMPFILE" exit 1 fi

sudo mv "$TMPFILE" /usr/local/bin/rx sudo chmod +x /usr/local/bin/rx echo "Installation complete! You can now run 'rx' from anywhere."

