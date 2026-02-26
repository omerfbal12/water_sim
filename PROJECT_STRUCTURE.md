# Project Structure

## File Organization

```
zed-base/
├── src/
│   └── main.cpp                    # Main application (474 lines)
├── build/                          # Build artifacts (created after compilation)
├── CMakeLists.txt                  # CMake build configuration
├── Makefile                        # Make build configuration
├── build.sh                        # Bash build script (recommended)
├── README.md                       # Main project documentation
├── BUILD_INSTRUCTIONS.md           # Detailed build guide
├── MACOS_SETUP.md                  # macOS-specific setup guide
├── FEATURES.md                     # Complete features documentation
├── PROJECT_STRUCTURE.md            # This file
└── .git/                          # Git repository

```

## File Descriptions

### Source Code

#### `src/main.cpp` (474 lines)
The complete application in a single file. Contains:
- **Physics Engine**: Gravity, parabolic trajectories, collision detection
- **Particle System**: Splash generation, particle physics, rendering
- **Rock Object**: Projectile simulation with velocity and position tracking
- **Game State**: Central state management
- **SDL2 Integration**: Window creation, event handling, rendering
- **Interactive Controls**: Keyboard input handling
- **Main Loop**: 60 FPS game loop with physics updates

Key Components:
- `Vec2` struct: 2D vector math with operators
- `SplashParticle` struct: Individual water droplet representation
- `SplashSystem` class: Particle generation and management
- `Rock` struct: Projectile object
- `GameState` struct: Current simulation state
- `SplashSimulation` class: Main application controller

### Build Configuration

#### `CMakeLists.txt`
CMake build configuration for cross-platform compilation:
- Finds SDL2 using pkg-config or standard find_package
- Handles macOS Homebrew paths
- Sets C++17 standard
- Configures compiler optimizations
- Links necessary frameworks on macOS

#### `Makefile`
GNU Make build configuration:
- Auto-detects OS (Linux, macOS, Windows)
- Uses pkg-config on Linux
- Handles Homebrew paths on macOS
- Simple targets: all, clean, run

#### `build.sh`
Bash build script (recommended for Unix-like systems):
- Auto-detects OS and compiler
- Checks for SDL2 installation
- Provides helpful error messages
- Creates build directory
- Compiles and links in one step
- Executable with: `./build.sh`

### Documentation

#### `README.md`
Main project documentation:
- Feature overview
- Quick start guide for macOS and Linux
- Control reference
- How it works explanation
- Building instructions (all methods)
- Customization guide
- System requirements
- Troubleshooting

#### `BUILD_INSTRUCTIONS.md`
Detailed build guide:
- Prerequisites for each OS
- Step-by-step build instructions
- Three build methods (build.sh, Makefile, CMake)
- Control reference
- Feature list
- Physics parameters
- Troubleshooting

#### `MACOS_SETUP.md`
macOS-specific guide:
- Xcode Command Line Tools setup
- Homebrew installation
- SDL2 installation verification
- Building and running
- macOS-specific troubleshooting
- Performance notes

#### `FEATURES.md`
Complete features documentation:
- Core simulation features
- Physics engine details
- Interactive features
- Visual features
- Physics parameters
- Advanced features
- Customization options
- Future enhancement ideas
- Performance metrics

#### `PROJECT_STRUCTURE.md`
This file - describes the project organization

## Build Artifacts (after compilation)

```
build/
├── main.o                  # Compiled object file
└── splash_sim              # Final executable (in project root)
```

## How to Use Each Build Method

### Method 1: build.sh (Recommended)
```bash
chmod +x build.sh
./build.sh
./splash_sim
```

### Method 2: Makefile
```bash
make              # Build
make run          # Build and run
make clean        # Clean build artifacts
```

### Method 3: CMake
```bash
mkdir build
cd build
cmake ..
make
cd ..
./splash_sim
```

## Code Statistics

| Metric | Value |
|--------|-------|
| Lines of Code | 474 |
| Main Classes | 3 |
| Structs | 4 |
| Functions | 15+ |
| Dependencies | SDL2 only |
| C++ Standard | C++17 |

## Key Components Explained

### Physics Engine
- Located in `Rock::Update()` and `SplashSystem::Update()`
- Implements gravity using `vel.y += kGravity * dt`
- Parabolic motion through continuous position updates
- Collision detection at water surface

### Particle System
- `CreateSplash()` generates particles based on impact speed
- Each particle has position, velocity, and lifetime
- Particles affected by gravity and water surface bouncing
- Alpha fade-out based on lifetime ratio

### Rendering System
- `DrawBackground()` - Sky blue background
- `DrawWater()` - Water surface and body
- `Rock::Draw()` - Brown circle for rock
- `SplashSystem::Draw()` - Blue circles for particles with fade

### Game Loop
- 60 FPS target with vsync
- Input handling via SDL_PollEvent
- Physics updates at fixed timestep (0.016 seconds)
- Render after each update

## Configuration Points

All easily customizable in `src/main.cpp`:

```cpp
// Physics
constexpr float kGravity = 500.0f;

// Rendering
constexpr int kWindowWidth = 1200;
constexpr int kWindowHeight = 600;

// Water
constexpr float kWaterY = 450.0f;

// Timing
constexpr float kDt = 0.016f;
```

## Dependencies

### External
- **SDL2**: Simple DirectMedia Layer for graphics and windowing
  - Provides window creation, event handling, rendering
  - Hardware-accelerated graphics
  - Cross-platform compatibility

### Standard Library
- `<cmath>` - Mathematical functions (sin, cos, sqrt)
- `<iostream>` - Console input/output
- `<random>` - Random number generation
- `<vector>` - Dynamic arrays for particles
- `<chrono>` - Timing (SDL2 handles most timing)

## Platform Support

- **macOS**: Intel and Apple Silicon (M1, M2, etc.)
- **Linux**: Ubuntu, Debian, Fedora, Arch, etc.
- **Windows**: With appropriate SDL2 setup

## Next Steps

1. **Build**: Choose one build method and compile
2. **Run**: Execute `./splash_sim`
3. **Experiment**: Try different speeds and angles
4. **Customize**: Modify physics parameters in `src/main.cpp`
5. **Extend**: Add new features (wind, multiple rocks, etc.)

## Getting Help

- Check `README.md` for overview
- See `BUILD_INSTRUCTIONS.md` for build issues
- Review `MACOS_SETUP.md` if on macOS
- Check `FEATURES.md` for feature details
- Look at error messages in terminal
