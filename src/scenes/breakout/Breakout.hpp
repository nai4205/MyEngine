#pragma once

#include "../../components/CameraComponent.hpp"
#include "../../components/MaterialComponent.hpp"
#include "../../components/MeshComponent.hpp"
#include "../../components/TransformComponent.hpp"
#include "../../resources/ResourceManager.hpp"
#include "../Scene.hpp"
#include "./components/GameLevelComponent.hpp"
#include "./systems/LevelManagerSystem.hpp"
#include "./systems/SpriteRenderSystem.hpp"
#include <cstdint>

class Breakout : public Scene {
private:
  void initComponents(World &world) {
    world.registerComponent<MaterialComponent>();
    world.registerComponent<MeshComponent>();
    world.registerComponent<TransformComponent>();
    world.registerComponent<CameraComponent>();
    world.registerComponent<TagComponent>();
    world.registerComponent<SceneComponent>();
    world.registerComponent<GameLevelComponent>();
  }
  void initSystems(World &world, float width, float height) {
    world.addSystem<LevelManagerSystem>();
    world.addSystem<SpriteRenderSystem>(width, height);
  }

public:
  Breakout(float width, float height)
      : screenWidth(width), screenHeight(height) {}

  void load(World &world) override {
    initComponents(world);

    std::cout << "Loading Breakout..." << std::endl;

    auto &resources = ResourceManager::instance();

    // ==== SHADERS ====
    uint32_t spriteShaderID = resources.loadShader(
        "spriteShader",
        "../src/scenes/breakout/shaders/spriteShaderVertex.glsl",
        "../src/scenes/breakout/shaders/spriteShaderFragment.glsl");

    // ==== TEXTURES ====
    uint32_t blockTexture =
        resources.loadTexture("../src/assets/breakout/block.png", false);
    uint32_t blockSolidTexture =
        resources.loadTexture("../src/assets/breakout/block_solid.png", false);

    // ==== SYSTEMS ====
    initSystems(world, screenWidth, screenHeight);

    // ==== SCENE ====
    Entity sceneEntity = world.createEntity();
    SceneComponent sceneComp = SceneComponent("Breakout");
    sceneComp.clearColor = getClearColor();
    TagComponent tagComp;
    tagComp.add(ACTIVESCENE);
    world.addComponent(sceneEntity, sceneComp);
    world.addComponent(sceneEntity, tagComp);

    // ==== LEVEL ====
    Entity levelEntity = world.createEntity();
    GameLevelComponent level;
    level.path = "../src/scenes/breakout/levels/standard.txt";
    level.levelWidth = screenWidth;
    level.levelHeight = screenHeight / 2;
    level.shaderID = spriteShaderID;
    level.blockTexture = blockTexture;
    level.blockSolidTexture = blockSolidTexture;
    world.addComponent(levelEntity, level);
    world.addComponent(levelEntity, TagComponent(ACTIVELEVEL));

    createCamera(world);
  }

  const std::string &getName() const override {
    static std::string name = "Breakout";
    return name;
  }

  const glm::vec4 &getClearColor() const override { return clearColor; }

private:
  float screenWidth;
  float screenHeight;
  glm::vec4 clearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1);

  void createCamera(World &world) {
    Entity camera = world.createEntity();
    trackEntity(camera);

    CameraComponent cam;
    cam.isOrthographic = true;
    cam.orthoWidth = 800.0f;
    cam.orthoHeight = 600.0f;

    world.addComponent(camera, cam);

    TagComponent tag(ACTIVE);
    world.addComponent(camera, tag);
  }
};
