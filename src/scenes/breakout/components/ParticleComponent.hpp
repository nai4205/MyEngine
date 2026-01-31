#pragma once
#include "../../gl_common.hpp"

struct ParticleComponent {
  glm::vec2 Position, Velocity;
  glm::vec4 Color;
  float Life;
};
