#pragma once

#include "../../components/TransformComponent.hpp"
#include "../../ecs/World.hpp"
#include "../breakout/components/Collider2D.hpp"
#include "components/VelocityComponent.hpp"

extern World gWorld;

namespace WaterSimCollision {

constexpr float WALL_DAMPENING = 0.8f;

inline void onParticleWall(Entity particle, Entity wall,
                           const glm::vec2 &normal, float penetration) {
  auto *vel = gWorld.getComponent<WaterVelocityComponent>(particle);
  auto *transform = gWorld.getComponent<TransformComponent>(particle);
  if (!vel || !transform)
    return;

  transform->position.x += normal.x * penetration;
  transform->position.y += normal.y * penetration;

  float dotProduct = glm::dot(vel->velocity, normal);
  if (dotProduct < 0) { // Only reflect if moving into the wall
    vel->velocity -= 2.0f * dotProduct * normal;
    vel->velocity *= WALL_DAMPENING;
  }
}

} // namespace WaterSimCollision
