# macOS Setup Guide

This guide is specifically for macOS users who want to build and run the Splash Simulation.

## Prerequisites

### 1. Install Xcode Command Line Tools

If you haven't already, install the Xcode command line tools:

```bash
xcode-select --install
```

This provides the C++ compiler and other build tools needed.

### 2. Install Homebrew (if not already installed)

Visit https://brew.sh/ or run:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3. Install SDL2

```bash
brew install sdl2
```

Verify the installation:

```bash
pkg-config --cflags --libs sdl2
```

You should see output like: `-I/usr/local/opt/sdl2/include -L/usr/local/opt/sdl2/lib -lSDL2`

## Building

Navigate to the project directory and run:

```bash
./build.sh
```

If you encounter any issues, you can also use:

```bash
make
```

Or if you prefer CMake:

```bash
mkdir build
cd build
cmake ..
make
```

## Running

After successful build:

```bash
./splash_sim
```

A window should appear with the simulation. If it doesn't:

1. Check the terminal for error messages
2. Ensure your Mac's display supports SDL2
3. Try running with verbose output: `./splash_sim 2>&1 | head -20`

## Controls

Once the application is running:

- **↑/↓ Keys** - Adjust launch angle
- **←/→ Keys** - Adjust throw speed
- **SPACE** - Throw the rock
- **R** - Reset
- **H** - Help
- **ESC or Q** - Quit

## Troubleshooting

### "command not found: ./build.sh"

Make sure you're in the project directory:

```bash
cd /path/to/zed-base
ls -la build.sh
```

If the file exists but isn't executable:

```bash
chmod +x build.sh
./build.sh
```

### "SDL2 not found"

Try these steps:

```bash
# Check if SDL2 is installed
brew list | grep sdl2

# If not installed, install it
brew install sdl2

# If already installed, try linking it
brew link sdl2 --force

# Then try building again
./build.sh
```

### "No such file or directory: src/main.cpp"

Make sure you're in the correct directory:

```bash
pwd
ls -la src/
```

### Build fails with "clang: error: linker command failed"

This usually means SDL2 isn't properly linked. Try:

```bash
brew reinstall sdl2
make clean
./build.sh
```

### Window appears but is black or unresponsive

- Try closing and reopening the application
- Check that your Mac isn't in low-power mode
- Try running from a different terminal window
- Restart your Mac if issues persist

## Performance Notes

The simulation should run smoothly on any modern Mac. If you experience lag:

1. Close other applications
2. Check Activity Monitor (Cmd+Space, type "Activity Monitor")
3. Ensure you have at least 100 MB of free RAM

## Getting Help

If you encounter issues:

1. Check the BUILD_INSTRUCTIONS.md file
2. Review the error messages in the terminal
3. Ensure all prerequisites are installed correctly
4. Try building in a fresh terminal window

## Next Steps

Once the application is running, try:

1. Throwing rocks at different angles
2. Adjusting the speed to see how impact affects splash size
3. Experimenting with different combinations of speed and angle
4. Observing how particle physics create realistic water effects

Enjoy the simulation!
