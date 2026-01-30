#pragma once
#include "../../gl_common.hpp"

struct BallComponent {
  float radius;
  glm::vec2 velocity;
  bool stuck;
};
