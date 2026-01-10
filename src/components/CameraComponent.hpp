#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// CameraComponent component - calculates view matrix from orientation
// Uses GameObject's Transform for position
class CameraComponent {
public:
  // CameraComponent orientation (Euler angles in degrees)
  float yaw;
  float pitch;

  // CameraComponent vectors (calculated from yaw/pitch)
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 worldUp;

  // CameraComponent properties
  float zoom; // FOV in degrees for perspective, or ortho size for orthographic
  bool isOrthographic;
  float orthoWidth;
  float orthoHeight;

  // Constructor
  CameraComponent(float initialYaw = -90.0f, float initialPitch = 0.0f,
                  float initialZoom = 45.0f,
                  glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f),
                  bool ortho = false, float orthoW = 10.0f, float orthoH = 10.0f)
      : yaw(initialYaw), pitch(initialPitch), zoom(initialZoom),
        worldUp(upVector), front(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)), right(glm::vec3(1.0f, 0.0f, 0.0f)),
        isOrthographic(ortho), orthoWidth(orthoW), orthoHeight(orthoH) {
    updateCameraComponentVectors();
    std::cout << "Camera initialized - Yaw: " << yaw << "°, Pitch: " << pitch
              << "° (" << (isOrthographic ? "Orthographic" : "Perspective") << ")" << std::endl;
  }

  // Get view matrix using provided position (from Transform)
  glm::mat4 getViewMatrix(const glm::vec3 &position) const {
    return glm::lookAt(position, position + front, up);
  }

  // Get projection matrix
  glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f,
                                float farPlane = 100.0f) const {
    if (isOrthographic) {
      // Use orthoHeight as the base size, calculate width from aspect ratio
      float halfHeight = orthoHeight * 0.5f;
      float halfWidth = halfHeight * aspectRatio;
      return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
    } else {
      return glm::perspective(glm::radians(zoom), aspectRatio, nearPlane,
                              farPlane);
    }
  }

  // Calculate the new Front vector
  void updateCameraComponentVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    // Also re-calculate the Right and Up vector
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
  }

  void rotate(float yawOffset, float pitchOffset, bool constrainPitch = true) {
    yaw += yawOffset;
    pitch += pitchOffset;

    // Constrain pitch to prevent view flip
    if (constrainPitch) {
      if (pitch > MAX_PITCH)
        pitch = MAX_PITCH;
      if (pitch < MIN_PITCH)
        pitch = MIN_PITCH;
    }

    updateCameraComponentVectors();

    // std::cout << "Camera Angles - Yaw: " << yaw << "°, Pitch: " << pitch <<
    // "°" << std::endl;
  }

  // Modify zoom (used by CameraComponentController)
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

#endif // CAMERA_HPP
