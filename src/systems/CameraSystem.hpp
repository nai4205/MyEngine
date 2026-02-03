#pragma once

#include "../components/CameraComponent.hpp"
#include "../components/TransformComponent.hpp"

#include "../ecs/Entity.hpp"
#include "../ecs/System.hpp"
#include "../ecs/Tag.hpp"
#include "../ecs/World.hpp"

extern World gWorld;

class CameraSystem : public System {
public:
  void update(float &deltaTime) override {
    gWorld.forEachWith<TransformComponent, CameraComponent>(
        [](Entity entity, TransformComponent &transform,
           CameraComponent &camera) { camera.updateCameraComponentVectors(); });
  }

  Entity getActiveCameraEntity(World &world) {
    Entity activeCameraEntity = NULL_ENTITY;

    gWorld.forEachWith<CameraComponent, TagComponent>(
        [&](Entity entity, CameraComponent &camera, TagComponent &tag) {
          if (activeCameraEntity != NULL_ENTITY)
            return; // Already found
          if (tag.has(ACTIVE)) {
            activeCameraEntity = entity;
          }
        });

    return activeCameraEntity;
  }

  // TODO: mixed usage of this and the camera utils one
  CameraComponent *getActiveCamera(World &world) {
    Entity cameraEntity = getActiveCameraEntity(gWorld);
    if (cameraEntity != NULL_ENTITY) {
      return gWorld.getComponent<CameraComponent>(cameraEntity);
    }
    return nullptr;
  }

  TransformComponent *getActiveCameraTransform(World &world) {
    Entity cameraEntity = getActiveCameraEntity(gWorld);
    if (cameraEntity != NULL_ENTITY) {
      return gWorld.getComponent<TransformComponent>(cameraEntity);
    }
    return nullptr;
  }
};
