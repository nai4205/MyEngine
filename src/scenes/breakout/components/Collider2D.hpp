#pragma once
#include "../../gl_common.hpp"

enum class ColliderType { AABB, Circle };

struct Collider2D {
  ColliderType type;

  glm::vec2 halfExtents;
  float radius;

  static Collider2D makeAABB(glm::vec2 size) {
    Collider2D c;
    c.type = ColliderType::AABB;
    c.halfExtents = size * 0.5f;
    c.radius = 0;
    return c;
  }

  static Collider2D makeCircle(float r) {
    Collider2D c;
    c.type = ColliderType::Circle;
    c.radius = r;
    c.halfExtents = glm::vec2(0);
    return c;
  }
};
