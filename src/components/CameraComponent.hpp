#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// TODO: seperate logic and data
class CameraComponent {
public:
  float yaw;
  float pitch;

  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 worldUp;

  float zoom;
  bool isOrthographic;
  float orthoWidth;
  float orthoHeight;

  CameraComponent(float initialYaw = -90.0f, float initialPitch = 0.0f,
                  float initialZoom = 45.0f,
                  glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f),
                  bool ortho = false, float orthoW = 10.0f,
                  float orthoH = 10.0f)
      : yaw(initialYaw), pitch(initialPitch), zoom(initialZoom),
        worldUp(upVector), front(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)), right(glm::vec3(1.0f, 0.0f, 0.0f)),
        isOrthographic(ortho), orthoWidth(orthoW), orthoHeight(orthoH) {
    updateCameraComponentVectors();
  }

  glm::mat4 getViewMatrix(const glm::vec3 &position) const {
    return glm::lookAt(position, position + front, up);
  }

  glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f,
                                float farPlane = 100.0f) const {
    if (isOrthographic) {
      return glm::ortho(0.0f, orthoWidth, orthoHeight, 0.0f, nearPlane,
                        farPlane);
    } else {
      return glm::perspective(glm::radians(zoom), aspectRatio, nearPlane,
                              farPlane);
    }
  }

  void updateCameraComponentVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
  }

  void rotate(float yawOffset, float pitchOffset, bool constrainPitch = true) {
    yaw += yawOffset;
    pitch += pitchOffset;

    if (constrainPitch) {
      if (pitch > MAX_PITCH)
        pitch = MAX_PITCH;
      if (pitch < MIN_PITCH)
        pitch = MIN_PITCH;
    }

    updateCameraComponentVectors();
  }

  void adjustZoom(float offset) {
    zoom -= offset;
    if (zoom < MIN_ZOOM)
      zoom = MIN_ZOOM;
    if (zoom > MAX_ZOOM)
      zoom = MAX_ZOOM;
  }

  // Get front direction flattened to XZ plane (for horizontal movement)
  glm::vec3 getFlatFront() const {
    return glm::normalize(glm::vec3(front.x, 0.0f, front.z));
  }

private:
  static constexpr float MIN_ZOOM = 1.0f;
  static constexpr float MAX_ZOOM = 45.0f;
  static constexpr float MIN_PITCH = -89.0f;
  static constexpr float MAX_PITCH = 89.0f;
};
