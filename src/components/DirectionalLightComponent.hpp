#ifndef DIRECTIONAL_LIGHT_COMPONENT_HPP
#define DIRECTIONAL_LIGHT_COMPONENT_HPP

#include <glm/glm.hpp>

// Directional Light Component - light from a distant source (like the sun)
struct DirectionalLightComponent {
  glm::vec3 direction;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;

  DirectionalLightComponent(glm::vec3 dir = glm::vec3(-0.2f, -1.0f, -0.3f),
                            glm::vec3 amb = glm::vec3(0.05f),
                            glm::vec3 diff = glm::vec3(0.4f),
                            glm::vec3 spec = glm::vec3(0.5f))
      : direction(dir), ambient(amb), diffuse(diff), specular(spec) {}
};

#endif // DIRECTIONAL_LIGHT_COMPONENT_HPP
