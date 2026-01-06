#pragma once

#include "../../components/CameraComponent.hpp"
#include "../../components/TransformComponent.hpp"
#include "../../ecs/Tag.hpp"
#include "../World.hpp"
#include <glm/glm.hpp>

struct ActiveCameraData {
  bool found = false;
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);
};

inline ActiveCameraData getActiveCamera(World &world, float aspectRatio,
                                        float nearPlane = 0.1f,
                                        float farPlane = 100.0f) {
  ActiveCameraData data;

  world.forEachWith<CameraComponent, TransformComponent, TagComponent>(
      [&](Entity entity, CameraComponent &camera, TransformComponent &transform,
          TagComponent &tag) {
        if (data.found)
          return;
        if (tag.type != ACTIVE)
          return;

        data.found = true;
        data.position = transform.position;
        data.front = camera.front;
        data.view = camera.getViewMatrix(transform.position);
        data.projection =
            camera.getProjectionMatrix(aspectRatio, nearPlane, farPlane);
      });

  return data;
}
