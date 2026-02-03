#pragma once

#include <glm/glm.hpp>

enum class CameraFollowMode { INSTANT, SMOOTH };

struct CameraFollowComponent {
  glm::vec3 offset{0.0f};
  float smoothSpeed = 5.0f;
  CameraFollowMode mode = CameraFollowMode::SMOOTH;

  bool followX = false;
  bool followY = false;
  bool followZ = true;

  CameraFollowComponent() = default;

  CameraFollowComponent(glm::vec3 offsetFromTarget, float speed = 5.0f,
                        CameraFollowMode followMode = CameraFollowMode::SMOOTH)
      : offset(offsetFromTarget), smoothSpeed(speed), mode(followMode) {}

  void setFollowAxes(bool x, bool y, bool z) {
    followX = x;
    followY = y;
    followZ = z;
  }
};
