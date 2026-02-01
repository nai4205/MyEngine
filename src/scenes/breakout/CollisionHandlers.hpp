#pragma once

#include "../../components/TransformComponent.hpp"
#include "../../ecs/World.hpp"
#include "components/BrickComponent.hpp"
#include "components/Collider2D.hpp"
#include "components/ParticleEmitterComponent.hpp"
#include "components/PostProcessingComponent.hpp"
#include "components/PowerUpComponent.hpp"
#include "components/VelocityComponent.hpp"
#include "systems/CollisionSystem2D.hpp"
#include "systems/PowerUpSystem.hpp"
#include <glm/glm.hpp>

extern World gWorld;

namespace CollisionHandlers {

inline void onBallBrick(Entity ball, Entity brick, const glm::vec2 &normal,
                        float penetration) {
  auto *ballVel = gWorld.getComponent<VelocityComponent>(ball);
  auto *brickComp = gWorld.getComponent<BrickComponent>(brick);
  auto *ballTransform = gWorld.getComponent<TransformComponent>(ball);
  auto *brickTransform = gWorld.getComponent<TransformComponent>(brick);

  if (!ballVel || !brickComp || !ballTransform || brickComp->destroyed)
    return;

  if (!brickComp->isSolid) {
    brickComp->destroyed = true;

    // Activate the dormant particle emitter for a short burst
    auto *emitter = gWorld.getComponent<ParticleEmitterComponent>(brick);
    if (emitter) {
      emitter->spawnRate = 10;
      emitter->durationRemaining = 0.3f;
    }

    // Try to spawn a powerup at brick position
    if (brickTransform) {
      if (auto *powerUpSystem = gWorld.getSystem<PowerUpSystem>()) {
        glm::vec2 brickPos(brickTransform->position.x,
                           brickTransform->position.y);
        powerUpSystem->trySpawnPowerUp(brickPos);
      }
    }

  } else {
    // Shake screen when hitting solid brick
    gWorld.forEachWith<PostProcessingComponent>(
        [](Entity e, PostProcessingComponent &fx) {
          fx.shake = true;
          fx.shakeTime = 0.2f;
        });
  }

  if (glm::length(normal) > 0.0f) {
    glm::vec2 n = glm::normalize(normal);
    ballTransform->position += glm::vec3(n * penetration, 0.0f);
    ballVel->velocity = glm::reflect(ballVel->velocity, -n);
  }
}

inline void onBallPlayer(Entity ball, Entity player, const glm::vec2 &normal,
                         float penetration) {
  auto *ballVel = gWorld.getComponent<VelocityComponent>(ball);
  auto *ballTransform = gWorld.getComponent<TransformComponent>(ball);

  if (!ballVel || !ballTransform)
    return;

  if (penetration > 0.0f && glm::length(normal) > 0.0f) {
    glm::vec2 n = glm::normalize(normal);
    ballTransform->position += glm::vec3(n * penetration, 0.0f);
  }

  ballVel->velocity.y = -std::abs(ballVel->velocity.y);
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
