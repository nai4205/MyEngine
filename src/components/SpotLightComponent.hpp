#ifndef SPOT_LIGHT_COMPONENT_HPP
#define SPOT_LIGHT_COMPONENT_HPP

#include <glm/glm.hpp>

// Spot Light Component - directional cone light (like a flashlight)
struct SpotLightComponent {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float constant;
  float linear;
  float quadratic;
  float cutOff;
  float outerCutOff;
  bool active;

  SpotLightComponent(glm::vec3 amb = glm::vec3(0.0f),
                     glm::vec3 diff = glm::vec3(1.0f),
                     glm::vec3 spec = glm::vec3(1.0f), float c = 1.0f,
                     float l = 0.09f, float q = 0.032f, float cutoff = 12.5f,
                     float outerCutoff = 15.0f, bool isActive = true)
      : ambient(amb), diffuse(diff), specular(spec), constant(c), linear(l),
        quadratic(q), cutOff(glm::cos(glm::radians(cutoff))),
        outerCutOff(glm::cos(glm::radians(outerCutoff))), active(isActive) {}
};

#endif // SPOT_LIGHT_COMPONENT_HPP
