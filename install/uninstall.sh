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
    rm -f "$INSTALL_PATH"
fi

if $IS_TERMUX; then
    if [ -f "$HOME/.bashrc" ]; then
        sed -i '/\.local\/bin/d' "$HOME/.bashrc"
    fi
    if [ -f "$HOME/.zshrc" ]; then
        sed -i '/\.local\/bin/d' "$HOME/.zshrc"
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    if [ -f "$HOME/.bash_profile" ]; then
        sed -i '' '/\/usr\/local\/bin\/rx/d' "$HOME/.bash_profile"
    fi
    if [ -f "$HOME/.zshrc" ]; then
        sed -i '' '/\/usr\/local\/bin\/rx/d' "$HOME/.zshrc"
    fi
else
    if [ -f "$HOME/.bashrc" ]; then
        sed -i '/\/usr\/local\/bin\/rx/d' "$HOME/.bashrc"
    fi
    if [ -f "$HOME/.zshrc" ]; then
        sed -i '/\/usr\/local\/bin\/rx/d' "$HOME/.zshrc"
    fi
fi