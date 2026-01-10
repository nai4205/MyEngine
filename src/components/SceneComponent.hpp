#include <string>
#pragma once

struct SceneComponent {
  std::string name;

  SceneComponent() = default;
  SceneComponent(const std::string &n) : name(n) {}
};
