#!/bin/bash

# Build script for Splash Simulation
# This script handles building on macOS, Linux, and other Unix-like systems

set -e

echo "================================"
echo "Splash Simulation Build Script"
echo "================================"

# Detect OS
OS=$(uname -s)
echo "Detected OS: $OS"

# Check for required tools
check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 is not installed or not in PATH"
        return 1
    fi
}

# Check for C++ compiler
if check_command clang++; then
    CXX="clang++"
elif check_command g++; then
    CXX="g++"
elif check_command c++; then
    CXX="c++"
else
    echo "Error: No C++ compiler found (clang++, g++, or c++)"
    exit 1
fi

echo "Using compiler: $CXX"

# Check for SDL2
echo "Checking for SDL2..."

if [ "$OS" = "Darwin" ]; then
    # macOS
    echo "macOS detected. Looking for SDL2..."
    
    if [ -d "/usr/local/opt/sdl2" ]; then
        SDL2_PATH="/usr/local/opt/sdl2"
        echo "Found SDL2 at: $SDL2_PATH"
    elif [ -d "/opt/homebrew/opt/sdl2" ]; then
        SDL2_PATH="/opt/homebrew/opt/sdl2"
        echo "Found SDL2 at: $SDL2_PATH"
    else
        echo ""
        echo "ERROR: SDL2 not found!"
        echo ""
        echo "Please install SDL2 using Homebrew:"
        echo "  brew install sdl2"
        echo ""
        exit 1
    fi
    
    SDL2_CFLAGS="-I$SDL2_PATH/include"
    SDL2_LIBS="-L$SDL2_PATH/lib -lSDL2 -framework CoreFoundation"
    
elif [ "$OS" = "Linux" ]; then
    # Linux
    echo "Linux detected. Checking for SDL2..."
    
    if ! pkg-config --exists sdl2; then
        echo ""
        echo "ERROR: SDL2 development files not found!"
        echo ""
        echo "Please install SDL2:"
        echo "  Ubuntu/Debian: sudo apt-get install libsdl2-dev"
        echo "  Fedora/RHEL:   sudo dnf install SDL2-devel"
        echo "  Arch:          sudo pacman -S sdl2"
        echo ""
        exit 1
    fi
    
    SDL2_CFLAGS=$(pkg-config --cflags sdl2)
    SDL2_LIBS=$(pkg-config --libs sdl2)
    echo "SDL2 found via pkg-config"
else
    echo "Unsupported OS: $OS"
    exit 1
fi

# Create build directory
mkdir -p build

# Compile
echo ""
echo "Compiling..."
CXXFLAGS="-std=c++17 -O2 -Wall -Wextra"
$CXX $CXXFLAGS $SDL2_CFLAGS -c src/main.cpp -o build/main.o

# Link
echo "Linking..."
$CXX -o splash_sim build/main.o $SDL2_LIBS

echo ""
echo "================================"
echo "Build successful!"
echo "================================"
echo ""
echo "Run the application with:"
echo "  ./splash_sim"
echo ""
