#pragma once

#include "../../components/CameraComponent.hpp"
#include "../../components/MaterialComponent.hpp"
#include "../../components/MeshComponent.hpp"
#include "../../components/TransformComponent.hpp"
#include "../../resources/ResourceManager.hpp"
#include "../Scene.hpp"
#include "./components/GameLevelComponent.hpp"
#include "./components/PlayerComponent.hpp"
#include "./systems/BallMovementSystem.hpp"
#include "./systems/BreakoutRenderSystem.hpp"
#include "./systems/CollisionSystem2D.hpp"
#include "./systems/LevelManagerSystem.hpp"
#include "./systems/PlayerMovementSystem.hpp"
#include "CollisionHandlers.hpp"
#include "components/BallComponent.hpp"
#include "components/BrickComponent.hpp"
#include "components/Collider2D.hpp"
#include "components/ParticleEmitterComponent.hpp"
#include "components/PostProcessingComponent.hpp"
#include "components/PowerUpComponent.hpp"
#include "components/VelocityComponent.hpp"
#include "systems/ParticleSystem.hpp"
#include "systems/PostProcessingSystem.hpp"
#include "systems/PowerUpSystem.hpp"
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
    world.registerComponent<BallComponent>();
    world.registerComponent<BrickComponent>();
    world.registerComponent<Collider2D>();
    world.registerComponent<VelocityComponent>();
    world.registerComponent<ParticleEmitterComponent>();
    world.registerComponent<PostProcessingComponent>();
    world.registerComponent<PowerUpComponent>();
  }
  void initSystems(World &world, float width, float height, uint32_t shaderID,
                   const PowerUpTextures &powerUpTextures) {
    world.addSystem<LevelManagerSystem>();
    world.addSystem<PlayerMovementSystem>(width);
    world.addSystem<BreakoutRenderSystem>(width, height);
    world.addSystem<BallMovementSystem>(width);

    // PowerUp system with pre-allocated pool
    world.addSystem<PowerUpSystem>(width, height, shaderID, powerUpTextures);

    // Collision system and handler callbacks
    auto *collision = world.addSystem<CollisionSystem2D>();
    CollisionHandlers::registerAll(collision);

    world.addSystem<ParticleSystem>(width, height);

    // Post-processing must be last - renders framebuffer to screen
    world.addSystem<PostProcessingSystem>(width, height);
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
    uint32_t particleTextureID =
        resources.loadTexture("../src/assets/breakout/particle.png", false);

    // PowerUp textures
    uint32_t texSpeed = resources.loadTexture(
        "../src/assets/breakout/powerup_speed.png", false);
    uint32_t texSticky = resources.loadTexture(
        "../src/assets/breakout/powerup_sticky.png", false);
    uint32_t texPassThrough = resources.loadTexture(
        "../src/assets/breakout/powerup_passthrough.png", false);
    uint32_t texPadSize = resources.loadTexture(
        "../src/assets/breakout/powerup_increase.png", false);
    uint32_t texConfuse = resources.loadTexture(
        "../src/assets/breakout/powerup_confuse.png", false);
    uint32_t texChaos = resources.loadTexture(
        "../src/assets/breakout/powerup_chaos.png", false);

    // ==== SYSTEMS ====
    PowerUpTextures powerUpTextures;
    powerUpTextures.speed = texSpeed;
    powerUpTextures.sticky = texSticky;
    powerUpTextures.passThrough = texPassThrough;
    powerUpTextures.padSize = texPadSize;
    powerUpTextures.confuse = texConfuse;
    powerUpTextures.chaos = texChaos;

    initSystems(world, screenWidth, screenHeight, spriteShaderID,
                powerUpTextures);

    // ==== SCENE ====
    Entity sceneEntity = world.createEntity();
    SceneComponent sceneComp = SceneComponent("Breakout");
    sceneComp.clearColor = getClearColor();
    TagComponent tagComp;
    tagComp.add(ACTIVESCENE);
    PostProcessingComponent fx;
    world.addComponent(sceneEntity, sceneComp);
    world.addComponent(sceneEntity, tagComp);
    world.addComponent(sceneEntity, fx);

    // ==== LEVEL ====
    Entity standardLevel = world.createEntity();
    Entity spaceInvaderLevel = world.createEntity();
    Entity bounceGaloreLevel = world.createEntity();
    GameLevelComponent level;
    level.path = "../src/scenes/breakout/levels/standard.txt";
    level.levelWidth = screenWidth;
    level.levelHeight = screenHeight / 2;
    level.shaderID = spriteShaderID;
    level.blockTexture = blockTexture;
    level.blockSolidTexture = blockSolidTexture;
    level.particleTextureID = particleTextureID;
    world.addComponent(standardLevel, level);

    level.path = "../src/scenes/breakout/levels/space_invader.txt";
    world.addComponent(spaceInvaderLevel, level);

    level.path = "../src/scenes/breakout/levels/bounce_galore.txt";
    world.addComponent(bounceGaloreLevel, level);

    world.addComponent(spaceInvaderLevel, TagComponent(ACTIVELEVEL));

    // ==== PLAYER ==== //
    uint32_t paddleTexture =
        resources.loadTexture("../src/assets/breakout/paddle.png", false);
    createPlayer(world, spriteShaderID, paddleTexture);

    // ==== BALL ==== //
    uint32_t ballTexture =
        resources.loadTexture("../src/assets/breakout/smiley.png", false);

    createBall(world, spriteShaderID, ballTexture, particleTextureID);

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
    playerComp.speed = 500.0f;
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

    // AABB collider for the paddle - collides with ball and powerups
    world.addComponent(
        player,
        Collider2D::makeAABB(PLAYER_SIZE, CollisionLayer::Player,
                             CollisionLayer::Ball | CollisionLayer::PowerUp));
  }

  void createBall(World &world, uint32_t shaderID, uint32_t textureID,
                  uint32_t particleTextureID) {
    Entity ball = world.createEntity();

    BallComponent ballComp;
    ballComp.radius = 25.0f;
    ballComp.stuck = false;
    world.addComponent(ball, ballComp);

    VelocityComponent velComp;
    velComp.velocity = glm::vec2(200.0f, 300.0f);
    world.addComponent(ball, velComp);

    // Circle collider for the ball - collides with bricks and player
    world.addComponent(
        ball,
        Collider2D::makeCircle(ballComp.radius, CollisionLayer::Ball,
                               CollisionLayer::Brick | CollisionLayer::Player));

    TransformComponent transform;
    transform.position =
        glm::vec3(screenWidth / 2.0f - ballComp.radius,
                  screenHeight - 20.0f - ballComp.radius * 2.0f, 0.0f);

    transform.scale =
        glm::vec3(ballComp.radius * 2.0f, ballComp.radius * 2.0f, 1.0f);

    world.addComponent(ball, transform);

    MaterialComponent material;
    material.shaderProgram = shaderID;
    material.textures[0] = textureID;
    material.useTextures = (textureID != 0);
    material.color = glm::vec3(1.0f);
    world.addComponent(ball, material);

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
    world.addComponent(ball, meshComp);

    // Particle emitter - particles trail behind the ball
    ParticleEmitterComponent emitter;
    emitter.maxParticles = 500;
    emitter.spawnRate = 2;
    emitter.offset = glm::vec2(ballComp.radius / 2.0f);
    emitter.particleSize = glm::vec2(10.0f);
    emitter.gravity = glm::vec2(0.0f, -250.0f);
    emitter.shaderID = shaderID;
    emitter.textureID = particleTextureID;
    emitter.meshVAO = mesh.vao;
    emitter.meshVertexCount = mesh.vertexCount;
    world.addComponent(ball, emitter);
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
