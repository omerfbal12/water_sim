# 2D Splash Simulation

A realistic 2D water splash simulation game written in C++ using SDL2. Throw rocks at water and watch as they create physics-accurate splashes!

## Features

✨ **Physics-Accurate Simulation**
- Realistic parabolic rock trajectories with gravity
- Impact-based splash generation
- Particle system with gravity and collision detection
- Water droplet bouncing and decay

🎮 **Interactive Gameplay**
- Adjust rock throw speed (100-800 pixels/second)
- Adjust launch angle (10-80 degrees)
- Real-time visual feedback
- Reset simulation at any time

🎨 **Visual Quality**
- Smooth 60 FPS rendering
- SDL2 hardware acceleration
- Particle fade-out effects
- Water surface visualization

## Quick Start

### On macOS

1. Install SDL2 using Homebrew:
```bash
brew install sdl2
```

2. Build the project:
```bash
./build.sh
```

3. Run the simulation:
```bash
./splash_sim
```

### On Linux

1. Install SDL2 development files:
```bash
sudo apt-get install libsdl2-dev
```

2. Build the project:
```bash
./build.sh
```

3. Run the simulation:
```bash
./splash_sim
```

## Controls

| Key | Action |
|-----|--------|
| **UP/DOWN** | Adjust launch angle |
| **LEFT/RIGHT** | Adjust throw speed |
| **SPACE** | Throw the rock |
| **R** | Reset simulation |
| **H** | Toggle help text |
| **ESC/Q** | Quit |

## How It Works

### Rock Physics
- Rocks follow a parabolic trajectory under gravity
- Initial velocity is calculated from speed and angle
- Collision detection with water surface triggers splash

### Splash System
- Impact speed determines particle count and initial velocity
- Particles are ejected at random angles (30-150 degrees)
- Each particle experiences gravity and bounces off water
- Particles fade out over time based on impact speed

### Rendering
- Background: Sky blue
- Water: Darker blue body with lighter surface line
- Rock: Brown circle
- Splash particles: Light blue circles with alpha fade

## Building

### Using build.sh (Recommended)
```bash
./build.sh
```

### Using Makefile
```bash
make
```

### Using CMake
```bash
mkdir build && cd build
cmake ..
make
```

## Customization

Edit `src/main.cpp` to adjust:

```cpp
constexpr float kGravity = 500.0f;      // Gravitational acceleration
constexpr float kWaterY = 450.0f;       // Water surface Y position
constexpr int kWindowWidth = 1200;      // Window width
constexpr int kWindowHeight = 600;      // Window height
```

## System Requirements

- **OS**: macOS, Linux, or Windows
- **Compiler**: C++17 compatible (clang++, g++, or MSVC)
- **SDL2**: 2.0 or later
- **RAM**: 50 MB minimum
- **GPU**: Any GPU with SDL2 support

## Project Structure

```
zed-base/
├── src/
│   └── main.cpp              # Main application code
├── build/                    # Build artifacts (created after build)
├── CMakeLists.txt           # CMake configuration
├── Makefile                 # Make configuration
├── build.sh                 # Build script
├── BUILD_INSTRUCTIONS.md    # Detailed build guide
└── README.md               # This file
```

## Physics Details

### Gravity
- Simulated gravity: 500 pixels/second²
- Equivalent to real-world gravity scaled to window coordinates

### Splash Particle Count
- Minimum: 20 particles
- Maximum: 100 particles
- Formula: `impact_speed * 0.15` (clamped to 20-100)

### Particle Lifetime
- Minimum: 0.8 seconds
- Maximum: 2.4 seconds
- Formula: `impact_speed * 0.008` (clamped to 0.8-2.4)

### Particle Velocity
- Base speed: `impact_speed * 0.4` (clamped to 100-400 px/s)
- Direction: Random angle between 30-150 degrees
- Variation: ±40% random multiplier per particle

## Performance

- **Frame Rate**: 60 FPS (vsync enabled)
- **Particle Limit**: ~100 particles per splash
- **Memory Usage**: <50 MB
- **CPU Usage**: <5% on modern systems

## Troubleshooting

### "SDL2 not found" error
Make sure SDL2 is installed:
- **macOS**: `brew install sdl2`
- **Linux**: `sudo apt-get install libsdl2-dev`

### "No compiler found" error
Install a C++ compiler:
- **macOS**: `xcode-select --install`
- **Linux**: `sudo apt-get install build-essential`

### Window doesn't appear
- Check that your display supports SDL2
- Try running with: `./splash_sim 2>&1`
- Check console output for error messages

## License

This project is provided as-is for educational purposes.

## Credits

Built with:
- **SDL2** - Simple DirectMedia Layer
- **C++17** - Modern C++ standard
