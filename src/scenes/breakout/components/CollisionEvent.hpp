#pragma once
#include "../../../ecs/World.hpp"
#include "../../gl_common.hpp"

struct CollisionEvent {
  Entity entityA;
  Entity entityB;
  glm::vec2 normal;
  float penetration;
};
