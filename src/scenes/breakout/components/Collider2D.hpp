#pragma once
#include "../../gl_common.hpp"
#include <cstdint>

enum class ColliderType { AABB, Circle };

enum class CollisionLayer : uint32_t {
  None = 0,
  Ball = 1 << 0,
  Brick = 1 << 1,
  Player = 1 << 2,
  Wall = 1 << 3,
};

// Bitwise operators for CollisionLayer
inline uint32_t operator|(CollisionLayer a, CollisionLayer b) {
  return static_cast<uint32_t>(a) | static_cast<uint32_t>(b);
}

inline uint32_t operator|(uint32_t a, CollisionLayer b) {
  return a | static_cast<uint32_t>(b);
}

inline uint32_t operator&(uint32_t mask, CollisionLayer layer) {
  return mask & static_cast<uint32_t>(layer);
}

struct Collider2D {
  ColliderType type;

  glm::vec2 halfExtents;
  float radius;
  CollisionLayer layer = CollisionLayer::None;
  uint32_t collidesWith = 0;

  static Collider2D makeAABB(glm::vec2 size, CollisionLayer layer,
                             uint32_t collidesWith) {
    Collider2D c;
    c.type = ColliderType::AABB;
    c.halfExtents = size * 0.5f;
    c.radius = 0;
    c.layer = layer;
    c.collidesWith = collidesWith;
    return c;
  }

  static Collider2D makeCircle(float r, CollisionLayer layer,
                               uint32_t collidesWith) {
    Collider2D c;
    c.type = ColliderType::Circle;
    c.radius = r;
    c.halfExtents = glm::vec2(0);
    c.layer = layer;
    c.collidesWith = collidesWith;
    return c;
  }
};
