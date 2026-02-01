#pragma once
#include "../../gl_common.hpp"
#include <cstdint>
#include <vector>

struct Particle {
  glm::vec2 position;
  glm::vec2 velocity;
  glm::vec4 color;
  float life = 0.0f;
};

struct ParticleEmitterComponent {
  // Pool settings
  unsigned int maxParticles = 500;
  unsigned int spawnRate = 2;
  glm::vec2 offset = glm::vec2(0.0f);
  glm::vec2 particleSize = glm::vec2(10.0f);

  // Particle behavior
  float particleLifetime = 1.0f;
  glm::vec2 gravity = glm::vec2(0.0f);
  bool trailMode =
      true; // true: particles trail behind, false: particles move outward

  // Emitter duration (0 = infinite, >0 = stops after this many seconds)
  float duration = 0.0f;
  float durationRemaining = 0.0f;

  // Shared rendering resources for particles
  uint32_t shaderID = 0;
  uint32_t textureID = 0;
  uint32_t meshVAO = 0;
  unsigned int meshVertexCount = 6;

  // Per-emitter particle pool
  std::vector<Particle> particles;
  unsigned int lastUsedParticle = 0;
};
