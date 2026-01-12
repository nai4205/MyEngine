#pragma once

#include "CameraComponent.hpp"
#include "PhysicsComponent.hpp"
#include "TransformComponent.hpp"
#include <glm/glm.hpp>

enum CameraMovement { CAM_FORWARD, CAM_BACKWARD, CAM_LEFT, CAM_RIGHT };

class CameraControllerComponent {
public:
  float movementSpeed;
  float mouseSensitivity;
  float jumpForce;
  bool freeMode;

  CameraControllerComponent(float speed = 2.5f, float sensitivity = 0.1f,
                            float jump = 5.0f, bool freeFly = false)
      : movementSpeed(speed), mouseSensitivity(sensitivity), jumpForce(jump),
        freeMode(freeFly) {}

  void processKeyboard(CameraMovement direction, TransformComponent &transform,
                       CameraComponent &camera, PhysicsComponent *physics,
                       float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    glm::vec3 moveDirection(0.0f);

    // Free mode: use full camera.front (includes vertical component)
    // FPS mode: use camera.getFlatFront() (horizontal only)
    if (direction == CAM_FORWARD)
      moveDirection = freeMode ? camera.front : camera.getFlatFront();
    else if (direction == CAM_BACKWARD)
      moveDirection = freeMode ? -camera.front : -camera.getFlatFront();
    else if (direction == CAM_LEFT)
      moveDirection = -camera.right;
    else if (direction == CAM_RIGHT)
      moveDirection = camera.right;

    transform.position += moveDirection * velocity;
  }

  void processMouseMovement(CameraComponent &camera, float xOffset,
                            float yOffset) {
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    camera.rotate(xOffset, yOffset);
  }

  void processMouseScroll(CameraComponent &camera, float yOffset) {
    camera.adjustZoom(yOffset);
  }

  void processJump(PhysicsComponent *physics) {
    if (physics) {
      physics->jump(jumpForce);
    }
  }
};
