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
    CameraComponent *activeCamera = nullptr;
    TransformComponent *activeCameraTransform = nullptr;

    gWorld.forEachWith<CameraComponent, TransformComponent, TagComponent>(
        [&](Entity entity, CameraComponent &camera,
            TransformComponent &transform, TagComponent &tag) {
          if (activeCamera)
            return;
          if (tag.type != ACTIVE)
            return;
          activeCamera = &camera;
          activeCameraTransform = &transform;
        });

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    if (activeCamera && activeCameraTransform) {
      float aspectRatio =
          static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
      projection = activeCamera->getProjectionMatrix(aspectRatio, 0.1f, 100.0f);
      view = activeCamera->getViewMatrix(activeCameraTransform->position);
    }

    gWorld.forEachWith<TransformComponent,
                       RenderComponent>([&](Entity entity,
                                            TransformComponent &transform,
                                            RenderComponent &renderComp) {
      if (renderComp.hasRenderer()) {
        std::visit(
            [&](auto &&renderer) {
              using T = std::decay_t<decltype(renderer)>;

              if constexpr (std::is_same_v<T, std::shared_ptr<MeshRenderer>>) {
                auto material = renderer->getMaterial();
                material->setMat4("model", transform.getModelMatrix());
                material->setMat4("projection", projection);
                material->setMat4("view", view);
                renderer->render();
              } else if constexpr (std::is_same_v<
                                       T,
                                       std::shared_ptr<MeshRendererIndexed>>) {
                auto material = renderer->getMaterial();
                material->setMat4("model", transform.getModelMatrix());
                material->setMat4("projection", projection);
                material->setMat4("view", view);
                renderer->render();
              }
            },
            renderComp.renderer);
      }
    });
  }

  void setScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
  }
};
