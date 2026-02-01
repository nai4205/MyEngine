#pragma once
#include <glm/glm.hpp>

enum class PowerUpType {
  Speed,        // Increases ball speed
  Sticky,       // Ball sticks to paddle
  PassThrough,  // Ball passes through bricks
  PadSizeInc,   // Increases paddle size
  Confuse,      // Confuse effect (negative)
  Chaos         // Chaos effect (negative)
};

struct PowerUpComponent {
  PowerUpType type;
  glm::vec3 color;
  float duration;      // 0 = instant effect, >0 = timed effect
  bool activated = false;
  bool destroyed = false;
};
