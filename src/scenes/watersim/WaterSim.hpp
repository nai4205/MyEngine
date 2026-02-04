#pragma once
#include "../../components/CameraComponent.hpp"
#include "../../components/MaterialComponent.hpp"
#include "../../components/MeshComponent.hpp"
#include "../../components/SceneComponent.hpp"
#include "../../components/TransformComponent.hpp"
#include "../../resources/ResourceManager.hpp"
#include "../../systems/SpriteRenderSystem.hpp"
#include "../Scene.hpp"
#include "../breakout/components/Collider2D.hpp"
#include "../breakout/systems/CollisionSystem2D.hpp"
#include "CollisionHandlers.hpp"
#include "components/SimulatorSettings.hpp"
#include "systems/FluidPhysicsSystem.hpp"
#include <vector>

class WaterSim : public Scene {
private:
  void initComponents(World &world) {
    world.registerComponent<MaterialComponent>();
    world.registerComponent<MeshComponent>();
    world.registerComponent<TransformComponent>();
    world.registerComponent<CameraComponent>();
    world.registerComponent<TagComponent>();
    world.registerComponent<SceneComponent>();
    world.registerComponent<FluidPhysicsComponent>();
    world.registerComponent<WaterVelocityComponent>();
    world.registerComponent<Collider2D>();
    world.registerComponent<ParticleComponent>();
  }

  void initSystems(World &world, float width, float height, uint32_t shaderID) {
    world.addSystem<SpriteRenderSystem>(width, height);

    auto *fluidPhysics =
        world.addSystem<FluidPhysicsSystem>(width, height, shaderID, settings);
    fluidPhysics->createParticles();

    auto *collision = world.addSystem<CollisionSystem2D>();
    collision->onCollision(CollisionLayer::Ball, CollisionLayer::Wall,
                           WaterSimCollision::onParticleWall);
  }

public:
  WaterSim(float width, float height)
      : screenWidth(width), screenHeight(height) {}

  void load(World &world) override {
    initComponents(world);
    auto &resources = ResourceManager::instance();

    spriteShaderID = resources.loadShader(
        "spriteShader",
        "../src/scenes/breakout/shaders/spriteShaderVertex.glsl",
        "../src/scenes/breakout/shaders/spriteShaderFragment.glsl");

    initSystems(world, screenWidth, screenHeight, spriteShaderID);

    Entity sceneEntity = world.createEntity();
    SceneComponent sceneComp = SceneComponent("WaterSim");
    sceneComp.clearColor = getClearColor();
    TagComponent tagComp;
    tagComp.add(ACTIVESCENE);
    world.addComponent(sceneEntity, sceneComp);
    world.addComponent(sceneEntity, tagComp);

    createBoundaryWalls(world);
    createCamera(world);
  }

  const std::string &getName() const override {
    static std::string name = "WaterSim";
    return name;
  }

  const glm::vec4 &getClearColor() const override { return clearColor; }

private:
  float screenWidth;
  float screenHeight;
  uint32_t spriteShaderID;
  glm::vec4 clearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1);
  SimulatorSettings settings;

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

  void createBoundaryWalls(World &world) {
    float margin = settings.boxMargin;
    float wallThickness = settings.wallThickness;

    float left = margin;
    float right = screenWidth - margin;
    float bottom = margin;
    float top = screenHeight - margin;

    float boxWidth = right - left;
    float boxHeight = top - bottom;

    createWall(world, glm::vec2(left, bottom - wallThickness),
               glm::vec2(boxWidth, wallThickness), spriteShaderID);

    createWall(world, glm::vec2(left, top), glm::vec2(boxWidth, wallThickness),
               spriteShaderID);

    createWall(world, glm::vec2(left - wallThickness, bottom - wallThickness),
               glm::vec2(wallThickness, boxHeight + wallThickness * 2),
               spriteShaderID);

    createWall(world, glm::vec2(right, bottom - wallThickness),
               glm::vec2(wallThickness, boxHeight + wallThickness * 2),
               spriteShaderID);
  }

  void createWall(World &world, glm::vec2 position, glm::vec2 size,
                  float spriteShaderID) {
    Entity wall = world.createEntity();
    trackEntity(wall);

    auto &resources = ResourceManager::instance();
    std::vector<Vertex> vertices = {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // bottom-left
        {{size.x, 0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {1.0f, 0.0f}}, // bottom-right
        {{size.x, size.y, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // top-right
        {{0.0f, size.y, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},   // top-left
    };
    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
    MeshData wallMesh = resources.createIndexedMesh(vertices, indices);

    TransformComponent transform;
    transform.position = glm::vec3(position, 0.0f);
    transform.scale = glm::vec3(1.0f);
    world.addComponent(wall, transform);

    MeshComponent mesh;
    mesh.vao = wallMesh.vao;
    mesh.vertexCount = wallMesh.vertexCount;
    mesh.indexCount = wallMesh.indexCount;
    world.addComponent(wall, mesh);

    MaterialComponent material;
    material.shaderProgram = spriteShaderID;
    material.useTextures = false;
    material.color = glm::vec3(0.4f, 0.4f, 0.4f);
    world.addComponent(wall, material);

    // AABB collider - Wall layer, collides with Ball layer
    Collider2D collider =
        Collider2D::makeAABB(size, CollisionLayer::Wall,
                             static_cast<uint32_t>(CollisionLayer::Ball));
    world.addComponent(wall, collider);
  }
};
