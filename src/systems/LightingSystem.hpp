#pragma once
#include "../resources/ResourceManager.hpp"

#include "../components/CameraComponent.hpp"
#include "../components/DirectionalLightComponent.hpp"
#include "../components/MaterialComponent.hpp"
#include "../components/PointLightComponent.hpp"
#include "../components/SpotLightComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/Tag.hpp"
#include "../ecs/World.hpp"
#include <unordered_set>

extern World gWorld;

class LightingSystem : public System {
public:
  void render() override {
    std::unordered_set<uint32_t> shadersNeedingLighting;

    gWorld.forEachWith<MaterialComponent>(
        [&](Entity entity, MaterialComponent &material) {
          if (material.receivesLighting && material.shaderProgram != 0) {
            shadersNeedingLighting.insert(material.shaderProgram);
          }
        });

    auto &resources = ResourceManager::instance();

    for (uint32_t shaderID : shadersNeedingLighting) {
      Shader *shader = resources.getShader(shaderID);
      if (shader) {
        applyLightsToShader(shader);
      }
    }
  }

private:
  void applyLightsToShader(Shader *shader) {
    shader->use();

    applyDirectionalLight(shader);
    applyPointLights(shader);
    applySpotLight(shader);
  }

  void applyDirectionalLight(Shader *shader) {
    bool found = false;

    gWorld.forEachWith<DirectionalLightComponent>(
        [&](Entity entity, DirectionalLightComponent &light) {
          if (found)
            return;

          shader->setVec3("dirLight.direction", light.direction);
          shader->setVec3("dirLight.ambient", light.ambient);
          shader->setVec3("dirLight.diffuse", light.diffuse);
          shader->setVec3("dirLight.specular", light.specular);
          found = true;
        });

    if (!found) {
      shader->setVec3("dirLight.ambient", glm::vec3(0.0f));
      shader->setVec3("dirLight.diffuse", glm::vec3(0.0f));
      shader->setVec3("dirLight.specular", glm::vec3(0.0f));
    }
  }

  void applyPointLights(Shader *shader) {
    int index = 0;

    gWorld.forEachWith<PointLightComponent, TransformComponent>(
        [&](Entity entity, PointLightComponent &light,
            TransformComponent &transform) {
          if (index >= 10)
            return;

          std::string base = "pointLights[" + std::to_string(index) + "]";
          shader->setVec3(base + ".position", transform.position);
          shader->setVec3(base + ".ambient", light.ambient);
          shader->setVec3(base + ".diffuse", light.diffuse);
          shader->setVec3(base + ".specular", light.specular);
          shader->setFloat(base + ".constant", light.constant);
          shader->setFloat(base + ".linear", light.linear);
          shader->setFloat(base + ".quadratic", light.quadratic);
          index++;
        });

    // Zero out unused slots
    for (int i = index; i < 10; i++) {
      std::string base = "pointLights[" + std::to_string(i) + "]";
      shader->setVec3(base + ".ambient", glm::vec3(0.0f));
      shader->setVec3(base + ".diffuse", glm::vec3(0.0f));
      shader->setVec3(base + ".specular", glm::vec3(0.0f));
      shader->setFloat(base + ".constant", 1.0f);
      shader->setFloat(base + ".linear", 0.0f);
      shader->setFloat(base + ".quadratic", 1.0f);
    }
  }

  void applySpotLight(Shader *shader) {
    // Find spotlight and active camera
    SpotLightComponent *spotLight = nullptr;
    TransformComponent *spotTransform = nullptr;
    CameraComponent *activeCamera = nullptr;
    TransformComponent *cameraTransform = nullptr;

    gWorld.forEachWith<SpotLightComponent, TransformComponent>(
        [&](Entity entity, SpotLightComponent &light,
            TransformComponent &transform) {
          if (!spotLight) {
            spotLight = &light;
            spotTransform = &transform;
          }
        });

    gWorld.forEachWith<CameraComponent, TransformComponent, TagComponent>(
        [&](Entity entity, CameraComponent &camera,
            TransformComponent &transform, TagComponent &tag) {
          if (!activeCamera && tag.has(ACTIVE)) {
            activeCamera = &camera;
            cameraTransform = &transform;
          }
        });

    if (spotLight && activeCamera && cameraTransform) {
      shader->setVec3("spotLight.position", cameraTransform->position);
      shader->setVec3("spotLight.direction", activeCamera->front);
      shader->setVec3("spotLight.ambient", spotLight->ambient);
      shader->setVec3("spotLight.diffuse", spotLight->diffuse);
      shader->setVec3("spotLight.specular", spotLight->specular);
      shader->setFloat("spotLight.constant", spotLight->constant);
      shader->setFloat("spotLight.linear", spotLight->linear);
      shader->setFloat("spotLight.quadratic", spotLight->quadratic);
      shader->setFloat("spotLight.cutOff", spotLight->cutOff);
      shader->setFloat("spotLight.outerCutOff", spotLight->outerCutOff);
      shader->setBool("spotLight.isActive", spotLight->active);
    } else {
      shader->setBool("spotLight.isActive", false);
    }
  }
};
