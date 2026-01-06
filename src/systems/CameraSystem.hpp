#ifndef CAMERA_SYSTEM_HPP
#define CAMERA_SYSTEM_HPP

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
    // Query for all entities with Transform and Camera components
    auto entities = gWorld.getEntitiesWith<Transform, Camera>();

    for (Entity entity : entities) {
      auto *camera = gWorld.getComponent<Camera>(entity);

      if (camera) {
        // Update camera vectors based on yaw and pitch
        camera->updateCameraVectors();
      }
    }
  }

  Entity getActiveCameraEntity(World &world) {
    auto entities = gWorld.getEntitiesWith<Camera, Tag>();

    for (Entity entity : entities) {
      auto *tag = gWorld.getComponent<Tag>(entity);
      if (tag && tag->type == ACTIVE) {
        return entity;
      }
    }

    return NULL_ENTITY;
  }

  // Get the main camera component
  Camera *getActiveCamera(World &world) {
    Entity cameraEntity = getActiveCameraEntity(gWorld);
    if (cameraEntity != NULL_ENTITY) {
      return gWorld.getComponent<Camera>(cameraEntity);
    }
    return nullptr;
  }

  // Get the main camera's transform
  Transform *getActiveCameraTransform(World &world) {
    Entity cameraEntity = getActiveCameraEntity(gWorld);
    if (cameraEntity != NULL_ENTITY) {
      return gWorld.getComponent<Transform>(cameraEntity);
    }
    return nullptr;
  }
};

#endif // CAMERA_SYSTEM_HPP
