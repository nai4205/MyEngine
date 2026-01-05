#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "../shader_h.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

// Base Light class - common properties for all lights
class Light {
public:
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;

  Light(glm::vec3 amb = glm::vec3(0.05f), glm::vec3 diff = glm::vec3(0.8f),
        glm::vec3 spec = glm::vec3(1.0f))
      : ambient(amb), diffuse(diff), specular(spec) {}

  virtual ~Light() = default;

  virtual void applyToShader(std::shared_ptr<Shader> shader,
                             const std::string &uniformName) const = 0;
};

// Directional Light (like the sun)
class DirectionalLight : public Light {
public:
  glm::vec3 direction;

  DirectionalLight(glm::vec3 dir = glm::vec3(-0.2f, -1.0f, -0.3f),
                   glm::vec3 amb = glm::vec3(0.05f),
                   glm::vec3 diff = glm::vec3(0.4f),
                   glm::vec3 spec = glm::vec3(0.5f))
      : Light(amb, diff, spec), direction(dir) {}

  void applyToShader(std::shared_ptr<Shader> shader,
                     const std::string &uniformName) const override {
    shader->setVec3(uniformName + ".direction", direction);
    shader->setVec3(uniformName + ".ambient", ambient);
    shader->setVec3(uniformName + ".diffuse", diffuse);
    shader->setVec3(uniformName + ".specular", specular);
  }
};

// Point Light (omnidirectional with attenuation)
class PointLight : public Light {
public:
  glm::vec3 position;
  float constant;
  float linear;
  float quadratic;

  PointLight(glm::vec3 pos = glm::vec3(0.0f),
             glm::vec3 amb = glm::vec3(0.05f),
             glm::vec3 diff = glm::vec3(0.8f),
             glm::vec3 spec = glm::vec3(1.0f))
      : Light(amb, diff, spec), position(pos), constant(1.0f), linear(0.09f),
        quadratic(0.032f) {}

  void applyToShader(std::shared_ptr<Shader> shader,
                     const std::string &uniformName) const override {
    shader->setVec3(uniformName + ".position", position);
    shader->setVec3(uniformName + ".ambient", ambient);
    shader->setVec3(uniformName + ".diffuse", diffuse);
    shader->setVec3(uniformName + ".specular", specular);
    shader->setFloat(uniformName + ".constant", constant);
    shader->setFloat(uniformName + ".linear", linear);
    shader->setFloat(uniformName + ".quadratic", quadratic);
  }
};

// Spot Light (directional cone with attenuation)
class SpotLight : public Light {
public:
  glm::vec3 position;
  glm::vec3 direction;
  float cutOff;
  float outerCutOff;
  float constant;
  float linear;
  float quadratic;
  bool active;

  SpotLight(glm::vec3 pos = glm::vec3(0.0f),
            glm::vec3 dir = glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3 amb = glm::vec3(0.0f),
            glm::vec3 diff = glm::vec3(1.0f),
            glm::vec3 spec = glm::vec3(1.0f))
      : Light(amb, diff, spec), position(pos), direction(dir),
        cutOff(glm::cos(glm::radians(12.5f))),
        outerCutOff(glm::cos(glm::radians(15.0f))), constant(1.0f),
        linear(0.09f), quadratic(0.032f), active(true) {}

  void applyToShader(std::shared_ptr<Shader> shader,
                     const std::string &uniformName) const override {
    shader->setBool(uniformName + ".active", active);
    shader->setVec3(uniformName + ".position", position);
    shader->setVec3(uniformName + ".direction", direction);
    shader->setVec3(uniformName + ".ambient", ambient);
    shader->setVec3(uniformName + ".diffuse", diffuse);
    shader->setVec3(uniformName + ".specular", specular);
    shader->setFloat(uniformName + ".constant", constant);
    shader->setFloat(uniformName + ".linear", linear);
    shader->setFloat(uniformName + ".quadratic", quadratic);
    shader->setFloat(uniformName + ".cutOff", cutOff);
    shader->setFloat(uniformName + ".outerCutOff", outerCutOff);
  }
};

#endif
