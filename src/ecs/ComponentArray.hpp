#ifndef COMPONENT_ARRAY_HPP
#define COMPONENT_ARRAY_HPP

#include "Entity.hpp"
#include <unordered_map>

// Template class for storing components of a specific type
template <typename T> class ComponentArray {
private:
  std::unordered_map<Entity, T> components;

public:
  // Insert a component for an entity
  void insert(Entity entity, T component) { components[entity] = component; }

  // Remove a component from an entity
  void remove(Entity entity) { components.erase(entity); }

  // Get a component for an entity (returns nullptr if not found)
  T *get(Entity entity) {
    auto it = components.find(entity);
    if (it != components.end()) {
      return &it->second;
    }
    return nullptr;
  }

  // Check if an entity has this component
  bool has(Entity entity) const {
    return components.find(entity) != components.end();
  }

  auto begin() { return components.begin(); }
  auto end() { return components.end(); }
  auto begin() const { return components.begin(); }
  auto end() const { return components.end(); }

  size_t size() const { return components.size(); }

  void entityDestroyed(Entity entity) {
    if (has(entity)) {
      remove(entity);
    }
  }
};

#endif // COMPONENT_ARRAY_HPP
