#pragma once

#include "../components/CameraComponent.hpp"
#include "../components/RenderComponent.hpp"
#include "../components/Transform.hpp"

#include "../ecs/System.hpp"
#include "../ecs/Tag.hpp"
#include "../ecs/World.hpp"
#include <glm/glm.hpp>

extern World gWorld;

class RenderSystem : public System {
private:
  unsigned int screenWidth = 800;
  unsigned int screenHeight = 600;

public:
  RenderSystem(unsigned int width = 800, unsigned int height = 600)
      : screenWidth(width), screenHeight(height) {}

  void render() override {
    Camera *camera = nullptr;
    Transform *cameraTransform = nullptr;

    auto cameraEntities = gWorld.getEntitiesWith<Camera, Transform, Tag>();
    for (Entity entity : cameraEntities) {
      auto *tag = gWorld.getComponent<Tag>(entity);
      if (tag && tag->type == ACTIVE) {
        camera = gWorld.getComponent<Camera>(entity);
        cameraTransform = gWorld.getComponent<Transform>(entity);
        break;
      }
    }

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    if (camera && cameraTransform) {
      float aspectRatio =
          static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
      projection = camera->getProjectionMatrix(aspectRatio, 0.1f, 100.0f);
      view = camera->getViewMatrix(cameraTransform->position);
    }

    auto entities = gWorld.getEntitiesWith<Transform, RenderComponent>();

    for (Entity entity : entities) {
      auto *transform = gWorld.getComponent<Transform>(entity);
      auto *renderComp = gWorld.getComponent<RenderComponent>(entity);

      if (transform && renderComp && renderComp->hasRenderer()) {
        std::visit(
            [&](auto &&renderer) {
              using T = std::decay_t<decltype(renderer)>;

              if constexpr (std::is_same_v<T, std::shared_ptr<MeshRenderer>>) {
                // Render non-indexed mesh
                auto material = renderer->getMaterial();
                material->setMat4("model", transform->getModelMatrix());
                material->setMat4("projection", projection);
                material->setMat4("view", view);
                renderer->render();
              } else if constexpr (std::is_same_v<
                                       T,
                                       std::shared_ptr<MeshRendererIndexed>>) {
                // Render indexed mesh
                auto material = renderer->getMaterial();
                material->setMat4("model", transform->getModelMatrix());
                material->setMat4("projection", projection);
                material->setMat4("view", view);
                renderer->render();
              }
            },
            renderComp->renderer);
      }
    }
  }

  void setScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
  }
};
