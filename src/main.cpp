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

// Water physics constants
constexpr float kWaterDragCoefficient = 2.5f;  // Drag coefficient in water
constexpr float kWaterBuoyancy = 350.0f;       // Upward buoyancy force (less than gravity, so ball sinks slowly)
constexpr float kTerminalVelocityWater = 80.0f; // Maximum sinking speed in water

// Stone skipping constants
constexpr float kSkipMinSpeed = 150.0f;        // Minimum speed to skip (lowered for easier triggering)
constexpr float kSkipMaxAngle = 60.0f;         // Maximum angle (degrees) for skipping (exaggerated)
constexpr float kSkipMinAngle = 1.0f;          // Minimum angle for skipping (very shallow works)
constexpr float kSkipVelocityDamping = 0.5f;   // Velocity retained after skip
constexpr float kSkipBounceMultiplier = 1.2f;  // Upward bounce multiplier
constexpr int kSkipMaxBounces = 2;             // Maximum number of skips

// Bubble constants
constexpr float kBubbleSpawnRate = 0.03f;      // Seconds between bubble spawns
constexpr float kBubbleRiseSpeed = 40.0f;      // Bubbles rise speed

// ============================================================================
// Window and Rendering
// ============================================================================
constexpr int kWindowWidth = 1200;
constexpr int kWindowHeight = 600;
constexpr int kThrowOriginX = 1000;
constexpr int kThrowOriginY = 200;  // Lowered from 120 to make skipping easier

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
  float max_radius = 300.0f;
  float amplitude = 25.0f;
  float life = 2.5f;
  float max_life = 2.5f;
  float impact_speed = 200.0f;  // Store impact speed for amplitude scaling
};

class WaveSystem {
 public:
  std::vector<Ripple> ripples;
  float wave_time = 0.0f;
  float water_displacement = 0.0f;  // Tracks overall water level displacement
  float displacement_velocity = 0.0f;

  void AddRipple(const Vec2& pos, float impact_speed) {
    Ripple ripple;
    ripple.center = pos;
    ripple.radius = 0.0f;
    ripple.impact_speed = impact_speed;
    // Scale amplitude based on impact speed
    ripple.amplitude = Clamp(impact_speed * 0.08f, 15.0f, 40.0f);
    ripples.push_back(ripple);
    
    // Add initial water displacement based on impact
    displacement_velocity += impact_speed * 0.015f;
  }

  void Update(float dt) {
    wave_time += dt * kWaveSpeed;
    
    // Update water displacement with spring-like behavior
    // This creates a realistic "bobbing" effect when the water is disturbed
    float displacement_target = 0.0f;
    float spring_force = (water_displacement - displacement_target) * -2.0f;
    float damping = displacement_velocity * -0.5f;
    displacement_velocity += (spring_force + damping) * dt;
    water_displacement += displacement_velocity * dt;
    
    // Clamp displacement to reasonable bounds
    water_displacement = Clamp(water_displacement, -20.0f, 20.0f);
    
    for (auto& ripple : ripples) {
      // Ripple expansion slows down as it gets larger
      float expansion_speed = 150.0f * (1.0f - ripple.radius / ripple.max_radius * 0.5f);
      ripple.radius += expansion_speed * dt;
      ripple.life -= dt;
    }
    
    // Remove dead ripples using erase-remove idiom for in-place removal
    std::erase_if(ripples, [](const Ripple& r) { return r.life <= 0.0f; });
  }

  float GetWaveHeight(float x, float time) const {
    // Base waves - continuous sine waves
    float wave1 = std::sin(x * 0.02f + time) * kWaveLayer1Amp;
    float wave2 = std::sin(x * 0.05f + time * 1.3f) * kWaveLayer2Amp;
    float wave3 = std::sin(x * 0.1f + time * 0.7f) * kWaveLayer3Amp;
    float base_wave = wave1 + wave2 + wave3;
    
    // Ripple effect - smooth, circular waves emanating from impact points
    float ripple_effect = 0.0f;
    for (const auto& ripple : ripples) {
      if (ripple.life <= 0.0f) continue;
      
      float dist = std::abs(x - ripple.center.x);
      float ring_width = 60.0f;  // Width of the ripple ring
      
      // Create smooth ring-shaped waves
      float dist_from_ring = std::abs(dist - ripple.radius);
      
      if (dist_from_ring < ring_width) {
        // Life-based fade (ripples fade as they age)
        float life_factor = ripple.life / ripple.max_life;
        
        // Distance-based smooth falloff within the ring
        float ring_factor = 1.0f - (dist_from_ring / ring_width);
        ring_factor = ring_factor * ring_factor;  // Quadratic falloff for smoothness
        
        // Radial distance falloff (ripples weaken as they spread)
        float radial_falloff = 1.0f - (ripple.radius / ripple.max_radius);
        radial_falloff = radial_falloff * radial_falloff;
        
        // Sinusoidal wave pattern
        float wave_phase = dist * 0.08f - time * 3.0f;
        float wave_value = std::sin(wave_phase);
        
        // Combine all factors for smooth, realistic waves
        float contribution = wave_value * ripple.amplitude * life_factor * ring_factor * radial_falloff;
        ripple_effect += contribution;
      }
    }
    
    // Combine base waves with ripple effect and water displacement
    // The displacement creates a subtle overall level change
    return base_wave + ripple_effect + water_displacement;
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

  void Update(float dt, const WaveSystem& waves) {
    for (auto& p : particles) {
      if (p.life <= 0.0f) continue;

      // Apply gravity
      p.vel.y += kGravity * dt;

      // Update position
      p.pos = p.pos + p.vel * dt;

      // Decay life
      p.life -= dt;

      // Bounce off dynamic water surface using wave height at particle's x position
      float water_height = kWaterBaseY + waves.GetWaveHeight(p.pos.x, waves.wave_time);
      if (p.pos.y >= water_height) {
        p.pos.y = water_height;
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

    // Remove dead particles using erase-remove idiom for in-place removal
    std::erase_if(particles, [](const SplashParticle& p) { return p.life <= 0.0f; });
  }

  void Draw(SDL_Renderer* renderer) {
    for (const auto& p : particles) {
      if (p.life <= 0.0f) continue;

      // Fade out as particle dies - clamp alpha to valid range
      float alpha = Clamp((p.life / p.max_life) * 255.0f, 0.0f, 255.0f);
      uint8_t alpha_byte = static_cast<uint8_t>(alpha);

      // Draw particle based on type
      switch (static_cast<ParticleType>(p.type)) {
        case ParticleType::Droplet:
          SDL_SetRenderDrawColor(renderer, 90, 140, 200, alpha_byte);
          break;
        case ParticleType::Mist:
          SDL_SetRenderDrawColor(renderer, 180, 220, 255, static_cast<uint8_t>(Clamp(alpha_byte * 0.6f, 0.0f, 255.0f)));
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
// Bubble System for Sinking Balls
// ============================================================================
struct Bubble {
  Vec2 pos;
  float radius;
  float life;
  float max_life;
  float wobble_offset;
};

class BubbleSystem {
 public:
  std::vector<Bubble> bubbles;
  float spawn_timer = 0.0f;

  void SpawnBubble(const Vec2& ball_pos, float ball_radius) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> offset_dist(-ball_radius, ball_radius);
    std::uniform_real_distribution<float> radius_dist(1.0f, 3.0f);
    std::uniform_real_distribution<float> life_dist(1.0f, 2.5f);
    std::uniform_real_distribution<float> wobble_dist(0.0f, 6.28f);

    Bubble bubble;
    bubble.pos = {ball_pos.x + offset_dist(rng), ball_pos.y + offset_dist(rng)};
    bubble.radius = radius_dist(rng);
    bubble.max_life = life_dist(rng);
    bubble.life = bubble.max_life;
    bubble.wobble_offset = wobble_dist(rng);
    bubbles.push_back(bubble);
  }

  void Update(float dt, const WaveSystem& waves) {
    for (auto& b : bubbles) {
      if (b.life <= 0.0f) continue;

      // Bubbles rise upward with wobble
      b.pos.y -= kBubbleRiseSpeed * dt;
      b.pos.x += std::sin(b.wobble_offset + b.life * 5.0f) * 0.5f;

      // Decay life
      b.life -= dt;

      // Pop bubbles when they reach water surface
      float water_height = kWaterBaseY + waves.GetWaveHeight(b.pos.x, waves.wave_time);
      if (b.pos.y <= water_height) {
        b.life = 0.0f;  // Pop!
      }
    }

    // Remove dead bubbles
    std::erase_if(bubbles, [](const Bubble& b) { return b.life <= 0.0f; });
  }

  void Draw(SDL_Renderer* renderer) {
    for (const auto& b : bubbles) {
      if (b.life <= 0.0f) continue;

      // Fade out as bubble rises
      float alpha = Clamp((b.life / b.max_life) * 180.0f, 0.0f, 180.0f);
      uint8_t alpha_byte = static_cast<uint8_t>(alpha);

      // Light blue/white bubble
      SDL_SetRenderDrawColor(renderer, 200, 230, 255, alpha_byte);
      DrawFilledCircle(renderer, static_cast<int>(b.pos.x), static_cast<int>(b.pos.y), 
                       static_cast<int>(b.radius));
      
      // Highlight
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<uint8_t>(alpha * 0.5f));
      SDL_RenderDrawPoint(renderer, static_cast<int>(b.pos.x) - 1, static_cast<int>(b.pos.y) - 1);
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
  bool hit_water = false;  // True when ball first hits water (triggers splash)
  bool in_water = false;   // True while ball is in water
  bool is_moving = true;   // True while ball has significant velocity
  int skip_count = 0;      // Number of skips performed
  float bubble_spawn_timer = 0.0f;

  // Check if the ball should skip based on angle and speed
  bool ShouldSkip() const {
    float speed = vel.Length();
    if (speed < kSkipMinSpeed) return false;
    if (skip_count >= kSkipMaxBounces) return false;
    
    // Calculate angle of entry - we want shallow angles for skipping
    // The angle is measured from horizontal (0 = horizontal, 90 = straight down)
    float speed_horizontal = std::abs(vel.x);
    float speed_vertical = std::abs(vel.y);
    float entry_angle = std::atan2(speed_vertical, speed_horizontal) * 180.0f / 3.14159265f;
    
    // For skipping, we want a shallow angle (ball moving mostly horizontally)
    // entry_angle close to 0 means horizontal, close to 90 means vertical
    return entry_angle <= kSkipMaxAngle;
  }

  void Update(float dt, const WaveSystem& waves, BubbleSystem& bubbles) {
    if (!active) return;

    // Get current water surface height at ball's x position
    float water_height = kWaterBaseY + waves.GetWaveHeight(pos.x, waves.wave_time);
    
    if (!in_water) {
      // Ball is in the air - apply normal gravity
      vel.y += kGravity * dt;
      
      // Check if ball enters water
      if (pos.y >= water_height) {
        // Check for stone skipping - only if moving downward and should skip
        if (ShouldSkip() && vel.y > 0) {
          // Perform skip!
          skip_count++;
          pos.y = water_height - 2.0f;  // Position slightly above water surface
          
          // Bounce up with good energy retention
          vel.y = -std::abs(vel.y) * kSkipBounceMultiplier;
          vel.x *= kSkipVelocityDamping;  // Lose some horizontal speed
          
          hit_water = true;  // Still trigger splash effect
        } else {
          // Normal water entry
          hit_water = true;
          in_water = true;
          pos.y = water_height;
        }
      }
    } else {
      // Ball is in water - apply fluid physics
      
      // Spawn bubbles while sinking (less bubbles as ball slows down)
      float speed = vel.Length();
      if (is_moving && speed > 5.0f) {
        bubble_spawn_timer += dt;
        // Spawn rate decreases as ball slows down
        float adjusted_rate = kBubbleSpawnRate * (1.0f + (kTerminalVelocityWater - speed) / kTerminalVelocityWater);
        if (bubble_spawn_timer >= adjusted_rate) {
          bubble_spawn_timer = 0.0f;
          // Spawn fewer bubbles as speed decreases
          float spawn_chance = speed / kTerminalVelocityWater;
          if (spawn_chance > 0.1f) {
            bubbles.SpawnBubble(pos, radius);
          }
        }
      }
      
      // Buoyancy force (upward, reduces effective gravity)
      float buoyancy_accel = -kWaterBuoyancy;
      
      // Fluid drag force (proportional to velocity squared, opposes motion)
      if (speed > 0.1f) {
        Vec2 drag_dir = vel.Normalized();
        float drag_magnitude = kWaterDragCoefficient * speed * speed * 0.001f;
        vel.x -= drag_dir.x * drag_magnitude * dt;
        vel.y -= drag_dir.y * drag_magnitude * dt;
      }
      
      // Apply reduced gravity with buoyancy
      vel.y += (kGravity + buoyancy_accel) * dt;
      
      // Terminal velocity limit in water
      if (vel.y > kTerminalVelocityWater) {
        vel.y = kTerminalVelocityWater;
      }
      
      // Horizontal drag slows down horizontal movement quickly
      vel.x *= 0.98f;
    }

    // Update position
    pos = pos + vel * dt;

    // Clamp to window
    if (pos.x < 0.0f) pos.x = 0.0f;
    if (pos.x > static_cast<float>(kWindowWidth))
      pos.x = static_cast<float>(kWindowWidth);
    
    // Ball sinks to bottom and stays there
    if (pos.y >= kWindowHeight - 10.0f) {
      pos.y = kWindowHeight - 10.0f;
      vel.y = 0.0f;
      vel.x *= 0.5f;
    }
    
    // Check if ball has stopped moving
    is_moving = vel.Length() > 5.0f;
  }

  void Draw(SDL_Renderer* renderer) {
    if (!active) return;

    // Dark blue ball matching the throw origin indicator
    SDL_SetRenderDrawColor(renderer, 20, 40, 120, 255);
    DrawFilledCircle(renderer, static_cast<int>(pos.x),
                     static_cast<int>(pos.y), static_cast<int>(radius));
    
    // Add highlight for 3D effect
    SDL_SetRenderDrawColor(renderer, 60, 80, 160, 255);
    for (int dx = -2; dx <= 1; ++dx) {
      for (int dy = -2; dy <= 1; ++dy) {
        if (dx * dx + dy * dy <= 4) {
          SDL_RenderDrawPoint(renderer, static_cast<int>(pos.x) + dx - 1, 
                              static_cast<int>(pos.y) + dy - 1);
        }
      }
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
// Game State
// ============================================================================
struct GameState {
  std::vector<Rock> rocks;  // Multiple rocks for persistent balls
  SplashSystem splash;
  WaveSystem waves;
  BubbleSystem bubbles;
  float throw_speed = 300.0f;
  float throw_angle = 45.0f;
  float drag_distance = 0.0f;  // Track drag distance for trajectory visualization
  Vec2 throw_direction = {-1.0f, 0.0f};  // Normalized throw direction (opposite to drag)
  bool game_running = true;
  bool show_help = true;
  bool is_dragging = false;
  Vec2 drag_start = {static_cast<float>(kThrowOriginX), static_cast<float>(kThrowOriginY)};
  Vec2 drag_end = {static_cast<float>(kThrowOriginX), static_cast<float>(kThrowOriginY)};
  
  // Check if any rock is currently moving (prevents throwing)
  bool IsAnyRockMoving() const {
    for (const auto& rock : rocks) {
      if (rock.active && rock.is_moving) {
        return true;
      }
    }
    return false;
  }
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
  // Red arrow to match trajectory dots
  SDL_SetRenderDrawColor(renderer, 255, 80, 80, 220);
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

void DrawTrajectoryArc(SDL_Renderer* renderer, float speed, const Vec2& throw_dir, float drag_distance = 0.0f) {
  // Calculate dot size based on drag distance (more drag = bigger dots = faster throw)
  float base_dot_size = 2.0f;
  float max_dot_size = 6.0f;
  float drag_factor = Clamp(drag_distance / 200.0f, 0.0f, 1.0f);
  float dot_size = base_dot_size + (max_dot_size - base_dot_size) * drag_factor;
  
  // Red color with slight variation based on drag
  uint8_t red = static_cast<uint8_t>(200 + 55 * drag_factor);
  uint8_t green = static_cast<uint8_t>(50 + 30 * drag_factor);
  uint8_t blue = static_cast<uint8_t>(50);
  
  int num_points = 60;
  float time_step = 0.04f;  // Time between each dot
  
  for (int i = 1; i <= num_points; ++i) {
    float t = i * time_step;
    
    // Exact same physics as the ball - use throw direction directly
    float x = kThrowOriginX + throw_dir.x * speed * t;
    float y = kThrowOriginY + throw_dir.y * speed * t + 0.5f * kGravity * t * t;
    
    int screen_x = static_cast<int>(x);
    int screen_y = static_cast<int>(y);
    
    // Stop drawing if out of bounds or below water
    if (screen_x < 0 || screen_x >= kWindowWidth || screen_y >= kWindowHeight) {
      break;
    }
    if (screen_y > kWaterBaseY - 20) {
      break;
    }
    
    // Draw dot with size that decreases along trajectory
    float distance_factor = 1.0f - (i / static_cast<float>(num_points)) * 0.3f;
    int current_size = static_cast<int>(dot_size * distance_factor);
    
    SDL_SetRenderDrawColor(renderer, red, green, blue, 200);
    
    // Draw filled circle for each dot
    for (int dx = -current_size; dx <= current_size; ++dx) {
      for (int dy = -current_size; dy <= current_size; ++dy) {
        if (dx * dx + dy * dy <= current_size * current_size) {
          SDL_RenderDrawPoint(renderer, screen_x + dx, screen_y + dy);
        }
      }
    }
  }
  
  // Draw the ball at the throw origin (dark blue)
  SDL_SetRenderDrawColor(renderer, 20, 40, 120, 255);
  for (int dx = -8; dx <= 8; ++dx) {
    for (int dy = -8; dy <= 8; ++dy) {
      if (dx * dx + dy * dy <= 64) {
        SDL_RenderDrawPoint(renderer, kThrowOriginX + dx, kThrowOriginY + dy);
      }
    }
  }
  
  // Draw highlight on ball for 3D effect
  SDL_SetRenderDrawColor(renderer, 60, 80, 160, 255);
  for (int dx = -4; dx <= 0; ++dx) {
    for (int dy = -4; dy <= 0; ++dy) {
      if (dx * dx + dy * dy <= 16) {
        SDL_RenderDrawPoint(renderer, kThrowOriginX + dx - 2, kThrowOriginY + dy - 2);
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
          if (event.button.button == SDL_BUTTON_LEFT && !state.IsAnyRockMoving()) {
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
        state.rocks.clear();
        state.splash.particles.clear();
        state.bubbles.bubbles.clear();
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
    
    // Store drag distance for trajectory visualization
    state.drag_distance = distance;

    if (distance < 1.0f) {
      return;
    }

    // Direction the user is dragging
    Vec2 drag_direction = direction.Normalized();
    
    // The throw direction is OPPOSITE to the drag direction
    // If you drag left, the ball goes right. If you drag down, the ball goes up.
    state.throw_direction = {-drag_direction.x, -drag_direction.y};
    
    // Speed is proportional to drag distance - the further you drag, the faster the throw
    // This gives intuitive control: drag back more = throw harder
    state.throw_speed = Clamp(distance * 3.0f, 100.0f, 800.0f);
    
    // Update throw_angle for UI display (calculated from direction for display purposes)
    state.throw_angle = std::atan2(state.throw_direction.y, state.throw_direction.x) * 180.0f / 3.14159265f;
  }

  void ThrowRock() {
    // Prevent throwing if any ball is still moving
    if (state.IsAnyRockMoving()) {
      std::cout << "Cannot throw - a ball is still moving!\n";
      return;
    }
    
    // Create a new rock
    Rock new_rock;
    new_rock.active = true;
    new_rock.hit_water = false;
    new_rock.in_water = false;
    new_rock.is_moving = true;
    new_rock.skip_count = 0;
    new_rock.bubble_spawn_timer = 0.0f;
    new_rock.pos = {static_cast<float>(kThrowOriginX), static_cast<float>(kThrowOriginY)};
    new_rock.vel = {state.throw_direction.x * state.throw_speed,
                    state.throw_direction.y * state.throw_speed};
    
    state.rocks.push_back(new_rock);

    std::cout << "Threw rock with speed " << state.throw_speed << " at angle "
              << state.throw_angle << " degrees\n";
  }

  void PrintStatus() {
    std::cout << "Angle: " << state.throw_angle << "° | Speed: "
              << state.throw_speed << " px/s\n";
  }

  void Update(float dt) {
    state.waves.Update(dt);
    
    // Update all rocks
    for (auto& rock : state.rocks) {
      bool was_hit_water = rock.hit_water;
      rock.Update(dt, state.waves, state.bubbles);
      
      // Check if rock just hit water
      if (rock.hit_water && !was_hit_water) {
        float impact_speed = rock.vel.Length();
        state.splash.CreateSplash(rock.pos, impact_speed);
        state.waves.AddRipple(rock.pos, impact_speed);
        rock.hit_water = false;
      }
    }

    state.splash.Update(dt, state.waves);
    state.bubbles.Update(dt, state.waves);
  }

  void Render() {
    DrawBackground(renderer);
    DrawWater(renderer, state.waves);
    
    // Only show trajectory and ball at origin when no ball is moving
    if (!state.IsAnyRockMoving()) {
      DrawTrajectoryArc(renderer, state.throw_speed, state.throw_direction, state.drag_distance);
    }

    if (state.is_dragging) {
      DrawArrow(renderer, state.drag_start, state.drag_end);
    }

    // Draw all rocks (including settled ones at the bottom)
    for (auto& rock : state.rocks) {
      rock.Draw(renderer);
    }
    
    state.splash.Draw(renderer);
    state.bubbles.Draw(renderer);
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
