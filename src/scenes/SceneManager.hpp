#pragma once

#include "Scene.hpp"
#include <memory>
#include <unordered_map>

class SceneManager {
public:
  static SceneManager &instance() {
    static SceneManager inst;
    return inst;
  }

  template <typename T, typename... Args>
  void registerScene(const std::string &name, Args &&...args) {
    scenes[name] = std::make_unique<T>(std::forward<Args>(args)...);
  }

  void loadScene(const std::string &name, World &world) {
    if (currentScene) {
      currentScene->unload(world);
    }

    auto it = scenes.find(name);
    if (it != scenes.end()) {
      currentScene = it->second.get();
      currentScene->load(world);
    }
  }

  Scene *getCurrentScene() { return currentScene; }

private:
  SceneManager() = default;
  std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
  Scene *currentScene = nullptr;
};
