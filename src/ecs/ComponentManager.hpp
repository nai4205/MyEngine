#ifndef COMPONENT_MANAGER_HPP
#define COMPONENT_MANAGER_HPP

#include "ComponentArray.hpp"
#include "ComponentMask.hpp"
#include "Entity.hpp"
#include <cassert>
#include <memory>
#include <typeindex>
#include <unordered_map>

class ComponentManager {
private:
  std::unordered_map<std::type_index, std::shared_ptr<void>> componentArrays;
  std::unordered_map<std::type_index, ComponentTypeId> componentTypeIds;
  ComponentTypeId nextComponentTypeId = 0;

  template <typename T> std::shared_ptr<ComponentArray<T>> getComponentArray() {
    std::type_index typeIndex = std::type_index(typeid(T));

    auto it = componentArrays.find(typeIndex);
    if (it == componentArrays.end()) {
      return nullptr;
    }

    return std::static_pointer_cast<ComponentArray<T>>(it->second);
  }

public:
  template <typename T> void registerComponent() {
    std::type_index typeIndex = std::type_index(typeid(T));

    assert(nextComponentTypeId < MAX_COMPONENTS &&
           "Exceeded maximum number of component types!");

    componentTypeIds[typeIndex] = nextComponentTypeId++;

    componentArrays[typeIndex] = std::make_shared<ComponentArray<T>>();
  }

  template <typename T> ComponentTypeId getComponentTypeId() {
    std::type_index typeIndex = std::type_index(typeid(T));
    auto it = componentTypeIds.find(typeIndex);
    assert(it != componentTypeIds.end() && "Component type not registered!");
    return it->second;
  }

  template <typename T> void addComponent(Entity entity, T component) {
    auto array = getComponentArray<T>();
    if (array) {
      array->insert(entity, component);
    }
  }

  // Remove a component from an entity
  template <typename T> void removeComponent(Entity entity) {
    auto array = getComponentArray<T>();
    if (array) {
      array->remove(entity);
    }
  }

  // Get a component from an entity (returns nullptr if not found)
  template <typename T> T *getComponent(Entity entity) {
    auto array = getComponentArray<T>();
    if (array) {
      return array->get(entity);
    }
    return nullptr;
  }

  template <typename T> bool hasComponent(Entity entity) {
    auto array = getComponentArray<T>();
    if (array) {
      return array->has(entity);
    }
    return false;
  }

  template <typename T> ComponentArray<T> *getArrayForIteration() {
    auto array = getComponentArray<T>();
    return array.get();
  }

  void entityDestroyed(Entity entity) {
    for (auto &pair : componentArrays) {
      // Type-erase by casting to base interface
      // We need to call entityDestroyed on each array
      // This is a bit tricky with type erasure, so we'll handle it differently
      // For now, we'll manually remove from all arrays in World::destroyEntity
    }
  }
};

#endif // COMPONENT_MANAGER_HPP
