#pragma once

#include "../../components/TransformComponent.hpp"
#include "../../ecs/World.hpp"
#include "components/BallComponent.hpp"
#include "components/BrickComponent.hpp"
#include "components/Collider2D.hpp"
#include "systems/CollisionSystem2D.hpp"
#include <glm/glm.hpp>

extern World gWorld;

namespace CollisionHandlers {

// Inline since they are callback functions
inline void onBallBrick(Entity ball, Entity brick, const glm::vec2 &normal,
                        float penetration) {
  auto *ballComp = gWorld.getComponent<BallComponent>(ball);
  auto *brickComp = gWorld.getComponent<BrickComponent>(brick);
  auto *ballTransform = gWorld.getComponent<TransformComponent>(ball);

  if (!ballComp || !brickComp || !ballTransform || brickComp->destroyed)
    return;

  if (!brickComp->isSolid) {
    brickComp->destroyed = true;
  }

  if (glm::length(normal) > 0.0f) {
    glm::vec2 n = glm::normalize(normal);

    // Push ball out of collision
    // essentially scale the magnitude of the normal by the amount the ball
    // is inside the collision area
    ballTransform->position += glm::vec3(n * penetration, 0.0f);

    // Reflect velocity
    ballComp->velocity = glm::reflect(ballComp->velocity, -n);
  }
}

inline void onBallPlayer(Entity ball, Entity player, const glm::vec2 &normal,
                         float penetration) {
  auto *ballComp = gWorld.getComponent<BallComponent>(ball);
  auto *ballTransform = gWorld.getComponent<TransformComponent>(ball);

  if (!ballComp || !ballTransform)
    return;

  // Push ball out of paddle
  if (penetration > 0.0f && glm::length(normal) > 0.0f) {
    glm::vec2 n = glm::normalize(normal);
    ballTransform->position += glm::vec3(n * penetration, 0.0f);
  }

  ballComp->velocity.y = -std::abs(ballComp->velocity.y);
}

inline void registerAll(CollisionSystem2D *collision) {
  // Order holds because of the layers
  collision->onCollision(CollisionLayer::Ball, CollisionLayer::Brick,
                         onBallBrick);
  collision->onCollision(CollisionLayer::Ball, CollisionLayer::Player,
                         onBallPlayer);
}

} // namespace CollisionHandlers
