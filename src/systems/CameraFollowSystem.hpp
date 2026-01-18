#pragma once

#include "../components/CameraFollowComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/Entity.hpp"
#include "../ecs/System.hpp"
#include "../ecs/Tag.hpp"
#include "../ecs/World.hpp"

extern World gWorld;

class CameraFollowSystem : public System {
public:
  void update(float &deltaTime) override {
    // Find the player entity (entity with PLAYER tag)
    Entity playerEntity = NULL_ENTITY;
    TransformComponent *playerTransform = nullptr;

    gWorld.forEachWith<TransformComponent, TagComponent>(
        [&](Entity entity, TransformComponent &transform, TagComponent &tag) {
          if (tag.has(PLAYER)) {
            playerEntity = entity;
            playerTransform = &transform;
          }
        });

    if (playerEntity == NULL_ENTITY || playerTransform == nullptr) {
      return;
    }

    gWorld.forEachWith<TransformComponent, CameraFollowComponent>(
        [&](Entity entity, TransformComponent &cameraTransform,
            CameraFollowComponent &follow) {
          glm::vec3 targetPos = playerTransform->position + follow.offset;

          switch (follow.mode) {
          case CameraFollowMode::INSTANT:
            applyInstantFollow(cameraTransform, targetPos, follow);
            break;

          case CameraFollowMode::SMOOTH:
            applySmoothFollow(cameraTransform, targetPos, follow, deltaTime);
            break;

          case CameraFollowMode::DEADZONE:
            applyDeadzoneFollow(cameraTransform, playerTransform->position,
                                follow);
            break;
          }
        });
  }

private:
  void applyInstantFollow(TransformComponent &cameraTransform,
                          const glm::vec3 &targetPos,
                          const CameraFollowComponent &follow) {
    if (follow.followX) {
      cameraTransform.position.x = targetPos.x;
    }
    if (follow.followY) {
      cameraTransform.position.y = targetPos.y;
    }
    if (follow.followZ) {
      cameraTransform.position.z = targetPos.z;
    }
  }

  void applySmoothFollow(TransformComponent &cameraTransform,
                         const glm::vec3 &targetPos,
                         const CameraFollowComponent &follow, float deltaTime) {
    float t = glm::clamp(follow.smoothSpeed * deltaTime, 0.0f, 1.0f);

    if (follow.followX) {
      cameraTransform.position.x =
          glm::mix(cameraTransform.position.x, targetPos.x, t);
    }
    if (follow.followY) {
      cameraTransform.position.y =
          glm::mix(cameraTransform.position.y, targetPos.y, t);
    }
    if (follow.followZ) {
      cameraTransform.position.z =
          glm::mix(cameraTransform.position.z, targetPos.z, t);
    }
  }

  void applyDeadzoneFollow(TransformComponent &cameraTransform,
                           const glm::vec3 &playerPos,
                           const CameraFollowComponent &follow) {

    if (follow.followX && follow.deadzoneX > 0.0f) {
      float diff = playerPos.x - cameraTransform.position.x;
      if (diff > follow.deadzoneX) {
        cameraTransform.position.x = playerPos.x - follow.deadzoneX;
      } else if (diff < -follow.deadzoneX) {
        cameraTransform.position.x = playerPos.x + follow.deadzoneX;
      }
    }

    if (follow.followY) {
      cameraTransform.position.y = playerPos.y + follow.offset.y;
    }

    if (follow.followZ && follow.deadzoneZ > 0.0f) {
      float diff = playerPos.z - cameraTransform.position.z;
      if (diff > follow.deadzoneZ) {
        cameraTransform.position.z = playerPos.z - follow.deadzoneZ;
      } else if (diff < -follow.deadzoneZ) {
        cameraTransform.position.z = playerPos.z + follow.deadzoneZ;
      }
    }
  }
};
