#pragma once

#include "ComponentArray.hpp"
#include "ComponentMask.hpp"
#include "Entity.hpp"
#include <cassert>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

class ComponentManager {
private:
  std::vector<std::unique_ptr<IComponentArray>> componentArrays;
  std::unordered_map<std::type_index, ComponentTypeId> componentTypeIds;
  ComponentTypeId nextComponentTypeId = 0;

  template <typename T> ComponentArray<T> *getComponentArray() {
    ComponentTypeId id = getComponentTypeId<T>();
    return static_cast<ComponentArray<T> *>(componentArrays[id].get());
  }

public:
  template <typename T> void registerComponent() {
    std::type_index typeIndex(typeid(T));
    assert(componentTypeIds.find(typeIndex) == componentTypeIds.end());

    ComponentTypeId id = nextComponentTypeId++;
    componentTypeIds[typeIndex] = id;

    if (id >= componentArrays.size()) {
      componentArrays.resize(id + 1);
    }

    componentArrays[id] = std::make_unique<ComponentArray<T>>();
  }

  template <typename T> ComponentTypeId getComponentTypeId() {
    std::type_index typeIndex = std::type_index(typeid(T));
    auto it = componentTypeIds.find(typeIndex);
    assert(it != componentTypeIds.end() && "Component type not registered!");
    return it->second;
  }

  template <typename T> void addComponent(Entity e, T component) {
    getComponentArray<T>()->insert(e, component);
  }

  template <typename T> void removeComponent(Entity e) {
    getComponentArray<T>()->remove(e);
  }

  template <typename T> T *getComponent(Entity e) {
    return getComponentArray<T>()->get(e);
  }
  template <typename T> bool hasComponent(Entity e) {
    return getComponentArray<T>()->has(e);
  }
};
