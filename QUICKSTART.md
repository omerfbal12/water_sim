# Quick Start Guide

Get the Splash Simulation running in 5 minutes!

## For macOS Users

### Step 1: Install Dependencies (2 minutes)
```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install SDL2
brew install sdl2
```

### Step 2: Build (2 minutes)
```bash
cd /path/to/zed-base
./build.sh
```

### Step 3: Run (1 minute)
```bash
./splash_sim
```

## For Linux Users

### Step 1: Install Dependencies (1 minute)
```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev

# Fedora
sudo dnf install SDL2-devel

# Arch
sudo pacman -S sdl2
```

### Step 2: Build (2 minutes)
```bash
cd /path/to/zed-base
./build.sh
```

### Step 3: Run (1 minute)
```bash
./splash_sim
```

## First Time Using It

1. A window will open with a blue sky and water
2. The console will show available controls
3. Press SPACE to throw a rock
4. Use UP/DOWN arrows to change angle
5. Use LEFT/RIGHT arrows to change speed
6. Watch the splash particles!

## Quick Tips

- **Best splashes**: Speed 400-600, Angle 30-45 degrees
- **High splashes**: Speed 600+, Angle 60+ degrees
- **Small splashes**: Speed <300, Any angle
- **Reset**: Press R to clear and try again
- **Help**: Press H to see all controls

## Troubleshooting

### "SDL2 not found" (macOS)
```bash
brew install sdl2
brew link sdl2
./build.sh
```

### "SDL2 not found" (Linux)
```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev

# Fedora
sudo dnf install SDL2-devel

# Arch
sudo pacman -S sdl2

./build.sh
```

### "No compiler found"
- **macOS**: `xcode-select --install`
- **Linux**: `sudo apt-get install build-essential`

### Window doesn't appear
- Check terminal for error messages
- Try: `./splash_sim 2>&1`
- Make sure SDL2 is properly installed

## What You Can Do

✅ Throw rocks at different speeds
✅ Adjust launch angle
✅ Watch realistic splash animations
✅ Observe physics-based particle motion
✅ Reset and try again
✅ Customize physics in the code

## File Organization

```
zed-base/
├── src/main.cpp          ← The simulation code
├── build.sh              ← Build script (use this!)
├── Makefile              ← Alternative build
├── CMakeLists.txt        ← CMake configuration
├── README.md             ← Full documentation
└── MACOS_SETUP.md        ← macOS-specific help
```

## Next Steps

1. **Experiment**: Try different speeds and angles
2. **Read**: Check README.md for detailed info
3. **Customize**: Edit src/main.cpp to change physics
4. **Extend**: Add new features (wind, sound, etc.)

## Support

- **Build Issues**: See BUILD_INSTRUCTIONS.md
- **macOS Help**: See MACOS_SETUP.md
- **Feature Details**: See FEATURES.md
- **Code Structure**: See PROJECT_STRUCTURE.md

---

**That's it!** You're ready to go. Have fun throwing rocks! 🎮💧
