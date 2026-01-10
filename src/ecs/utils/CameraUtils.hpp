#pragma once

#include "../../components/CameraComponent.hpp"
#include "../../components/TransformComponent.hpp"
#include "../../ecs/Tag.hpp"
#include "../World.hpp"
#include <glm/glm.hpp>
#include <iostream>

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
        if (!tag.has(ACTIVE))
          return;

        data.found = true;
        data.position = transform.position;
        data.front = camera.front;
        data.view = camera.getViewMatrix(transform.position);
        data.projection =
            camera.getProjectionMatrix(aspectRatio, nearPlane, farPlane);

        // Debug logging (only once)
        static bool logged = false;
        if (!logged) {
          std::cout << "Active camera found - Pos: (" << transform.position.x << ", "
                    << transform.position.y << ", " << transform.position.z
                    << "), Front: (" << camera.front.x << ", " << camera.front.y
                    << ", " << camera.front.z << "), Ortho: "
                    << (camera.isOrthographic ? "YES" : "NO") << std::endl;
          logged = true;
        }
      });

  if (!data.found) {
    std::cout << "WARNING: No active camera found!" << std::endl;
  }

  return data;
}
