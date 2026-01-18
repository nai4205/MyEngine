#pragma once

#include <glm/glm.hpp>

enum class CameraFollowMode { INSTANT, SMOOTH, DEADZONE };

struct CameraFollowComponent {
  glm::vec3 offset{0.0f};
  float smoothSpeed = 5.0f;
  CameraFollowMode mode = CameraFollowMode::SMOOTH;

  float deadzoneX = 0.0f;
  float deadzoneZ = 2.0f;

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

  void setDeadzone(float x, float z) {
    deadzoneX = x;
    deadzoneZ = z;
  }
};
