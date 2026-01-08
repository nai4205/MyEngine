#pragma once

#include "../ecs/World.hpp"
#include "glm/detail/type_vec.hpp"
#include <string>
#include <vector>

class Scene {
public:
  virtual ~Scene() = default;

  virtual void load(World &world) = 0;

  virtual void unload(World &world) {
    for (Entity e : trackedEntities) {
      world.destroyEntity(e);
    }
    trackedEntities.clear();
  }

  virtual const std::string &getName() const = 0;
  virtual const glm::vec4 &getClearColor() const = 0;

protected:
  void trackEntity(Entity e) { trackedEntities.push_back(e); }

  std::vector<Entity> trackedEntities;
};
