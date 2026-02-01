#pragma once
#include "../../../components/TransformComponent.hpp"
#include "../../../ecs/Input.hpp"
#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../components/BallComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include <GLFW/glfw3.h>

extern World gWorld;

class PlayerMovementSystem : public System {
private:
  float screenWidth;

public:
  PlayerMovementSystem(float width) : screenWidth(width) {}

  void update(float &deltaTime) override {
    Input &input = gWorld.getInput();
    glm::vec3 *ballPos = nullptr;
    glm::vec2 *ballVelocity = nullptr;
    gWorld.forEachWith<BallComponent, TransformComponent, VelocityComponent>(
        [&](Entity entity, BallComponent &ball, TransformComponent &transform,
            VelocityComponent &vel) {
          ballPos = &transform.position;
          ballVelocity = &vel.velocity;
        });
    gWorld.forEachWith<TransformComponent, PlayerComponent>(
        [&](Entity entity, TransformComponent &transform,
            PlayerComponent &player) {
          float movement = player.speed * deltaTime;
          if (input.isKeyPressed(GLFW_KEY_A)) {
            if (transform.position.x >= 0.0f) {
              transform.position.x -= movement;
            }
          }
          if (input.isKeyPressed(GLFW_KEY_D)) {
            if (transform.position.x <= screenWidth - player.sizeX) {
              transform.position.x += movement;
            }
          }
          if (input.isKeyPressed(GLFW_KEY_SPACE) && ballPos && ballVelocity) {
            ballPos->x = screenWidth / 2;
            ballPos->y = 300;
            *ballVelocity = glm::vec2(0.0f, abs(ballVelocity->y));
          }
        });
  }
};
