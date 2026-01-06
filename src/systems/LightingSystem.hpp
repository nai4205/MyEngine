#ifndef LIGHTING_SYSTEM_HPP
#define LIGHTING_SYSTEM_HPP

#include "../shader_h.hpp"

#include "../components/CameraComponent.hpp"
#include "../components/DirectionalLightComponent.hpp"
#include "../components/PointLightComponent.hpp"
#include "../components/SpotLightComponent.hpp"
#include "../components/Transform.hpp"
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
  // Apply directional light to shader
  void applyDirectionalLight(std::shared_ptr<Shader> shader) {
    auto entities = gWorld.getEntitiesWith<DirectionalLightComponent>();

    if (entities.empty()) {
      // No directional light, set defaults
      shader->setVec3("dirLight.direction", glm::vec3(0.0f, -1.0f, 0.0f));
      shader->setVec3("dirLight.ambient", glm::vec3(0.0f));
      shader->setVec3("dirLight.diffuse", glm::vec3(0.0f));
      shader->setVec3("dirLight.specular", glm::vec3(0.0f));
      return;
    }

    // Take first directional light (shader supports only 1)
    Entity dirEntity = entities[0];
    auto *dirLight = gWorld.getComponent<DirectionalLightComponent>(dirEntity);

    if (dirLight) {
      shader->setVec3("dirLight.direction", dirLight->direction);
      shader->setVec3("dirLight.ambient", dirLight->ambient);
      shader->setVec3("dirLight.diffuse", dirLight->diffuse);
      shader->setVec3("dirLight.specular", dirLight->specular);
    }
  }

  // Apply up to 4 point lights to shader
  void applyPointLights(std::shared_ptr<Shader> shader) {
    auto entities = gWorld.getEntitiesWith<PointLightComponent, Transform>();

    // Apply up to 4 point lights (shader limit)
    for (size_t i = 0; i < 4; i++) {
      std::string uniformBase = "pointLights[" + std::to_string(i) + "]";

      if (i < entities.size()) {
        Entity lightEntity = entities[i];
        auto *pointLight =
            gWorld.getComponent<PointLightComponent>(lightEntity);
        auto *transform = gWorld.getComponent<Transform>(lightEntity);

        if (pointLight && transform) {
          shader->setVec3(uniformBase + ".position", transform->position);
          shader->setVec3(uniformBase + ".ambient", pointLight->ambient);
          shader->setVec3(uniformBase + ".diffuse", pointLight->diffuse);
          shader->setVec3(uniformBase + ".specular", pointLight->specular);
          shader->setFloat(uniformBase + ".constant", pointLight->constant);
          shader->setFloat(uniformBase + ".linear", pointLight->linear);
          shader->setFloat(uniformBase + ".quadratic", pointLight->quadratic);
        }
      } else {
        // No light at this index, set to zero/inactive
        shader->setVec3(uniformBase + ".position", glm::vec3(0.0f));
        shader->setVec3(uniformBase + ".ambient", glm::vec3(0.0f));
        shader->setVec3(uniformBase + ".diffuse", glm::vec3(0.0f));
        shader->setVec3(uniformBase + ".specular", glm::vec3(0.0f));
        shader->setFloat(uniformBase + ".constant", 1.0f);
        shader->setFloat(uniformBase + ".linear", 0.0f);
        shader->setFloat(uniformBase + ".quadratic", 1.0f);
      }
    }
  }

  // Apply spotlight to shader (follows main camera)
  void applySpotLight(std::shared_ptr<Shader> shader) {
    auto spotEntities = gWorld.getEntitiesWith<SpotLightComponent, Transform>();

    if (spotEntities.empty()) {
      shader->setBool("spotLight.active", false);
      return;
    }

    // Take first spotlight
    Entity spotEntity = spotEntities[0];
    auto *spotComp = gWorld.getComponent<SpotLightComponent>(spotEntity);
    auto *spotTransform = gWorld.getComponent<Transform>(spotEntity);

    if (!spotComp || !spotTransform) {
      shader->setBool("spotLight.active", false);
      return;
    }

    // Update spotlight to follow main camera
    auto cameraEntities = gWorld.getEntitiesWith<Camera, Transform, Tag>();
    for (Entity entity : cameraEntities) {
      auto *tag = gWorld.getComponent<Tag>(entity);
      if (tag && tag->type == ACTIVE) {
        auto *cam = gWorld.getComponent<Camera>(entity);
        auto *camTransform = gWorld.getComponent<Transform>(entity);

        if (cam && camTransform) {
          // Update spotlight Transform to match camera
          spotTransform->position = camTransform->position;

          // Apply to shader
          shader->setVec3("spotLight.position", spotTransform->position);
          shader->setVec3("spotLight.direction", cam->front);
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
        break;
      }
    }
  }
};

#endif // LIGHTING_SYSTEM_HPP
