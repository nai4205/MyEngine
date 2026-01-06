#ifndef LIGHT_MANAGER_HPP
#define LIGHT_MANAGER_HPP

#include "LightComponent.hpp"
#include <array>
#include <memory>

// Manages all lights in the scene and applies them to shaders
// Matches the shader's expected uniform structure:
// - 1 directional light (dirLight)
// - 4 point lights (pointLights[0-3])
// - 1 spot light (spotLight)
class LightManager {
public:
  LightManager() : dirLight(nullptr), spotLight(nullptr) {
    for (int i = 0; i < 4; i++) {
      pointLights[i] = nullptr;
    }
  }

  void setDirectionalLight(std::shared_ptr<DirectionalLight> light) {
    dirLight = light;
  }

  void setPointLight(int index, std::shared_ptr<PointLight> light) {
    if (index >= 0 && index < 4) {
      pointLights[index] = light;
    }
  }

  void setSpotLight(std::shared_ptr<SpotLight> light) { spotLight = light; }

  void applyAllToShader(std::shared_ptr<Shader> shader) const {
    shader->use();

    if (dirLight) {
      dirLight->applyToShader(shader, "dirLight");
    }

    for (int i = 0; i < 4; i++) {
      if (pointLights[i]) {
        std::string uniformName = "pointLights[" + std::to_string(i) + "]";
        pointLights[i]->applyToShader(shader, uniformName);
      }
    }

    if (spotLight) {
      spotLight->applyToShader(shader, "spotLight");
    } else {
      shader->setBool("spotLight.active", false);
    }
  }

  std::shared_ptr<DirectionalLight> getDirectionalLight() const {
    return dirLight;
  }
  std::shared_ptr<PointLight> getPointLight(int index) const {
    if (index >= 0 && index < 4) {
      return pointLights[index];
    }
    return nullptr;
  }
  std::shared_ptr<SpotLight> getSpotLight() const { return spotLight; }

private:
  std::shared_ptr<DirectionalLight> dirLight;
  std::array<std::shared_ptr<PointLight>, 4> pointLights;
  std::shared_ptr<SpotLight> spotLight;
};

#endif
