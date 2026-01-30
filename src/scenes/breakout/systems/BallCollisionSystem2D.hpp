#pragma once
#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../../components/TransformComponent.hpp"
#include "../components/BallComponent.hpp"
#include "../components/BrickComponent.hpp"

extern World gWorld;

class BallCollisionSystem2D : public System {
public:
  void update(float &deltaTime) override {
    gWorld.forEachWith<BallComponent, TransformComponent>(
        [&](Entity ballEntity, BallComponent &ball,
            TransformComponent &ballTransform) {
          glm::vec2 ballPos = glm::vec2(ballTransform.position);
          glm::vec2 ballSize =
              glm::vec2(ball.radius * 2.0f, ball.radius * 2.0f);

          gWorld.forEachWith<BrickComponent, TransformComponent>(
              [&](Entity brickEntity, BrickComponent &brick,
                  TransformComponent &brickTransform) {
                if (brick.destroyed)
                  return;

                glm::vec2 brickPos = glm::vec2(brickTransform.position);
                glm::vec2 brickSize = glm::vec2(brickTransform.scale);

                if (checkAABBCollision(ballPos, ballSize, brickPos,
                                       brickSize)) {
                  if (!brick.isSolid) {
                    brick.destroyed = true;
                  }
                }
              });
        });
  }

private:
  bool checkAABBCollision(glm::vec2 posA, glm::vec2 sizeA, glm::vec2 posB,
                          glm::vec2 sizeB) {
    bool collisionX = posA.x + sizeA.x >= posB.x && posB.x + sizeB.x >= posA.x;
    bool collisionY = posA.y + sizeA.y >= posB.y && posB.y + sizeB.y >= posA.y;
    return collisionX && collisionY;
  }
};
