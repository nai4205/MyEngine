#pragma once
#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../components/BallComponent.hpp"
#include "../components/BrickComponent.hpp"
#include "CollisionSystem2D.hpp"

extern World gWorld;

class BrickCollisionHandler : public System {
public:
  BrickCollisionHandler(CollisionSystem2D *collisionSystem)
      : collisionSystem(collisionSystem) {}

  void update(float &deltaTime) override {
    for (const auto &event : collisionSystem->events) {
      handleBallBrickCollision(event);
    }
  }

private:
  CollisionSystem2D *collisionSystem;

  void handleBallBrickCollision(const CollisionEvent &event) {
    BallComponent *ballA = gWorld.getComponent<BallComponent>(event.entityA);
    BrickComponent *brickB = gWorld.getComponent<BrickComponent>(event.entityB);

    if (ballA && brickB) {
      handleCollision(ballA, brickB, event.normal);
      return;
    }

    BallComponent *ballB = gWorld.getComponent<BallComponent>(event.entityB);
    BrickComponent *brickA = gWorld.getComponent<BrickComponent>(event.entityA);

    if (ballB && brickA) {
      handleCollision(ballB, brickA, -event.normal);
      return;
    }
  }

  void handleCollision(BallComponent *ball, BrickComponent *brick,
                       glm::vec2 normal) {
    if (brick->destroyed)
      return;

    if (!brick->isSolid) {
      brick->destroyed = true;
    }

    if (glm::length(normal) > 0.0f) {
      ball->velocity = glm::reflect(ball->velocity, -glm::normalize(normal));
    }
  }
};
