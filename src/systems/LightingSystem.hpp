#pragma once

#include "../shader_h.hpp"

#include "../components/CameraComponent.hpp"
#include "../components/DirectionalLightComponent.hpp"
#include "../components/PointLightComponent.hpp"
#include "../components/SpotLightComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/Entity.hpp"
#include "../ecs/System.hpp"
#include "../ecs/Tag.hpp"
#include "../ecs/World.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

extern World gWorld;

class LightingSystem : public System {
private:
  std::shared_ptr<Shader> targetShader;

public:
  LightingSystem(std::shared_ptr<Shader> shader) : targetShader(shader) {}

  void render() override { applyLights(targetShader); }

  void applyLights(std::shared_ptr<Shader> shader) {
    shader->use();

    applyDirectionalLight(shader);

    applyPointLights(shader);

    applySpotLight(shader);
  }

private:
  void applyDirectionalLight(std::shared_ptr<Shader> shader) {
    bool foundLight = false;

    gWorld.forEachWith<DirectionalLightComponent>(
        [&](Entity entity, DirectionalLightComponent &dirLight) {
          if (foundLight)
            return;

          shader->setVec3("dirLight.direction", dirLight.direction);
          shader->setVec3("dirLight.ambient", dirLight.ambient);
          shader->setVec3("dirLight.diffuse", dirLight.diffuse);
          shader->setVec3("dirLight.specular", dirLight.specular);
          foundLight = true;
        });

    if (!foundLight) {
      shader->setVec3("dirLight.direction", glm::vec3(0.0f, -1.0f, 0.0f));
      shader->setVec3("dirLight.ambient", glm::vec3(0.0f));
      shader->setVec3("dirLight.diffuse", glm::vec3(0.0f));
      shader->setVec3("dirLight.specular", glm::vec3(0.0f));
    }
  }

  void applyPointLights(std::shared_ptr<Shader> shader) {
    int lightIndex = 0;

    gWorld.forEachWith<PointLightComponent, TransformComponent>(
        [&](Entity entity, PointLightComponent &pointLight,
            TransformComponent &transform) {
          if (lightIndex >= 4) // shader limit
            return;

          std::string uniformBase =
              "pointLights[" + std::to_string(lightIndex) + "]";
          shader->setVec3(uniformBase + ".position", transform.position);
          shader->setVec3(uniformBase + ".ambient", pointLight.ambient);
          shader->setVec3(uniformBase + ".diffuse", pointLight.diffuse);
          shader->setVec3(uniformBase + ".specular", pointLight.specular);
          shader->setFloat(uniformBase + ".constant", pointLight.constant);
          shader->setFloat(uniformBase + ".linear", pointLight.linear);
          shader->setFloat(uniformBase + ".quadratic", pointLight.quadratic);

          lightIndex++;
        });

    for (int i = lightIndex; i < 4; i++) {
      std::string uniformBase = "pointLights[" + std::to_string(i) + "]";
      shader->setVec3(uniformBase + ".position", glm::vec3(0.0f));
      shader->setVec3(uniformBase + ".ambient", glm::vec3(0.0f));
      shader->setVec3(uniformBase + ".diffuse", glm::vec3(0.0f));
      shader->setVec3(uniformBase + ".specular", glm::vec3(0.0f));
      shader->setFloat(uniformBase + ".constant", 1.0f);
      shader->setFloat(uniformBase + ".linear", 0.0f);
      shader->setFloat(uniformBase + ".quadratic", 1.0f);
    }
  }

  void applySpotLight(std::shared_ptr<Shader> shader) {
    SpotLightComponent *spotComp = nullptr;
    TransformComponent *spotTransform = nullptr;

    gWorld.forEachWith<SpotLightComponent, TransformComponent>(
        [&](Entity entity, SpotLightComponent &spot,
            TransformComponent &transform) {
          if (spotComp)
            return;
          spotComp = &spot;
          spotTransform = &transform;
        });

    if (!spotComp || !spotTransform) {
      shader->setBool("spotLight.active", false);
      return;
    }

    CameraComponent *activeCamera = nullptr;
    TransformComponent *activeCameraTransform = nullptr;

    gWorld.forEachWith<CameraComponent, TransformComponent, TagComponent>(
        [&](Entity entity, CameraComponent &camera,
            TransformComponent &transform, TagComponent &tag) {
          if (activeCamera)
            return;
          if (tag.type == ACTIVE) {
            activeCamera = &camera;
            activeCameraTransform = &transform;
          }
        });

    if (activeCamera && activeCameraTransform) {
      spotTransform->position = activeCameraTransform->position;
      shader->setVec3("spotLight.position", spotTransform->position);
      shader->setVec3("spotLight.direction", activeCamera->front);
      shader->setVec3("spotLight.ambient", spotComp->ambient);
      shader->setVec3("spotLight.diffuse", spotComp->diffuse);
      shader->setVec3("spotLight.specular", spotComp->specular);
      shader->setFloat("spotLight.constant", spotComp->constant);
      shader->setFloat("spotLight.linear", spotComp->linear);
      shader->setFloat("spotLight.quadratic", spotComp->quadratic);
      shader->setFloat("spotLight.cutOff", spotComp->cutOff);
      shader->setFloat("spotLight.outerCutOff", spotComp->outerCutOff);
      shader->setBool("spotLight.active", spotComp->active);
    }
  }
};
