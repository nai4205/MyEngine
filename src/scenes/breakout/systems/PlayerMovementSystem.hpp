#pragma once
#include "../../../components/TransformComponent.hpp"
#include "../../../ecs/Input.hpp"
#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../components/PlayerComponent.hpp"

extern World gWorld;

class PlayerMovementSystem : public System {
private:
  float screenWidth;

public:
  PlayerMovementSystem(float width) : screenWidth(width) {}

  void update(float &deltaTime) override {
    Input &input = gWorld.getInput();
    gWorld.forEachWith<TransformComponent, PlayerComponent>(
        [&](Entity entity, TransformComponent &transform,
            PlayerComponent &player) {
          float velocity = player.velocity * deltaTime;
          if (input.isKeyPressed(GLFW_KEY_A)) {
            if (transform.position.x >= 0.0f) {
              transform.position.x -= velocity;
            }
          }
          if (input.isKeyPressed(GLFW_KEY_D)) {
            if (transform.position.x <= screenWidth - player.sizeX) {
              transform.position.x += velocity;
            }
          }
        });
  }
};
