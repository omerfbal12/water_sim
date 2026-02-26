#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <string>

// ============================================================================
// Physics Constants
// ============================================================================
constexpr float kGravity = 500.0f;  // pixels/s^2
constexpr float kDt = 0.016f;       // ~60 FPS
constexpr float kWaterBaseY = 420.0f;  // Base water surface Y
constexpr float kWaveSpeed = 1.5f;  // Wave animation speed
constexpr float kWaveLayer1Amp = 15.0f;
constexpr float kWaveLayer2Amp = 7.0f;
constexpr float kWaveLayer3Amp = 3.0f;

// ============================================================================
// Window and Rendering
// ============================================================================
constexpr int kWindowWidth = 1200;
constexpr int kWindowHeight = 600;
constexpr int kThrowOriginX = 1000;
constexpr int kThrowOriginY = 120;

// ============================================================================
// Math Utilities
// ============================================================================
struct Vec2 {
  float x = 0.0f;
  float y = 0.0f;

  Vec2 operator+(const Vec2& other) const {
    return {x + other.x, y + other.y};
  }

  Vec2 operator-(const Vec2& other) const {
    return {x - other.x, y - other.y};
  }

  Vec2 operator*(float scalar) const {
    return {x * scalar, y * scalar};
  }

  float Length() const {
    return std::sqrt(x * x + y * y);
  }

  Vec2 Normalized() const {
    float len = Length();
    if (len < 0.0001f) return {0.0f, 0.0f};
    return {x / len, y / len};
  }
};

float Clamp(float value, float min_val, float max_val) {
  if (value < min_val) return min_val;
  if (value > max_val) return max_val;
  return value;
}

// ============================================================================
// Water Wave System
// ============================================================================
struct Ripple {
  Vec2 center;
  float radius = 0.0f;
  float max_radius = 200.0f;
  float amplitude = 20.0f;
  float life = 1.5f;
  float max_life = 1.5f;
};

class WaveSystem {
 public:
  std::vector<Ripple> ripples;
  float wave_time = 0.0f;

  void AddRipple(const Vec2& pos) {
    Ripple ripple;
    ripple.center = pos;
    ripple.radius = 0.0f;
    ripples.push_back(ripple);
  }

  void Update(float dt) {
    wave_time += dt * kWaveSpeed;
    
    for (auto& ripple : ripples) {
      ripple.radius += 200.0f * dt;
      ripple.life -= dt;
    }
    
    // Remove dead ripples
    std::vector<Ripple> alive_ripples;
    for (const auto& ripple : ripples) {
      if (ripple.life > 0.0f) {
        alive_ripples.push_back(ripple);
      }
    }
    ripples = alive_ripples;
  }

  float GetWaveHeight(float x, float time) const {
    float wave1 = std::sin(x * 0.02f + time) * kWaveLayer1Amp;
    float wave2 = std::sin(x * 0.05f + time * 1.3f) * kWaveLayer2Amp;
    float wave3 = std::sin(x * 0.1f + time * 0.7f) * kWaveLayer3Amp;
    float base_wave = wave1 + wave2 + wave3;
    
    float ripple_effect = 0.0f;
    for (const auto& ripple : ripples) {
      if (ripple.life <= 0.0f) continue;
      
      float dist = std::abs(x - ripple.center.x);
      if (dist < ripple.radius + 40.0f && dist > ripple.radius - 40.0f) {
        float falloff = 1.0f - (ripple.life / ripple.max_life);
        float wave_contrib = std::sin(dist * 0.05f) * ripple.amplitude * (1.0f - falloff);
        ripple_effect += wave_contrib;
      }
    }
    
    return base_wave + ripple_effect;
  }
};

// ============================================================================
// Simple Bitmap Font Rendering
// ============================================================================
class BitmapFont {
 public:
  static void DrawChar(SDL_Renderer* renderer, char c, int x, int y, int scale = 1) {
    if (c < 32 || c > 126) return;
    
    // Simple 5x7 character patterns
    const uint8_t patterns[95][7] = {
      // Space
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // !
      {0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00},
      // "
      {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00},
      // #
      {0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x00, 0x00},
      // $
      {0x04, 0x0E, 0x14, 0x0E, 0x05, 0x0E, 0x04},
      // %
      {0x11, 0x0A, 0x04, 0x04, 0x02, 0x05, 0x08},
      // &
      {0x0C, 0x12, 0x0C, 0x0E, 0x15, 0x12, 0x0D},
      // '
      {0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00},
      // (
      {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02},
      // )
      {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08},
      // *
      {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00},
      // +
      {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
      // ,
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08},
      // -
      {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00},
      // .
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00},
      // /
      {0x01, 0x02, 0x04, 0x08, 0x10, 0x08, 0x00},
      // 0
      {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E},
      // 1
      {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
      // 2
      {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},
      // 3
      {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E},
      // 4
      {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
      // 5
      {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
      // 6
      {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},
      // 7
      {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
      // 8
      {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
      // 9
      {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C},
      // :
      {0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00},
      // ;
      {0x00, 0x04, 0x00, 0x00, 0x04, 0x04, 0x08},
      // <
      {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02},
      // =
      {0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00},
      // >
      {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08},
      // ?
      {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04},
      // @
      {0x0E, 0x11, 0x0D, 0x15, 0x16, 0x10, 0x0E},
      // A-Z (simplified patterns)
      {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},  // A
      {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E},  // B
      {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E},  // C
      {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E},  // D
      {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F},  // E
      {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10},  // F
      {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F},  // G
      {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11},  // H
      {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},  // I
      {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E},  // J
      {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},  // K
      {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F},  // L
      {0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11},  // M
      {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},  // N
      {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},  // O
      {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},  // P
      {0x0E, 0x11, 0x11, 0x11, 0x13, 0x12, 0x0D},  // Q
      {0x1E, 0x11, 0x11, 0x1E, 0x12, 0x11, 0x11},  // R
      {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E},  // S
      {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},  // T
      {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E},  // U
      {0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04},  // V
      {0x11, 0x11, 0x11, 0x15, 0x1B, 0x11, 0x11},  // W
      {0x11, 0x0A, 0x04, 0x04, 0x04, 0x0A, 0x11},  // X
      {0x11, 0x0A, 0x04, 0x04, 0x04, 0x04, 0x04},  // Y
      {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F},  // Z
      // Remaining symbols
      {0x0E, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0E},  // [
      {0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00},  // backslash
      {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E},  // ]
      {0x04, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00},  // ^
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F},  // _
      {0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00},  // `
      {0x00, 0x0E, 0x01, 0x0F, 0x11, 0x11, 0x0F},  // a
      {0x10, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x1E},  // b
      {0x00, 0x0E, 0x11, 0x10, 0x10, 0x11, 0x0E},  // c
      {0x01, 0x01, 0x0F, 0x11, 0x11, 0x11, 0x0F},  // d
      {0x00, 0x0E, 0x11, 0x1F, 0x10, 0x11, 0x0E},  // e
      {0x06, 0x09, 0x08, 0x1C, 0x08, 0x08, 0x08},  // f
      {0x00, 0x0F, 0x11, 0x11, 0x0F, 0x01, 0x0E},  // g
      {0x10, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x11},  // h
      {0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E},  // i
      {0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C},  // j
      {0x10, 0x10, 0x11, 0x12, 0x1C, 0x12, 0x11},  // k
      {0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E},  // l
      {0x00, 0x1A, 0x15, 0x15, 0x15, 0x15, 0x15},  // m
      {0x00, 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11},  // n
      {0x00, 0x0E, 0x11, 0x11, 0x11, 0x11, 0x0E},  // o
      {0x00, 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10},  // p
      {0x00, 0x0F, 0x11, 0x11, 0x0F, 0x01, 0x01},  // q
      {0x00, 0x1E, 0x11, 0x10, 0x10, 0x10, 0x10},  // r
      {0x00, 0x0E, 0x10, 0x0E, 0x01, 0x11, 0x0E},  // s
      {0x04, 0x04, 0x0E, 0x04, 0x04, 0x04, 0x02},  // t
      {0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0F},  // u
      {0x00, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04},  // v
      {0x00, 0x11, 0x11, 0x15, 0x1B, 0x11, 0x11},  // w
      {0x00, 0x11, 0x0A, 0x04, 0x04, 0x0A, 0x11},  // x
      {0x00, 0x11, 0x11, 0x0F, 0x01, 0x11, 0x0E},  // y
      {0x00, 0x1F, 0x02, 0x04, 0x08, 0x10, 0x1F},  // z
      {0x06, 0x08, 0x08, 0x10, 0x08, 0x08, 0x06},  // {
      {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},  // |
      {0x0C, 0x02, 0x02, 0x01, 0x02, 0x02, 0x0C},  // }
      {0x08, 0x15, 0x01, 0x00, 0x00, 0x00, 0x00},  // ~
    };
    
    const auto& pattern = patterns[c - 32];
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    for (int row = 0; row < 7; ++row) {
      uint8_t byte = pattern[row];
      for (int col = 0; col < 5; ++col) {
        if (byte & (1 << (4 - col))) {
          SDL_Rect pixel = {x + col * scale, y + row * scale, scale, scale};
          SDL_RenderFillRect(renderer, &pixel);
        }
      }
    }
  }

  static void DrawString(SDL_Renderer* renderer, const std::string& str, int x, int y, int scale = 1) {
    int char_x = x;
    for (char c : str) {
      DrawChar(renderer, c, char_x, y, scale);
      char_x += 6 * scale;
    }
  }

  static int GetStringWidth(const std::string& str, int scale = 1) {
    return static_cast<int>(str.length() * 6 * scale);
  }
};

// ============================================================================
// Particle System for Splash
// ============================================================================
enum class ParticleType {
  Droplet = 0,
  Mist = 1,
  Foam = 2,
};

struct SplashParticle {
  Vec2 pos;
  Vec2 vel;
  float life;
  float max_life;
  float radius;
  int type;
};

class SplashSystem {
 public:
  std::vector<SplashParticle> particles;

  void CreateSplash(const Vec2& impact_pos, float impact_speed) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> angle_dist(30.0f, 150.0f);
    std::uniform_real_distribution<float> speed_var(0.6f, 1.4f);
    std::uniform_int_distribution<int> type_dist(0, 2);

    int particle_count = static_cast<int>(Clamp(impact_speed * 0.2f, 30.0f, 140.0f));
    float base_speed = Clamp(impact_speed * 0.45f, 120.0f, 420.0f);
    float lifetime = Clamp(impact_speed * 0.01f, 0.9f, 2.4f);

    for (int i = 0; i < particle_count; ++i) {
      float angle_deg = angle_dist(rng);
      float angle_rad = angle_deg * 3.14159265f / 180.0f;
      float speed = base_speed * speed_var(rng);
      int type = type_dist(rng);

      SplashParticle particle;
      particle.pos = impact_pos;
      particle.vel = {std::cos(angle_rad) * speed, std::sin(angle_rad) * speed};
      particle.life = lifetime;
      particle.max_life = lifetime;
      particle.type = type;

      switch (static_cast<ParticleType>(type)) {
        case ParticleType::Droplet:
          particle.radius = 3.0f;
          break;
        case ParticleType::Mist:
          particle.radius = 1.0f;
          particle.vel = particle.vel * 0.6f;
          break;
        case ParticleType::Foam:
          particle.radius = 2.0f;
          particle.vel = particle.vel * 0.4f;
          break;
      }

      particles.push_back(particle);
    }
  }

  void Update(float dt) {
    for (auto& p : particles) {
      if (p.life <= 0.0f) continue;

      // Apply gravity
      p.vel.y += kGravity * dt;

      // Update position
      p.pos = p.pos + p.vel * dt;

      // Decay life
      p.life -= dt;

      // Bounce off water surface (note: wave height is handled elsewhere)
      if (p.pos.y >= kWaterBaseY) {
        p.pos.y = kWaterBaseY;
        p.vel.y *= -0.3f;
        p.vel.x *= 0.8f;
      }

      // Bounce off ground/edges
      if (p.pos.y >= kWindowHeight - 10.0f) {
        p.pos.y = kWindowHeight - 10.0f;
        p.vel.y = 0.0f;
        p.vel.x *= 0.7f;
      }
    }

    // Remove dead particles
    std::vector<SplashParticle> alive_particles;
    for (const auto& p : particles) {
      if (p.life > 0.0f) {
        alive_particles.push_back(p);
      }
    }
    particles = alive_particles;
  }

  void Draw(SDL_Renderer* renderer) {
    for (const auto& p : particles) {
      if (p.life <= 0.0f) continue;

      // Fade out as particle dies
      float alpha = (p.life / p.max_life) * 255.0f;
      uint8_t alpha_byte = static_cast<uint8_t>(alpha);

      // Draw particle based on type
      switch (static_cast<ParticleType>(p.type)) {
        case ParticleType::Droplet:
          SDL_SetRenderDrawColor(renderer, 90, 140, 200, alpha_byte);
          break;
        case ParticleType::Mist:
          SDL_SetRenderDrawColor(renderer, 180, 220, 255, static_cast<uint8_t>(alpha_byte * 0.6f));
          break;
        case ParticleType::Foam:
          SDL_SetRenderDrawColor(renderer, 220, 235, 245, alpha_byte);
          break;
      }

      DrawFilledCircle(renderer, static_cast<int>(p.pos.x),
                       static_cast<int>(p.pos.y), static_cast<int>(p.radius));
    }
  }

 private:
  void DrawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int x = -radius; x <= radius; ++x) {
      for (int y = -radius; y <= radius; ++y) {
        if (x * x + y * y <= radius * radius) {
          SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        }
      }
    }
  }
};

// ============================================================================
// Rock Object
// ============================================================================
struct Rock {
  Vec2 pos;
  Vec2 vel;
  float radius = 6.0f;
  bool active = false;
  bool hit_water = false;

  void Update(float dt, const WaveSystem& waves) {
    if (!active) return;

    // Apply gravity
    vel.y += kGravity * dt;

    // Update position
    pos = pos + vel * dt;

    // Check if hit water (using wave height)
    float water_height = kWaterBaseY + waves.GetWaveHeight(pos.x, waves.wave_time);
    if (pos.y >= water_height && !hit_water) {
      hit_water = true;
      active = false;
    }

    // Clamp to window
    if (pos.x < 0.0f) pos.x = 0.0f;
    if (pos.x > static_cast<float>(kWindowWidth))
      pos.x = static_cast<float>(kWindowWidth);
    if (pos.y > static_cast<float>(kWindowHeight))
      pos.y = static_cast<float>(kWindowHeight);
  }

  void Draw(SDL_Renderer* renderer) {
    if (!active) return;

    SDL_SetRenderDrawColor(renderer, 139, 90, 43, 255);  // Brown
    DrawFilledCircle(renderer, static_cast<int>(pos.x),
                     static_cast<int>(pos.y), static_cast<int>(radius));
  }

 private:
  void DrawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    for (int x = -radius; x <= radius; ++x) {
      for (int y = -radius; y <= radius; ++y) {
        if (x * x + y * y <= radius * radius) {
          SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        }
      }
    }
  }
};

// ============================================================================
// Game State
// ============================================================================
struct GameState {
  Rock rock;
  SplashSystem splash;
  WaveSystem waves;
  float throw_speed = 300.0f;
  float throw_angle = 45.0f;
  bool game_running = true;
  bool show_help = true;
  bool is_dragging = false;
  Vec2 drag_start = {static_cast<float>(kThrowOriginX), static_cast<float>(kThrowOriginY)};
  Vec2 drag_end = {static_cast<float>(kThrowOriginX), static_cast<float>(kThrowOriginY)};
};

// ============================================================================
// Rendering Functions
// ============================================================================
void DrawWater(SDL_Renderer* renderer, const WaveSystem& waves) {
  for (int x = 0; x < kWindowWidth; ++x) {
    float wave_height = waves.GetWaveHeight(static_cast<float>(x), waves.wave_time);
    int surface_y = static_cast<int>(kWaterBaseY + wave_height);
    surface_y = Clamp(surface_y, 0, kWindowHeight - 1);

    for (int y = surface_y; y < kWindowHeight; ++y) {
      float t = (y - surface_y) / static_cast<float>(kWindowHeight - surface_y + 1);
      uint8_t r = static_cast<uint8_t>(20 + 30 * t);
      uint8_t g = static_cast<uint8_t>(80 + 70 * t);
      uint8_t b = static_cast<uint8_t>(130 + 90 * t);
      SDL_SetRenderDrawColor(renderer, r, g, b, 255);
      SDL_RenderDrawPoint(renderer, x, y);
    }

    SDL_SetRenderDrawColor(renderer, 130, 210, 235, 220);
    SDL_RenderDrawPoint(renderer, x, surface_y);
  }
}

void DrawBackground(SDL_Renderer* renderer) {
  for (int y = 0; y < kWindowHeight; ++y) {
    float t = y / static_cast<float>(kWindowHeight);
    uint8_t r = static_cast<uint8_t>(120 + 40 * t);
    uint8_t g = static_cast<uint8_t>(180 + 40 * t);
    uint8_t b = static_cast<uint8_t>(230 + 20 * t);
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawLine(renderer, 0, y, kWindowWidth, y);
  }
}

void DrawArrow(SDL_Renderer* renderer, const Vec2& start, const Vec2& end) {
  SDL_SetRenderDrawColor(renderer, 255, 180, 60, 200);
  SDL_RenderDrawLine(renderer, static_cast<int>(start.x), static_cast<int>(start.y),
                     static_cast<int>(end.x), static_cast<int>(end.y));

  Vec2 dir = (start - end).Normalized();
  Vec2 left = {dir.x * 12.0f - dir.y * 6.0f, dir.y * 12.0f + dir.x * 6.0f};
  Vec2 right = {dir.x * 12.0f + dir.y * 6.0f, dir.y * 12.0f - dir.x * 6.0f};

  SDL_RenderDrawLine(renderer, static_cast<int>(end.x), static_cast<int>(end.y),
                     static_cast<int>(end.x + left.x), static_cast<int>(end.y + left.y));
  SDL_RenderDrawLine(renderer, static_cast<int>(end.x), static_cast<int>(end.y),
                     static_cast<int>(end.x + right.x), static_cast<int>(end.y + right.y));
}

void DrawTrajectoryArc(SDL_Renderer* renderer, float speed, float angle_deg) {
  float angle_rad = angle_deg * 3.14159265f / 180.0f;
  
  SDL_SetRenderDrawColor(renderer, 200, 200, 100, 120);
  
  int num_points = 50;
  for (int i = 1; i <= num_points; ++i) {
    float t = (i / static_cast<float>(num_points)) * 2.5f;
    
    float x = kThrowOriginX - std::cos(angle_rad) * speed * t;
    float y = kThrowOriginY - (std::sin(angle_rad) * speed * t - 0.5f * kGravity * t * t);
    
    int screen_x = static_cast<int>(x);
    int screen_y = static_cast<int>(y);
    
    if (screen_x < 0 || screen_x >= kWindowWidth || screen_y < 0 || screen_y >= kWindowHeight) {
      break;
    }
    
    if (i % 2 == 0) {
      SDL_RenderDrawPoint(renderer, screen_x, screen_y);
    }
  }
  
  SDL_SetRenderDrawColor(renderer, 255, 200, 0, 220);
  for (int dx = -4; dx <= 4; ++dx) {
    for (int dy = -4; dy <= 4; ++dy) {
      if (dx * dx + dy * dy <= 16) {
        SDL_RenderDrawPoint(renderer, kThrowOriginX + dx, kThrowOriginY + dy);
      }
    }
  }
}

void DrawUI(SDL_Renderer* renderer, const GameState& state) {
  SDL_SetRenderDrawColor(renderer, 20, 20, 40, 220);
  SDL_Rect ui_box = {10, 10, 300, 150};
  SDL_RenderFillRect(renderer, &ui_box);

  SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
  SDL_RenderDrawRect(renderer, &ui_box);

  BitmapFont::DrawString(renderer, "THROW CONTROL", 20, 20, 1);
  
  char speed_str[32];
  snprintf(speed_str, sizeof(speed_str), "Speed: %.0f px/s", state.throw_speed);
  BitmapFont::DrawString(renderer, speed_str, 20, 35, 1);
  
  char angle_str[32];
  snprintf(angle_str, sizeof(angle_str), "Angle: %.1f deg", state.throw_angle);
  BitmapFont::DrawString(renderer, angle_str, 20, 50, 1);

  BitmapFont::DrawString(renderer, "Drag mouse to aim", 20, 70, 1);
  BitmapFont::DrawString(renderer, "Release to throw", 20, 85, 1);
  BitmapFont::DrawString(renderer, "Arrows: Adjust", 20, 100, 1);
  BitmapFont::DrawString(renderer, "R: Reset  H: Help", 20, 115, 1);
  BitmapFont::DrawString(renderer, "Q: Quit", 20, 130, 1);
}

// ============================================================================
// Main Application
// ============================================================================
class SplashSimulation {
 public:
  SplashSimulation()
      : window(nullptr), renderer(nullptr), running(true) {}

  ~SplashSimulation() {
    Cleanup();
  }

  bool Initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      std::cerr << "SDL initialization failed: " << SDL_GetError() << '\n';
      return false;
    }

    window = SDL_CreateWindow("Splash Simulation", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, kWindowWidth,
                              kWindowHeight, SDL_WINDOW_SHOWN);

    if (!window) {
      std::cerr << "Window creation failed: " << SDL_GetError() << '\n';
      return false;
    }

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
      std::cerr << "Renderer creation failed: " << SDL_GetError() << '\n';
      return false;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    return true;
  }

  void Run() {
    PrintControls();

    uint32_t last_time = SDL_GetTicks();
    const uint32_t frame_time = 16;  // ~60 FPS

    while (running) {
      uint32_t current_time = SDL_GetTicks();
      uint32_t delta_ms = current_time - last_time;

      if (delta_ms < frame_time) {
        SDL_Delay(frame_time - delta_ms);
        continue;
      }

      last_time = current_time;

      HandleEvents();
      Update(kDt);
      Render();
    }
  }

 private:
  SDL_Window* window;
  SDL_Renderer* renderer;
  bool running;
  GameState state;

  void PrintControls() {
    std::cout << "\n=== Splash Simulation Controls ===\n";
    std::cout << "UP/DOWN      - Adjust throw angle (current: " << state.throw_angle
              << " degrees)\n";
    std::cout << "LEFT/RIGHT   - Adjust throw speed (current: " << state.throw_speed
              << " pixels/s)\n";
    std::cout << "SPACE        - Throw rock\n";
    std::cout << "R            - Reset\n";
    std::cout << "H            - Toggle help\n";
    std::cout << "ESC/Q        - Quit\n";
    std::cout << "==================================\n\n";
  }

  void HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = false;
          break;

        case SDL_KEYDOWN:
          HandleKeyDown(event.key.keysym.sym);
          break;

        case SDL_MOUSEBUTTONDOWN:
          if (event.button.button == SDL_BUTTON_LEFT) {
            state.is_dragging = true;
            state.drag_start = {static_cast<float>(kThrowOriginX), static_cast<float>(kThrowOriginY)};
            state.drag_end = {static_cast<float>(event.button.x), static_cast<float>(event.button.y)};
          }
          break;

        case SDL_MOUSEBUTTONUP:
          if (event.button.button == SDL_BUTTON_LEFT && state.is_dragging) {
            state.is_dragging = false;
            state.drag_end = {static_cast<float>(event.button.x), static_cast<float>(event.button.y)};
            UpdateThrowFromDrag();
            ThrowRock();
          }
          break;

        case SDL_MOUSEMOTION:
          if (state.is_dragging) {
            state.drag_end = {static_cast<float>(event.motion.x), static_cast<float>(event.motion.y)};
            UpdateThrowFromDrag();
          }
          break;

        default:
          break;
      }
    }
  }

  void HandleKeyDown(SDL_Keycode key) {
    switch (key) {
      case SDLK_ESCAPE:
      case SDLK_q:
        running = false;
        break;

      case SDLK_SPACE:
        ThrowRock();
        break;

      case SDLK_UP:
        state.throw_angle = Clamp(state.throw_angle + 2.0f, 10.0f, 80.0f);
        PrintStatus();
        break;

      case SDLK_DOWN:
        state.throw_angle = Clamp(state.throw_angle - 2.0f, 10.0f, 80.0f);
        PrintStatus();
        break;

      case SDLK_RIGHT:
        state.throw_speed = Clamp(state.throw_speed + 20.0f, 100.0f, 800.0f);
        PrintStatus();
        break;

      case SDLK_LEFT:
        state.throw_speed = Clamp(state.throw_speed - 20.0f, 100.0f, 800.0f);
        PrintStatus();
        break;

      case SDLK_r:
        state.rock.active = false;
        state.rock.hit_water = false;
        state.splash.particles.clear();
        PrintStatus();
        break;

      case SDLK_h:
        state.show_help = !state.show_help;
        PrintControls();
        break;

      default:
        break;
    }
  }

  void UpdateThrowFromDrag() {
    Vec2 direction = state.drag_end - state.drag_start;
    float distance = direction.Length();

    if (distance < 1.0f) {
      return;
    }

    Vec2 dir_norm = direction.Normalized();
    float angle_rad = std::atan2(-dir_norm.y, -dir_norm.x);
    float angle_deg = angle_rad * 180.0f / 3.14159265f;

    state.throw_angle = Clamp(angle_deg, 10.0f, 80.0f);
    state.throw_speed = Clamp(distance * 2.5f, 100.0f, 800.0f);
  }

  void ThrowRock() {
    state.rock.active = true;
    state.rock.hit_water = false;
    state.rock.pos = {static_cast<float>(kThrowOriginX), static_cast<float>(kThrowOriginY)};

    float angle_rad = state.throw_angle * 3.14159265f / 180.0f;
    state.rock.vel = {-std::cos(angle_rad) * state.throw_speed,
                      -std::sin(angle_rad) * state.throw_speed};

    std::cout << "Threw rock with speed " << state.throw_speed << " at angle "
              << state.throw_angle << " degrees\n";
  }

  void PrintStatus() {
    std::cout << "Angle: " << state.throw_angle << "° | Speed: "
              << state.throw_speed << " px/s\n";
  }

  void Update(float dt) {
    state.waves.Update(dt);
    state.rock.Update(dt, state.waves);

    if (state.rock.hit_water) {
      float impact_speed = state.rock.vel.Length();
      state.splash.CreateSplash(state.rock.pos, impact_speed);
      state.waves.AddRipple(state.rock.pos);
      state.rock.hit_water = false;
    }

    state.splash.Update(dt);
  }

  void Render() {
    DrawBackground(renderer);
    DrawWater(renderer, state.waves);
    
    DrawTrajectoryArc(renderer, state.throw_speed, state.throw_angle);

    if (state.is_dragging) {
      DrawArrow(renderer, state.drag_start, state.drag_end);
    }

    state.rock.Draw(renderer);
    state.splash.Draw(renderer);
    DrawUI(renderer, state);

    SDL_RenderPresent(renderer);
  }

  void Cleanup() {
    if (renderer) {
      SDL_DestroyRenderer(renderer);
      renderer = nullptr;
    }
    if (window) {
      SDL_DestroyWindow(window);
      window = nullptr;
    }
    SDL_Quit();
  }
};

// ============================================================================
// Entry Point
// ============================================================================
int main() {
  SplashSimulation app;

  if (!app.Initialize()) {
    return 1;
  }

  app.Run();
  return 0;
}
