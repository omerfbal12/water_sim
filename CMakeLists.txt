cmake_minimum_required(VERSION 3.10)
project(SplashSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to find SDL2 using pkg-config first (works on most Unix-like systems)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(SDL2 sdl2)
endif()

# If pkg-config didn't find it, try the standard find_package
if(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED)
endif()

# Create executable
add_executable(splash_sim src/main.cpp)

# Link SDL2
if(SDL2_LIBRARIES)
    target_link_libraries(splash_sim ${SDL2_LIBRARIES})
else()
    target_link_libraries(splash_sim SDL2)
endif()

if(SDL2_INCLUDE_DIRS)
    target_include_directories(splash_sim PRIVATE ${SDL2_INCLUDE_DIRS})
endif()

# Platform-specific settings
if(APPLE)
    target_link_libraries(splash_sim "-framework CoreFoundation")
    # On macOS, SDL2 might be installed via Homebrew
    if(EXISTS /usr/local/opt/sdl2)
        target_include_directories(splash_sim PRIVATE /usr/local/opt/sdl2/include)
        target_link_directories(splash_sim PRIVATE /usr/local/opt/sdl2/lib)
    elseif(EXISTS /opt/homebrew/opt/sdl2)
        target_include_directories(splash_sim PRIVATE /opt/homebrew/opt/sdl2/include)
        target_link_directories(splash_sim PRIVATE /opt/homebrew/opt/sdl2/lib)
    endif()
endif()

# Add some compiler flags for optimization
if(MSVC)
    target_compile_options(splash_sim PRIVATE /O2)
else()
    target_compile_options(splash_sim PRIVATE -O2)
endif()
