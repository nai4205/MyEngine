#pragma once

#include "../components/CameraComponent.hpp"
#include "../components/Transform.hpp"

#include "../ecs/Entity.hpp"
#include "../ecs/System.hpp"
#include "../ecs/Tag.hpp"
#include "../ecs/World.hpp"

extern World gWorld;

// System that updates camera vectors and matrices
class CameraSystem : public System {
public:
  void update(float &deltaTime) override {
    // Update camera vectors for all cameras
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
          if (tag.type == ACTIVE) {
            activeCameraEntity = entity;
          }
        });

    return activeCameraEntity;
  }

  // Get the main camera component
  CameraComponent *getActiveCamera(World &world) {
    Entity cameraEntity = getActiveCameraEntity(gWorld);
    if (cameraEntity != NULL_ENTITY) {
      return gWorld.getComponent<CameraComponent>(cameraEntity);
    }
    return nullptr;
  }

  // Get the main camera's transform
  TransformComponent *getActiveCameraTransform(World &world) {
    Entity cameraEntity = getActiveCameraEntity(gWorld);
    if (cameraEntity != NULL_ENTITY) {
      return gWorld.getComponent<TransformComponent>(cameraEntity);
    }
    return nullptr;
  }
};
