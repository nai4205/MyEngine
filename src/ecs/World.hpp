#pragma once

#include "ComponentManager.hpp"
#include "ComponentMask.hpp"
#include "Entity.hpp"
#include "EntityManager.hpp"
#include "Input.hpp"
#include "System.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

class World {
private:
  std::unique_ptr<EntityManager> entityManager;
  std::unique_ptr<ComponentManager> componentManager;
  std::vector<std::unique_ptr<System>> systems;

  // Entity signatures - bitmask of which components each entity has
  std::unordered_map<Entity, ComponentMask> entitySignatures;

  // Input resource - accessible from any system
  Input input;

public:
  World() {
    entityManager = std::make_unique<EntityManager>();
    componentManager = std::make_unique<ComponentManager>();
  }

  // ========== INPUT OPERATIONS ==========

  Input &getInput() { return input; }

  // ========== ENTITY OPERATIONS ==========

  Entity createEntity() { return entityManager->createEntity(); }

  // Destroy an entity and remove all its components
  void destroyEntity(Entity entity) {
    entityManager->destroyEntity(entity);

    // Remove entity signature
    entitySignatures.erase(entity);
  }

  uint32_t getLivingEntityCount() const {
    return entityManager->getLivingEntityCount();
  }

  // ========== COMPONENT OPERATIONS ==========

  template <typename T> void registerComponent() {
    componentManager->registerComponent<T>();
  }

  template <typename T> void addComponent(Entity entity, T component) {
    componentManager->addComponent<T>(entity, component);

    ComponentTypeId typeId = componentManager->getComponentTypeId<T>();
    entitySignatures[entity].set(typeId);
  }

  template <typename T> void removeComponent(Entity entity) {
    componentManager->removeComponent<T>(entity);

    // Update entity signature - clear the bit for this component type
    ComponentTypeId typeId = componentManager->getComponentTypeId<T>();
    auto it = entitySignatures.find(entity);
    if (it != entitySignatures.end()) {
      it->second.reset(typeId);
    }
  }

  template <typename T> T *getComponent(Entity entity) {
    return componentManager->getComponent<T>(entity);
  }

  template <typename T> bool hasComponent(Entity entity) {
    return componentManager->hasComponent<T>(entity);
  }

  // ========== QUERY OPERATIONS ==========
  template <typename... ComponentTypes> std::vector<Entity> getEntitiesWith() {
    std::vector<Entity> result;

    if constexpr (sizeof...(ComponentTypes) == 0) {
      return result;
    }

    // Build the query mask - set bits for all requested component types
    ComponentMask queryMask;
    (queryMask.set(componentManager->getComponentTypeId<ComponentTypes>()),
     ...);

    // Iterate through all entity signatures and check with bitwise AND
    for (const auto &[entity, signature] : entitySignatures) {
      // Check if entity has ALL the requested components using bitwise AND
      // (signature & queryMask) == queryMask means entity has all components
      if ((signature & queryMask) == queryMask) {
        result.push_back(entity);
      }
    }

    return result;
  }

  // ========== SYSTEM OPERATIONS ==========

  template <typename T, typename... Args> T *addSystem(Args &&...args) {
    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    T *systemPtr = system.get();
    systems.push_back(std::move(system));
    return systemPtr;
  }

  void update(float deltaTime) {
    for (auto &system : systems) {
      system->update(deltaTime);
    }
  }

  void render() {
    for (auto &system : systems) {
      system->render();
    }
  }
};
