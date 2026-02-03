#pragma once

#include "../../components/TransformComponent.hpp"
#include "../../ecs/World.hpp"
#include "components/BallComponent.hpp"
#include "components/BrickComponent.hpp"
#include "components/Collider2D.hpp"
#include "components/ParticleEmitterComponent.hpp"
#include "components/PlayerComponent.hpp"
#include "components/PostProcessingComponent.hpp"
#include "components/PowerUpComponent.hpp"
#include "components/VelocityComponent.hpp"
#include "systems/CollisionSystem2D.hpp"
#include "systems/PowerUpSystem.hpp"
#include <glm/glm.hpp>

extern World gWorld;

namespace CollisionHandlers {

enum Direction { UP, RIGHT, DOWN, LEFT };

// Determine collision direction using dot product with compass vectors
inline Direction vectorDirection(const glm::vec2 &target) {
  glm::vec2 compass[] = {
      glm::vec2(0.0f, -1.0f), // UP (negative Y in screen coords)
      glm::vec2(1.0f, 0.0f),  // RIGHT
      glm::vec2(0.0f, 1.0f),  // DOWN
      glm::vec2(-1.0f, 0.0f)  // LEFT
  };
  float maxDot = 0.0f;
  unsigned int bestMatch = 0;
  glm::vec2 normalized = glm::normalize(target);
  for (unsigned int i = 0; i < 4; i++) {
    float dot = glm::dot(normalized, compass[i]);
    if (dot > maxDot) {
      maxDot = dot;
      bestMatch = i;
    }
  }
  return static_cast<Direction>(bestMatch);
}

inline void onBallBrick(Entity ball, Entity brick, const glm::vec2 &normal,
                        float penetration) {
  auto *ballVel = gWorld.getComponent<VelocityComponent>(ball);
  auto *brickComp = gWorld.getComponent<BrickComponent>(brick);
  auto *ballTransform = gWorld.getComponent<TransformComponent>(ball);
  auto *brickTransform = gWorld.getComponent<TransformComponent>(brick);
  auto *ballComp = gWorld.getComponent<BallComponent>(ball);

  if (!ballVel || !brickComp || !ballTransform || !ballComp ||
      brickComp->destroyed)
    return;

  auto *powerUpSystem = gWorld.getSystem<PowerUpSystem>();
  bool passThrough =
      powerUpSystem && powerUpSystem->isPowerUpActive(PowerUpType::PassThrough);

  if (!brickComp->isSolid) {
    brickComp->destroyed = true;

    // Activate the dormant particle emitter for a short burst
    auto *emitter = gWorld.getComponent<ParticleEmitterComponent>(brick);
    if (emitter) {
      emitter->spawnRate = 10;
      emitter->durationRemaining = 0.3f;
    }

    // Try to spawn a powerup at brick position
    if (brickTransform && powerUpSystem) {
      glm::vec2 brickPos(brickTransform->position.x,
                         brickTransform->position.y);
      powerUpSystem->trySpawnPowerUp(brickPos);
    }

  } else {
    // Shake screen when hitting solid brick
    gWorld.forEachWith<PostProcessingComponent>(
        [](Entity e, PostProcessingComponent &fx) {
          fx.shake = true;
          fx.shakeTime = 0.2f;
        });
  }

  if (passThrough)
    return;
  if (glm::length(normal) > 0.0f) {
    Direction dir = vectorDirection(normal);

    if (dir == LEFT || dir == RIGHT) {
      // Horizontal collision - reverse x velocity
      ballVel->velocity.x = -ballVel->velocity.x;
      // Relocate ball outside brick
      if (dir == LEFT)
        ballTransform->position.x -= penetration;
      else
        ballTransform->position.x += penetration;
    } else {
      // Vertical collision - reverse y velocity
      ballVel->velocity.y = -ballVel->velocity.y;
      // Relocate ball outside brick
      if (dir == UP)
        ballTransform->position.y -= penetration;
      else
        ballTransform->position.y += penetration;
    }
  }
}

inline void onBallPlayer(Entity ball, Entity player, const glm::vec2 &normal,
                         float penetration) {
  auto *ballVel = gWorld.getComponent<VelocityComponent>(ball);
  auto *ballTransform = gWorld.getComponent<TransformComponent>(ball);
  auto *ballComp = gWorld.getComponent<BallComponent>(ball);
  auto *playerTransform = gWorld.getComponent<TransformComponent>(player);
  auto *playerComp = gWorld.getComponent<PlayerComponent>(player);

  if (!ballVel || !ballTransform || !ballComp || !playerTransform ||
      !playerComp)
    return;

  // Don't process if ball is stuck to paddle
  if (ballComp->stuck)
    return;

  // Reposition ball above paddle
  if (penetration > 0.0f && glm::length(normal) > 0.0f) {
    glm::vec2 n = glm::normalize(normal);
    ballTransform->position += glm::vec3(n * penetration, 0.0f);
  }

  // Calculate where ball hit the paddle relative to center
  float centerPaddle = playerTransform->position.x + playerComp->sizeX / 2.0f;
  float ballCenter = ballTransform->position.x + ballComp->radius;
  float distance = ballCenter - centerPaddle;
  float percentage = distance / (playerComp->sizeX / 2.0f);

  // Update velocity based on hit position
  float strength = 2.0f;
  glm::vec2 oldVelocity = ballVel->velocity;
  ballVel->velocity.x = glm::length(oldVelocity) * percentage * strength;
  ballVel->velocity.y = -std::abs(ballVel->velocity.y);
  ballVel->velocity =
      glm::normalize(ballVel->velocity) * glm::length(oldVelocity);
}

inline void onPowerUpPlayer(Entity powerup, Entity player,
                            const glm::vec2 &normal, float penetration) {
  auto *powerUpComp = gWorld.getComponent<PowerUpComponent>(powerup);
  if (!powerUpComp || powerUpComp->destroyed)
    return;

  if (auto *powerUpSystem = gWorld.getSystem<PowerUpSystem>()) {
    powerUpSystem->activatePowerUp(powerup);
  }
}

inline void registerAll(CollisionSystem2D *collision) {
  collision->onCollision(CollisionLayer::Ball, CollisionLayer::Brick,
                         onBallBrick);
  collision->onCollision(CollisionLayer::Ball, CollisionLayer::Player,
                         onBallPlayer);
  collision->onCollision(CollisionLayer::PowerUp, CollisionLayer::Player,
                         onPowerUpPlayer);
}

} // namespace CollisionHandlers
