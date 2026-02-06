#pragma once

#include "../components/CameraComponent.hpp"
#include "../components/CameraControllerComponent.hpp"
#include "../components/PhysicsComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/World.hpp"

extern World gWorld;
extern GLFWwindow *window;

class CameraControllerSystem : public System {
private:
  bool firstMouse = true;
  float lastX = 400.0f;
  float lastY = 300.0f;

  bool spaceWasPressed = false;

public:
  CameraControllerSystem() {}

  void update(float &deltaTime) override {
    Input &input = gWorld.getInput();

    gWorld.forEachWith<TransformComponent, CameraComponent,
                       CameraControllerComponent>(
        [&](Entity entity, TransformComponent &transform,
            CameraComponent &camera, CameraControllerComponent &controller) {
          auto *physics =
              gWorld.getComponent<PhysicsComponent>(entity); // Optional

          processKeyboard(input, transform, camera, controller, physics,
                          deltaTime);

          processMouseMovement(camera, controller);

          processMouseScroll(camera, controller);

          processJump(input, controller, physics);
        });
  }

private:
  void processKeyboard(Input &input, TransformComponent &transform,
                       CameraComponent &camera,
                       CameraControllerComponent &controller,
                       PhysicsComponent *physics, float deltaTime) {
    if (input.isKeyPressed(GLFW_KEY_W)) {
      controller.processKeyboard(CAM_FORWARD, transform, camera, physics,
                                 deltaTime);
    }
    if (input.isKeyPressed(GLFW_KEY_S)) {
      controller.processKeyboard(CAM_BACKWARD, transform, camera, physics,
                                 deltaTime);
    }
    if (input.isKeyPressed(GLFW_KEY_A)) {
      controller.processKeyboard(CAM_LEFT, transform, camera, physics,
                                 deltaTime);
    }
    if (input.isKeyPressed(GLFW_KEY_D)) {
      controller.processKeyboard(CAM_RIGHT, transform, camera, physics,
                                 deltaTime);
    }
  }

  void processMouseMovement(CameraComponent &camera,
                            CameraControllerComponent &controller) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse) {
      lastX = static_cast<float>(xpos);
      lastY = static_cast<float>(ypos);
      firstMouse = false;
    }

    float xOffset = static_cast<float>(xpos) - lastX;
    float yOffset = lastY - static_cast<float>(ypos);

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    controller.processMouseMovement(camera, xOffset, yOffset);
  }

  void processMouseScroll(CameraComponent &camera,
                          CameraControllerComponent &controller) {}

  void processJump(Input &input, CameraControllerComponent &controller,
                   PhysicsComponent *physics) {
    bool spacePressed = input.isKeyPressed(GLFW_KEY_SPACE);

    if (spacePressed) {
      controller.processJump(physics);
    }

    spaceWasPressed = spacePressed;
  }
};
