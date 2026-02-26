# Splash Simulation - Build Instructions

## Prerequisites

### macOS
You'll need to install SDL2. The easiest way is using Homebrew:

```bash
brew install sdl2 cmake
```

### Linux
```bash
sudo apt-get install libsdl2-dev cmake
```

### Windows
Download SDL2 from https://www.libsdl.org/download-2.0.php and follow the installation instructions.

## Building

1. Navigate to the project directory:
```bash
cd /path/to/zed-base
```

2. Create a build directory:
```bash
mkdir build
cd build
```

3. Run CMake:
```bash
cmake ..
```

4. Build the project:
```bash
make
```

5. Run the application:
```bash
./splash_sim
```

## Controls

- **UP/DOWN** - Adjust launch angle (10-80 degrees)
- **LEFT/RIGHT** - Adjust throw speed (100-800 pixels/second)
- **SPACE** - Throw the rock
- **R** - Reset simulation
- **H** - Toggle help text
- **ESC/Q** - Quit

## Features

- **Realistic Physics**: Parabolic trajectory with gravity (9.8 m/s² equivalent)
- **Impact-Based Splash**: Splash intensity scales with rock impact speed
- **Particle System**: Water droplets with physics-based motion
- **Interactive Controls**: Adjust speed and angle before throwing
- **Visual Feedback**: Real-time rendering of rock trajectory and splash

## Physics Parameters

You can adjust the following in `src/main.cpp`:
- `kGravity` - Gravitational acceleration (default: 500 pixels/s²)
- `kWaterY` - Water surface Y coordinate (default: 450)
- `kWindowWidth/Height` - Window dimensions (default: 1200x600)

## Troubleshooting

### SDL2 not found on macOS
If CMake can't find SDL2, try:
```bash
brew link sdl2
```

### Build fails with compiler errors
Make sure you're using C++17 or later:
```bash
g++ --version
clang --version
```

## Performance Notes

- Runs at 60 FPS on most systems
- Particle count scales with impact speed (20-100 particles per splash)
- Optimized rendering with SDL2 hardware acceleration
