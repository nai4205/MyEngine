#pragma once

#include "PhysicsComponent.hpp"
#include "TransformComponent.hpp"
#include <glm/glm.hpp>

// Player movement directions
enum PlayerMovement {
  PLAYER_LEFT,
  PLAYER_RIGHT,
  PLAYER_FORWARD,
  PLAYER_BACKWARD
};

class PlayerControllerComponent2D {
public:
  float movementSpeed;
  float jumpForce;
  bool canMoveVertically; // Allow forward/backward movement

  PlayerControllerComponent2D(float speed = 5.0f, float jump = 8.0f,
                              bool allowVertical = false)
      : movementSpeed(speed), jumpForce(jump),
        canMoveVertically(allowVertical) {}

  void processMovement(PlayerMovement direction, TransformComponent &transform,
                       float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    glm::vec3 moveDirection(0.0f);

    // For 2.5D platformer: left/right moves along Z-axis
    if (direction == PLAYER_LEFT)
      moveDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    else if (direction == PLAYER_RIGHT)
      moveDirection = glm::vec3(0.0f, 0.0f, 1.0f);
    else if (direction == PLAYER_FORWARD && canMoveVertically)
      moveDirection = glm::vec3(1.0f, 0.0f, 0.0f);
    else if (direction == PLAYER_BACKWARD && canMoveVertically)
      moveDirection = glm::vec3(-1.0f, 0.0f, 0.0f);

    transform.position += moveDirection * velocity;
  }

  void processJump(PhysicsComponent *physics) {
    if (physics) {
      physics->jump(jumpForce);
    }
  }
};
