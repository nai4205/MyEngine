#pragma once

#include "Entity.hpp"
#include <queue>

class EntityManager {
private:
  std::queue<Entity> availableEntities;

  uint32_t nextEntityId = 1;

  uint32_t livingEntityCount = 0;

public:
  // Create a new entity and return its ID
  Entity createEntity() {
    Entity id;

    if (!availableEntities.empty()) {
      id = availableEntities.front();
      availableEntities.pop();
    } else {
      id = nextEntityId++;
    }

    livingEntityCount++;
    return id;
  }

  void destroyEntity(Entity entity) {
    if (entity != NULL_ENTITY) {
      availableEntities.push(entity);
      livingEntityCount--;
    }
  }

  uint32_t getLivingEntityCount() const { return livingEntityCount; }
};
