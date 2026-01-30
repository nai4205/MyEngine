#pragma once

#include "../../ecs/World.hpp"
#include "components/BallComponent.hpp"
#include "components/BrickComponent.hpp"
#include "components/Collider2D.hpp"
#include "systems/CollisionSystem2D.hpp"
#include <glm/glm.hpp>

extern World gWorld;

namespace CollisionHandlers {

inline void onBallBrick(Entity ball, Entity brick, const glm::vec2 &normal) {
  auto *ballComp = gWorld.getComponent<BallComponent>(ball);
  auto *brickComp = gWorld.getComponent<BrickComponent>(brick);

  if (!ballComp || !brickComp || brickComp->destroyed)
    return;

  if (!brickComp->isSolid) {
    brickComp->destroyed = true;
  }

  if (glm::length(normal) > 0.0f) {
    ballComp->velocity =
        glm::reflect(ballComp->velocity, -glm::normalize(normal));
  }
}

inline void onBallPlayer(Entity ball, Entity player, const glm::vec2 &normal) {
  auto *ballComp = gWorld.getComponent<BallComponent>(ball);
  if (!ballComp)
    return;
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
