#pragma once

#include "../../components/CameraComponent.hpp"
#include "../../components/MaterialComponent.hpp"
#include "../../components/MeshComponent.hpp"
#include "../../components/TransformComponent.hpp"
#include "../../resources/ResourceManager.hpp"
#include "../Scene.hpp"
#include "./components/GameLevelComponent.hpp"
#include "./components/PlayerComponent.hpp"
#include "./systems/LevelManagerSystem.hpp"
#include "./systems/PlayerMovementSystem.hpp"
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
    world.registerComponent<PlayerComponent>();
  }
  void initSystems(World &world, float width, float height) {
    world.addSystem<LevelManagerSystem>();
    world.addSystem<PlayerMovementSystem>(width);
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

    // ==== PLAYER ====
    uint32_t paddleTexture =
        resources.loadTexture("../src/assets/breakout/paddle.png", false);
    createPlayer(world, spriteShaderID, paddleTexture);

    // ==== CAMERA ====
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

  void createPlayer(World &world, uint32_t shaderID, uint32_t textureID) {
    const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);

    Entity player = world.createEntity();

    PlayerComponent playerComp;
    playerComp.velocity = 500.0f;
    playerComp.sizeX = PLAYER_SIZE.x;
    playerComp.sizeY = PLAYER_SIZE.y;
    world.addComponent(player, playerComp);

    TransformComponent transform;
    transform.position = glm::vec3(screenWidth / 2.0f - PLAYER_SIZE.x / 2.0f,
                                   screenHeight - PLAYER_SIZE.y, 0.0f);
    transform.scale = glm::vec3(PLAYER_SIZE, 1.0f);
    world.addComponent(player, transform);

    MaterialComponent material;
    material.shaderProgram = shaderID;
    material.textures[0] = textureID;
    material.useTextures = (textureID != 0);
    material.color = glm::vec3(1.0f);
    world.addComponent(player, material);

    float vertices[] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};
    std::vector<VertexAttribute> layout = {
        {0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0}};
    auto &resources = ResourceManager::instance();
    MeshData mesh = resources.createMesh(vertices, sizeof(vertices), layout, 6);

    MeshComponent meshComp;
    meshComp.vao = mesh.vao;
    meshComp.vertexCount = mesh.vertexCount;
    meshComp.indexCount = 0;
    world.addComponent(player, meshComp);
  }

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
