#pragma once
#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../../components/TransformComponent.hpp"
#include "../components/Collider2D.hpp"
#include "../components/CollisionEvent.hpp"
#include <vector>

extern World gWorld;

struct CollisionEntity {
  Entity entity;
  Collider2D *collider;
  TransformComponent *transform;
};

class CollisionSystem2D : public System {
public:
  std::vector<CollisionEvent> events;

  void update(float &deltaTime) override {
    events.clear();

    std::vector<CollisionEntity> entities;
    gWorld.forEachWith<Collider2D, TransformComponent>(
        [&](Entity entity, Collider2D &collider,
            TransformComponent &transform) {
          entities.push_back({entity, &collider, &transform});
        });

    for (size_t i = 0; i < entities.size(); i++) {
      for (size_t j = i + 1; j < entities.size(); j++) {
        CollisionEvent event;
        if (checkCollision(entities[i], entities[j], event)) {
          events.push_back(event);
        }
      }
    }
  }

private:
  bool checkCollision(const CollisionEntity &a, const CollisionEntity &b,
                      CollisionEvent &event) {
    auto typeA = a.collider->type;
    auto typeB = b.collider->type;

    bool collision = false;
    glm::vec2 normal(0.0f);

    if (typeA == ColliderType::AABB && typeB == ColliderType::AABB) {
      collision = checkAABBvsAABB(a, b, normal);
      event = {a.entity, b.entity, normal, 0.0f};
    } else if (typeA == ColliderType::Circle && typeB == ColliderType::Circle) {
      collision = checkCirclevsCircle(a, b, normal);
      event = {a.entity, b.entity, normal, 0.0f};
    } else if (typeA == ColliderType::Circle && typeB == ColliderType::AABB) {
      collision = checkCirclevsAABB(a, b, normal);
      event = {a.entity, b.entity, normal, 0.0f};
    } else if (typeA == ColliderType::AABB && typeB == ColliderType::Circle) {
      collision = checkCirclevsAABB(b, a, normal);
      event = {b.entity, a.entity, normal, 0.0f};
    }

    return collision;
  }

  bool checkAABBvsAABB(const CollisionEntity &a, const CollisionEntity &b,
                       glm::vec2 &normal) {
    glm::vec2 posA = glm::vec2(a.transform->position);
    glm::vec2 sizeA = a.collider->halfExtents * 2.0f;
    glm::vec2 posB = glm::vec2(b.transform->position);
    glm::vec2 sizeB = b.collider->halfExtents * 2.0f;

    bool collisionX = posA.x + sizeA.x >= posB.x && posB.x + sizeB.x >= posA.x;
    bool collisionY = posA.y + sizeA.y >= posB.y && posB.y + sizeB.y >= posA.y;

    if (collisionX && collisionY) {
      glm::vec2 centerA = posA + a.collider->halfExtents;
      glm::vec2 centerB = posB + b.collider->halfExtents;
      glm::vec2 diff = centerB - centerA;
      if (glm::length(diff) > 0.0f) {
        normal = glm::normalize(diff);
      }
      return true;
    }
    return false;
  }

  bool checkCirclevsCircle(const CollisionEntity &a, const CollisionEntity &b,
                           glm::vec2 &normal) {
    glm::vec2 centerA = glm::vec2(a.transform->position) + a.collider->radius;
    glm::vec2 centerB = glm::vec2(b.transform->position) + b.collider->radius;

    glm::vec2 diff = centerB - centerA;
    float distance = glm::length(diff);
    float radiusSum = a.collider->radius + b.collider->radius;

    if (distance < radiusSum) {
      if (distance > 0.0f) {
        normal = glm::normalize(diff);
      }
      return true;
    }
    return false;
  }

  bool checkCirclevsAABB(const CollisionEntity &circle,
                         const CollisionEntity &aabb, glm::vec2 &normal) {
    // Get center point of circle
    glm::vec2 center =
        glm::vec2(circle.transform->position) + circle.collider->radius;

    // Calculate AABB info (center, half-extents)
    glm::vec2 aabbHalfExtents = aabb.collider->halfExtents;
    glm::vec2 aabbCenter =
        glm::vec2(aabb.transform->position) + aabbHalfExtents;

    // Get difference vector between both centers
    glm::vec2 difference = center - aabbCenter;
    glm::vec2 clamped =
        glm::clamp(difference, -aabbHalfExtents, aabbHalfExtents);

    // Add clamped value to AABB_center to get closest point on AABB
    glm::vec2 closest = aabbCenter + clamped;

    // Check if distance from circle center to closest point < radius
    difference = closest - center;
    float distance = glm::length(difference);

    if (distance < circle.collider->radius) {
      if (distance > 0.0f) {
        normal = glm::normalize(difference);
      }
      return true;
    }
    return false;
  }
};
