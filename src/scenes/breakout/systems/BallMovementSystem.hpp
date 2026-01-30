#pragma once
#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../../components/TransformComponent.hpp"
#include "../components/BallComponent.hpp"

extern World gWorld;

class BallMovementSystem : public System {

public:
  BallMovementSystem(float width = 800.0f) : screenWidth(width) {}
  void update(float &deltaTime) override {
    gWorld.forEachWith<BallComponent, TransformComponent>(
        [&](Entity entity, BallComponent &ball, TransformComponent &transform) {
          glm::vec2 size = glm::vec2(ball.radius * 2.0f, ball.radius * 2.0f);
          if (!ball.stuck) {
            transform.position += glm::vec3(ball.velocity * deltaTime, 0.0f);
            if (transform.position.x <= 0.0f) {
              ball.velocity.x = -ball.velocity.x;
              transform.position.x = 0.0f;
            } else if (transform.position.x + size.x >= screenWidth) {
              ball.velocity.x = -ball.velocity.x;
              transform.position.x = screenWidth - size.x;
            }
            if (transform.position.y <= 0.0f) {
              ball.velocity.y = -ball.velocity.y;
              transform.position.y = 0.0f;
            }
          }
        });
  }

private:
  float screenWidth;
};
