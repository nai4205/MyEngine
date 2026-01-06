#pragma once

#include "ComponentManager.hpp"
#include "ComponentMask.hpp"
#include "Entity.hpp"
#include "EntityManager.hpp"
#include "Input.hpp"
#include "System.hpp"
#include <memory>
#include <vector>

class World {
private:
  std::unique_ptr<EntityManager> entityManager;
  std::unique_ptr<ComponentManager> componentManager;
  std::vector<std::unique_ptr<System>> systems;

  std::vector<ComponentMask> entitySignatures;

  Input input;

public:
  World() {
    entityManager = std::make_unique<EntityManager>();
    componentManager = std::make_unique<ComponentManager>();
    entitySignatures.reserve(MAX_COMPONENTS);
  }

  // ========== INPUT OPERATIONS ==========

  Input &getInput() { return input; }

  // ========== ENTITY OPERATIONS ==========

  Entity createEntity() {
    Entity entity = entityManager->createEntity();
    if (entity >= entitySignatures.size()) {
      entitySignatures.resize(entity + 1);
    }
    entitySignatures[entity].reset();
    return entity;
  }

  // Destroy an entity and remove all its components
  void destroyEntity(Entity entity) {
    entityManager->destroyEntity(entity);

    if (entity < entitySignatures.size()) {
      entitySignatures[entity].reset();
    }
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
    ComponentTypeId typeId = componentManager->getComponentTypeId<T>();
    entitySignatures[entity].reset(typeId);
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

    ComponentMask queryMask;
    (queryMask.set(componentManager->getComponentTypeId<ComponentTypes>()),
     ...);

    for (Entity entity = 0; entity < entitySignatures.size(); ++entity) {
      const ComponentMask &signature = entitySignatures[entity];
      if ((signature & queryMask) == queryMask) {
        result.emplace_back(entity);
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
