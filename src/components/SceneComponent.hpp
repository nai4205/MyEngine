#include <glm/glm.hpp>
#include <string>
#pragma once

struct SceneComponent {
  std::string name;
  glm::vec3 clearColor;

  SceneComponent() = default;
  SceneComponent(const std::string &n) : name(n) {}
};
