#pragma once

#include "../components/PhysicsComponent.hpp"
#include "../components/PlayerControllerComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/World.hpp"
#include "../gl_common.hpp"

extern World gWorld;

class PlayerControllerSystem : public System {
private:
  bool spaceWasPressed = false;

public:
  PlayerControllerSystem() = default;

  void update(float &deltaTime) override {
    Input &input = gWorld.getInput();

    gWorld.forEachWith<TransformComponent, PlayerControllerComponent2D>(
        [&](Entity entity, TransformComponent &transform,
            PlayerControllerComponent2D &controller) {
          auto *physics =
              gWorld.getComponent<PhysicsComponent>(entity); // Optional

          processKeyboard(input, transform, controller, deltaTime);
          processJump(input, controller, physics);
        });
  }

private:
  void processKeyboard(Input &input, TransformComponent &transform,
                       PlayerControllerComponent2D &controller,
                       float deltaTime) {
    // Arrow keys or A/D for left/right movement
    if (input.isKeyPressed(GLFW_KEY_LEFT) || input.isKeyPressed(GLFW_KEY_A)) {
      controller.processMovement(PLAYER_LEFT, transform, deltaTime);
    }
    if (input.isKeyPressed(GLFW_KEY_RIGHT) || input.isKeyPressed(GLFW_KEY_D)) {
      controller.processMovement(PLAYER_RIGHT, transform, deltaTime);
    }

    // W/S for forward/backward (if enabled)
    if (input.isKeyPressed(GLFW_KEY_W)) {
      controller.processMovement(PLAYER_FORWARD, transform, deltaTime);
    }
    if (input.isKeyPressed(GLFW_KEY_S)) {
      controller.processMovement(PLAYER_BACKWARD, transform, deltaTime);
    }
  }

  void processJump(Input &input, PlayerControllerComponent2D &controller,
                   PhysicsComponent *physics) {
    bool spacePressed = input.isKeyPressed(GLFW_KEY_SPACE);

    // Only jump on initial press (not while held)
    if (spacePressed && !spaceWasPressed) {
      controller.processJump(physics);
    }

    spaceWasPressed = spacePressed;
  }
};
