#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>

constexpr size_t MAX_MATERIAL_TEXTURES = 4;

struct MaterialComponent {
  uint32_t shaderProgram = 0;

  glm::vec3 ambient = glm::vec3(0.1f);
  glm::vec3 diffuse = glm::vec3(0.8f);
  glm::vec3 specular = glm::vec3(1.0f);
  float shininess = 32.0f;

  // Index 0 = diffuse, 1 = specular, 2 = normal, 3 = emission
  std::array<uint32_t, MAX_MATERIAL_TEXTURES> textures = {0, 0, 0, 0};

  bool useTextures = false;
  bool receivesLighting = true;
  bool hasTransparency = false;
};
