# Makefile for Splash Simulation

CXX := clang++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
LDFLAGS := 

# Detect OS and set appropriate flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CXXFLAGS += $(shell pkg-config --cflags sdl2)
    LDFLAGS += $(shell pkg-config --libs sdl2)
else ifeq ($(UNAME_S),Darwin)
    # macOS with Homebrew SDL2
    SDL2_PATH ?= /usr/local/opt/sdl2
    ifeq ($(wildcard $(SDL2_PATH)),)
        SDL2_PATH := /opt/homebrew/opt/sdl2
    endif
    
    ifeq ($(wildcard $(SDL2_PATH)),)
        $(error SDL2 not found. Please install with: brew install sdl2)
    endif
    
    CXXFLAGS += -I$(SDL2_PATH)/include
    LDFLAGS += -L$(SDL2_PATH)/lib -lSDL2 -framework CoreFoundation
else
    # Windows or other
    CXXFLAGS += $(shell sdl2-config --cflags 2>/dev/null)
    LDFLAGS += $(shell sdl2-config --libs 2>/dev/null)
endif

# Source files
SRC := src/main.cpp
OBJ := build/main.o
TARGET := splash_sim

# Default target
all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build successful! Run with: ./$(TARGET)"

build/main.o: $(SRC)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf build $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
