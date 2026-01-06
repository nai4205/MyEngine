#ifndef LIGHTING_PRESETS_HPP
#define LIGHTING_PRESETS_HPP

#include <glm/glm.hpp>

enum class LightingType { DESERT, FACTORY, HORROR, BIOCHEMICAL_LAB };

struct PointLightConfig {
  glm::vec3 color;
  float constant;
  float linear;
  float quadratic;
  float ambientMultiplier;
};

struct SpotLightConfig {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float constant;
  float linear;
  float quadratic;
  float cutOffDegrees;
  float outerCutOffDegrees;
};

struct LightingProperties {
  glm::vec4 clearColor;

  // Directional light
  glm::vec3 dirLightDirection;
  glm::vec3 dirLightAmbient;
  glm::vec3 dirLightDiffuse;
  glm::vec3 dirLightSpecular;

  // Point lights (4)
  PointLightConfig pointLights[4];

  // Spotlight
  SpotLightConfig spotlight;
};

class LightingPresets {
public:
  static LightingProperties getProperties(LightingType type) {
    LightingProperties props;

    switch (type) {
    case LightingType::DESERT:
      props.clearColor = glm::vec4(0.75f, 0.52f, 0.3f, 1.0f);
      props.dirLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
      props.dirLightAmbient = glm::vec3(0.3f, 0.24f, 0.14f);
      props.dirLightDiffuse = glm::vec3(0.7f, 0.42f, 0.26f);
      props.dirLightSpecular = glm::vec3(0.5f, 0.5f, 0.5f);
      props.pointLights[0] = {glm::vec3(1.0f, 0.6f, 0.0f), 1.0f, 0.09f, 0.032f,
                              0.1f};
      props.pointLights[1] = {glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.09f, 0.032f,
                              0.1f};
      props.pointLights[2] = {glm::vec3(1.0f, 1.0f, 0.0f), 1.0f, 0.09f, 0.032f,
                              0.1f};
      props.pointLights[3] = {glm::vec3(0.2f, 0.2f, 1.0f), 1.0f, 0.09f, 0.032f,
                              0.1f};
      props.spotlight = {glm::vec3(0.0f, 0.0f, 0.0f),
                         glm::vec3(0.8f, 0.8f, 0.0f),
                         glm::vec3(0.8f, 0.8f, 0.0f),
                         1.0f,
                         0.09f,
                         0.032f,
                         12.5f,
                         13.0f};
      break;

    case LightingType::FACTORY:
      props.clearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
      props.dirLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
      props.dirLightAmbient = glm::vec3(0.05f, 0.05f, 0.1f);
      props.dirLightDiffuse = glm::vec3(0.2f, 0.2f, 0.7f);
      props.dirLightSpecular = glm::vec3(0.7f, 0.7f, 0.7f);
      props.pointLights[0] = {glm::vec3(0.2f, 0.2f, 0.6f), 1.0f, 0.09f, 0.032f,
                              0.1f};
      props.pointLights[1] = {glm::vec3(0.3f, 0.3f, 0.7f), 1.0f, 0.09f, 0.032f,
                              0.1f};
      props.pointLights[2] = {glm::vec3(0.0f, 0.0f, 0.3f), 1.0f, 0.09f, 0.032f,
                              0.1f};
      props.pointLights[3] = {glm::vec3(0.4f, 0.4f, 0.4f), 1.0f, 0.09f, 0.032f,
                              0.1f};
      props.spotlight = {glm::vec3(0.0f, 0.0f, 0.0f),
                         glm::vec3(1.0f, 1.0f, 1.0f),
                         glm::vec3(1.0f, 1.0f, 1.0f),
                         1.0f,
                         0.009f,
                         0.0032f,
                         10.0f,
                         12.5f};
      break;

    case LightingType::HORROR:
      props.clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
      props.dirLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
      props.dirLightAmbient = glm::vec3(0.0f, 0.0f, 0.0f);
      props.dirLightDiffuse = glm::vec3(0.05f, 0.05f, 0.05f);
      props.dirLightSpecular = glm::vec3(0.2f, 0.2f, 0.2f);
      props.pointLights[0] = {glm::vec3(0.1f, 0.1f, 0.1f), 1.0f, 0.14f, 0.07f,
                              0.1f};
      props.pointLights[1] = {glm::vec3(0.1f, 0.1f, 0.1f), 1.0f, 0.14f, 0.07f,
                              0.1f};
      props.pointLights[2] = {glm::vec3(0.1f, 0.1f, 0.1f), 1.0f, 0.22f, 0.20f,
                              0.1f};
      props.pointLights[3] = {glm::vec3(0.3f, 0.1f, 0.1f), 1.0f, 0.14f, 0.07f,
                              0.1f};
      props.spotlight = {glm::vec3(0.0f, 0.0f, 0.0f),
                         glm::vec3(1.0f, 1.0f, 1.0f),
                         glm::vec3(1.0f, 1.0f, 1.0f),
                         1.0f,
                         0.09f,
                         0.032f,
                         10.0f,
                         15.0f};
      break;

    case LightingType::BIOCHEMICAL_LAB:
      props.clearColor = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
      props.dirLightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
      props.dirLightAmbient = glm::vec3(0.5f, 0.5f, 0.5f);
      props.dirLightDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
      props.dirLightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
      props.pointLights[0] = {glm::vec3(0.4f, 0.7f, 0.1f), 1.0f, 0.07f, 0.017f,
                              0.1f};
      props.pointLights[1] = {glm::vec3(0.4f, 0.7f, 0.1f), 1.0f, 0.07f, 0.017f,
                              0.1f};
      props.pointLights[2] = {glm::vec3(0.4f, 0.7f, 0.1f), 1.0f, 0.07f, 0.017f,
                              0.1f};
      props.pointLights[3] = {glm::vec3(0.4f, 0.7f, 0.1f), 1.0f, 0.07f, 0.017f,
                              0.1f};
      props.spotlight = {glm::vec3(0.0f, 0.0f, 0.0f),
                         glm::vec3(0.0f, 1.0f, 0.0f),
                         glm::vec3(0.0f, 1.0f, 0.0f),
                         1.0f,
                         0.07f,
                         0.017f,
                         7.0f,
                         10.0f};
      break;
    }

    return props;
  }

  static glm::vec4 getClearColor(LightingType type) {
    return getProperties(type).clearColor;
  }
};

#endif
