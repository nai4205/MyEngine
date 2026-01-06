#ifndef POINT_LIGHT_COMPONENT_HPP
#define POINT_LIGHT_COMPONENT_HPP

#include <glm/glm.hpp>

// Point Light Component - omnidirectional light with distance attenuation
struct PointLightComponent {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float constant;
  float linear;
  float quadratic;

  PointLightComponent(glm::vec3 amb = glm::vec3(0.05f),
                      glm::vec3 diff = glm::vec3(0.8f),
                      glm::vec3 spec = glm::vec3(1.0f), float c = 1.0f,
                      float l = 0.09f, float q = 0.032f)
      : ambient(amb), diffuse(diff), specular(spec), constant(c), linear(l),
        quadratic(q) {}
};

#endif // POINT_LIGHT_COMPONENT_HPP
