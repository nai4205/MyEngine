#pragma once

#include "ComponentManager.hpp"
#include "ComponentMask.hpp"
#include "Entity.hpp"
#include "EntityManager.hpp"
#include "Input.hpp"
#include "System.hpp"
#include "../components/NameComponent.hpp"
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

class World {
private:
  std::unique_ptr<EntityManager> entityManager;
  std::unique_ptr<ComponentManager> componentManager;
  std::vector<std::unique_ptr<System>> systems;

  std::vector<ComponentMask> entitySignatures;

  struct QueryCache {
    std::vector<Entity> entities;
    bool dirty = true;
  };

  mutable std::unordered_map<ComponentMask, QueryCache> queryCache;

  std::unordered_map<ComponentTypeId, std::vector<ComponentMask>>
      componentToQueryMasks;

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

    invalidateAllQueries();

    return entity;
  }

  void destroyEntity(Entity entity) {
    if (entity == NULL_ENTITY)
      return;

    ComponentMask oldSignature;
    if (entity < entitySignatures.size()) {
      oldSignature = entitySignatures[entity];
      entitySignatures[entity].reset();
    }

    entityManager->destroyEntity(entity);

    invalidateQueriesForMask(oldSignature);
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

    if (entity >= entitySignatures.size()) {
      entitySignatures.resize(entity + 1);
    }
    entitySignatures[entity].set(typeId);

    invalidateQueriesForComponentType(typeId);
  }

  template <typename T> void removeComponent(Entity entity) {
    ComponentTypeId typeId = componentManager->getComponentTypeId<T>();

    componentManager->removeComponent<T>(entity);
    entitySignatures[entity].reset(typeId);

    invalidateQueriesForComponentType(typeId);
  }

  template <typename T> T *getComponent(Entity entity) {
    return componentManager->getComponent<T>(entity);
  }

  template <typename T> bool hasComponent(Entity entity) {
    return componentManager->hasComponent<T>(entity);
  }

  // ========== ENTITY SEARCH OPERATIONS ==========

  Entity findEntityByName(const std::string &name) {
    const auto &entities = getEntitiesWith<NameComponent>();
    for (Entity entity : entities) {
      auto *nameComp = getComponent<NameComponent>(entity);
      if (nameComp && nameComp->name == name) {
        return entity;
      }
    }
    return NULL_ENTITY;
  }

  std::vector<Entity> findEntitiesByNameContains(const std::string &substring) {
    std::vector<Entity> result;
    const auto &entities = getEntitiesWith<NameComponent>();
    for (Entity entity : entities) {
      auto *nameComp = getComponent<NameComponent>(entity);
      if (nameComp && nameComp->name.find(substring) != std::string::npos) {
        result.push_back(entity);
      }
    }
    return result;
  }

  // ========== OPTIMIZED QUERY OPERATIONS ==========
  template <typename... ComponentTypes>
  const std::vector<Entity> &getEntitiesWith() {
    static_assert(sizeof...(ComponentTypes) > 0,
                  "Must query for at least one component type");

    ComponentMask queryMask = buildQueryMask<ComponentTypes...>();

    auto it = queryCache.find(queryMask);
    if (it == queryCache.end()) {
      // First time seeing this query - create cache entry and register it
      queryCache[queryMask] = QueryCache{};
      registerQueryMask<ComponentTypes...>(queryMask);
      it = queryCache.find(queryMask);
    }

    QueryCache &cache = it->second;

    if (cache.dirty) {
      rebuildQueryCache(queryMask, cache);
    }

    return cache.entities;
  }

  template <typename... ComponentTypes>
  std::vector<Entity> getEntitiesWithUncached() {
    std::vector<Entity> result;

    if constexpr (sizeof...(ComponentTypes) == 0) {
      return result;
    }

    ComponentMask queryMask = buildQueryMask<ComponentTypes...>();

    for (Entity entity = 0; entity < entitySignatures.size(); ++entity) {
      const ComponentMask &signature = entitySignatures[entity];
      if ((signature & queryMask) == queryMask) {
        result.emplace_back(entity);
      }
    }

    return result;
  }

  template <typename... ComponentTypes, typename Func>
  void forEachEntity(Func &&callback) {
    const auto &entities = getEntitiesWith<ComponentTypes...>();
    for (Entity entity : entities) {
      callback(entity);
    }
  }

  template <typename... ComponentTypes, typename Func>
  void forEachWith(Func &&callback) {
    const auto &entities = getEntitiesWith<ComponentTypes...>();
    for (Entity entity : entities) {
      callback(entity, *getComponent<ComponentTypes>(entity)...);
    }
  }

  // ========== CACHE CONTROL ==========

  void invalidateAllQueries() {
    for (auto &[mask, cache] : queryCache) {
      cache.dirty = true;
    }
  }

  struct CacheStats {
    size_t totalQueries;
    size_t dirtyQueries;
    size_t totalCachedEntities;
  };

  CacheStats getCacheStats() const {
    CacheStats stats{0, 0, 0};
    stats.totalQueries = queryCache.size();
    for (const auto &[mask, cache] : queryCache) {
      if (cache.dirty)
        stats.dirtyQueries++;
      stats.totalCachedEntities += cache.entities.size();
    }
    return stats;
  }

  // ========== SYSTEM OPERATIONS ==========

  template <typename T, typename... Args> T *addSystem(Args &&...args) {
    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    T *systemPtr = system.get();
    systems.emplace_back(std::move(system));
    return systemPtr;
  }

  template <typename T> T *getSystem() {
    for (auto &system : systems) {
      if (T *castedSystem = dynamic_cast<T *>(system.get())) {
        return castedSystem;
      }
    }
    return nullptr;
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

private:
  // ========== QUERY CACHE HELPERS ==========
  template <typename... ComponentTypes> ComponentMask buildQueryMask() {
    ComponentMask mask;
    (mask.set(componentManager->getComponentTypeId<ComponentTypes>()), ...);
    return mask;
  }

  template <typename... ComponentTypes>
  void registerQueryMask(const ComponentMask &queryMask) {
    (registerMaskForComponent<ComponentTypes>(queryMask), ...);
  }

  template <typename T>
  void registerMaskForComponent(const ComponentMask &queryMask) {
    ComponentTypeId typeId = componentManager->getComponentTypeId<T>();
    auto &masks = componentToQueryMasks[typeId];

    if (std::find(masks.begin(), masks.end(), queryMask) == masks.end()) {
      masks.emplace_back(queryMask);
    }
  }

  void invalidateQueriesForComponentType(ComponentTypeId typeId) {
    auto it = componentToQueryMasks.find(typeId);
    if (it != componentToQueryMasks.end()) {
      for (const ComponentMask &mask : it->second) {
        auto cacheIt = queryCache.find(mask);
        if (cacheIt != queryCache.end()) {
          cacheIt->second.dirty = true;
        }
      }
    }
  }

  void invalidateQueriesForMask(const ComponentMask &entityMask) {
    for (auto &[queryMask, cache] : queryCache) {
      if ((entityMask & queryMask) == queryMask) {
        cache.dirty = true;
      }
    }
  }

  void rebuildQueryCache(const ComponentMask &queryMask, QueryCache &cache) {
    cache.entities.clear();
    cache.entities.reserve(entitySignatures.size() / 4);

    for (Entity entity = 0; entity < entitySignatures.size(); ++entity) {
      const ComponentMask &signature = entitySignatures[entity];
      if ((signature & queryMask) == queryMask) {
        cache.entities.emplace_back(entity);
      }
    }

    cache.dirty = false;
  }
};
