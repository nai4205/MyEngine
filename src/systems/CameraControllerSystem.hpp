#ifndef CAMERA_CONTROLLER_SYSTEM_HPP
#define CAMERA_CONTROLLER_SYSTEM_HPP

#include "../components/CameraComponent.hpp"
#include "../components/CameraController.hpp"
#include "../components/Physics.hpp"
#include "../components/Transform.hpp"
#include "../ecs/System.hpp"
#include "../ecs/World.hpp"
#include <GLFW/glfw3.h>

extern World gWorld;

// System that processes input for camera control
class CameraControllerSystem : public System {
private:
  GLFWwindow *window;

  // Mouse state (handled directly with GLFW)
  bool firstMouse = true;
  float lastX = 400.0f;
  float lastY = 300.0f;

  // Jump button state (to prevent repeated jumps)
  bool spaceWasPressed = false;

public:
  CameraControllerSystem(GLFWwindow *win) : window(win) {}

  void update(float &deltaTime) override {
    Input &input = gWorld.getInput();

    // Query for all entities with camera controller components
    auto entities =
        gWorld.getEntitiesWith<Transform, Camera, CameraController>();

    for (Entity entity : entities) {
      auto *transform = gWorld.getComponent<Transform>(entity);
      auto *camera = gWorld.getComponent<Camera>(entity);
      auto *controller = gWorld.getComponent<CameraController>(entity);
      auto *physics = gWorld.getComponent<Physics>(entity); // Optional

      if (transform && camera && controller) {
        // Process keyboard input (WASD) - using Input system
        processKeyboard(input, *transform, *camera, *controller, physics,
                        deltaTime);

        // Process mouse input (look around) - using direct GLFW
        processMouseMovement(*camera, *controller);

        // Process scroll input (zoom) - using direct GLFW
        processMouseScroll(*camera, *controller);

        // Process jump (space) - using Input system
        processJump(input, *controller, physics);
      }
    }
  }

private:
  void processKeyboard(Input &input, Transform &transform, Camera &camera,
                       CameraController &controller, Physics *physics,
                       float deltaTime) {
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

  void processMouseMovement(Camera &camera, CameraController &controller) {
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

  void processMouseScroll(Camera &camera, CameraController &controller) {
    // Placeholder - scroll handled via callback if needed
  }

  void processJump(Input &input, CameraController &controller,
                   Physics *physics) {
    bool spacePressed = input.isKeyPressed(GLFW_KEY_SPACE);

    if (spacePressed && !spaceWasPressed) {
      controller.processJump(physics);
    }

    spaceWasPressed = spacePressed;
  }
};

#endif // CAMERA_CONTROLLER_SYSTEM_HPP
