#!/bin/bash

set -e

IS_TERMUX=false
INSTALL_PATH=""

if [[ "$PREFIX" == *"com.termux"* ]]; then
    IS_TERMUX=true
    INSTALL_PATH="$HOME/.local/bin/rx"
else
    INSTALL_PATH="/usr/local/bin/rx"
fi

if [ -f "$INSTALL_PATH" ]; then
    if $IS_TERMUX; then
        rm -f "$INSTALL_PATH"
        sed -i '/export PATH="\$HOME\/.local\/bin:\$PATH"/d' "$HOME/.bashrc" 2>/dev/null || true
        sed -i '/export PATH="\$HOME\/.local\/bin:\$PATH"/d' "$HOME/.zshrc" 2>/dev/null || true
    else
        sudo rm -f "$INSTALL_PATH"
        if [[ "$OSTYPE" == "darwin"* ]]; then
            sed -i '' '/\/usr\/local\/bin\/rx/d' "$HOME/.bash_profile" 2>/dev/null || true
            sed -i '' '/\/usr\/local\/bin\/rx/d' "$HOME/.zshrc" 2>/dev/null || true
        else
            sed -i '/\/usr\/local\/bin\/rx/d' "$HOME/.bashrc" 2>/dev/null || true
            sed -i '/\/usr\/local\/bin\/rx/d' "$HOME/.zshrc" 2>/dev/null || true
        fi
    fi
    source "$HOME/.bashrc" 2>/dev/null || true
    source "$HOME/.zshrc" 2>/dev/null || true
    echo "rx uninstalled and PATH cleaned."
else
    echo "rx not found at $INSTALL_PATH, nothing to uninstall."
fi