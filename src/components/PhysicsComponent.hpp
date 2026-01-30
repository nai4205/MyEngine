#pragma once

#include <glm/glm.hpp>

class PhysicsComponent {
public:
  glm::vec3 velocity;
  float gravity;
  float groundLevel;
  bool isGrounded;

  PhysicsComponent(float gravityForce = -9.81f, float ground = 0.0f)
      : velocity(0.0f), gravity(gravityForce), groundLevel(ground),
        isGrounded(false) {} // Objects start in air and fall

  void update(glm::vec3 &position, float deltaTime) {
    if (!isGrounded) {
      velocity.y += gravity * deltaTime;
      position += velocity * deltaTime;

      if (position.y <= groundLevel) {
        position.y = groundLevel;
        velocity.y = 0.0f;
        isGrounded = true;
      }
    }
  }

  void jump(float jumpForce = 5.0f) {
    if (isGrounded) {
      velocity.y = jumpForce;
      isGrounded = false;
    }
  }

  void applyImpulse(const glm::vec3 &impulse) {
    velocity += impulse;
    if (glm::length(impulse) > 0.0f) {
      isGrounded = false;
    }
  }

  void resetToGround(glm::vec3 &position) {
    position.y = groundLevel;
    velocity = glm::vec3(0.0f);
    isGrounded = true;
  }
};
