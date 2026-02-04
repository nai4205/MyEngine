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
#include "components/FluidPhysicsComponent.hpp"
#include "components/VelocityComponent.hpp"
#include "systems/FluidPhysicsSystem.hpp"

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
  }

  void initSystems(World &world, float width, float height) {
    world.addSystem<SpriteRenderSystem>(width, height);
    world.addSystem<FluidPhysicsSystem>();

    auto *collision = world.addSystem<CollisionSystem2D>();
    collision->onCollision(CollisionLayer::Ball, CollisionLayer::Wall,
                           WaterSimCollision::onParticleWall);
  }

public:
  WaterSim(float width, float height)
      : screenWidth(width), screenHeight(height) {}

  void load(World &world) override {
    initComponents(world);
    initSystems(world, screenWidth, screenHeight);
    auto &resources = ResourceManager::instance();

    Entity sceneEntity = world.createEntity();
    SceneComponent sceneComp = SceneComponent("WaterSim");
    sceneComp.clearColor = getClearColor();
    TagComponent tagComp;
    tagComp.add(ACTIVESCENE);
    world.addComponent(sceneEntity, sceneComp);
    world.addComponent(sceneEntity, tagComp);

    uint32_t spriteShaderID = resources.loadShader(
        "spriteShader",
        "../src/scenes/breakout/shaders/spriteShaderVertex.glsl",
        "../src/scenes/breakout/shaders/spriteShaderFragment.glsl");

    createCircle(world, resources, spriteShaderID);
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

  void createCircle(World &world, auto &resources, uint32_t spriteShaderID) {
    float radius = 25.0f;
    MeshData circleMesh = resources.createCircleMesh(radius, 32);
    Entity circle = world.createEntity();
    trackEntity(circle);

    TransformComponent transform;
    transform.position =
        glm::vec3(screenWidth / 2.0f, screenHeight / 2.0f, 0.0f);
    transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    world.addComponent(circle, transform);

    MeshComponent mesh;
    mesh.vao = circleMesh.vao;
    mesh.vertexCount = circleMesh.vertexCount;
    mesh.indexCount = circleMesh.indexCount;
    world.addComponent(circle, mesh);

    MaterialComponent material;
    material.shaderProgram = spriteShaderID;
    material.useTextures = false;
    material.color = glm::vec3(1.0f, 0.0f, 0.0f);
    world.addComponent(circle, material);

    FluidPhysicsComponent physics;
    physics.gravity = -500.0f;
    world.addComponent(circle, physics);

    WaterVelocityComponent vel;
    vel.velocity = glm::vec2(0.0f, 100.0f);
    world.addComponent(circle, vel);

    Collider2D collider =
        Collider2D::makeCircle(radius, CollisionLayer::Ball,
                               static_cast<uint32_t>(CollisionLayer::Wall));
    world.addComponent(circle, collider);
  }

  void createBoundaryWalls(World &world) {
    float margin = 50.0f; // Slightly smaller than screen
    float wallThickness = 20.0f;

    float left = margin;
    float right = screenWidth - margin;
    float bottom = margin;
    float top = screenHeight - margin;

    float boxWidth = right - left;
    float boxHeight = top - bottom;

    createWall(world, glm::vec2(left, bottom - wallThickness),
               glm::vec2(boxWidth, wallThickness));

    createWall(world, glm::vec2(left, top), glm::vec2(boxWidth, wallThickness));

    createWall(world, glm::vec2(left - wallThickness, bottom - wallThickness),
               glm::vec2(wallThickness, boxHeight + wallThickness * 2));

    createWall(world, glm::vec2(right, bottom - wallThickness),
               glm::vec2(wallThickness, boxHeight + wallThickness * 2));
  }

  void createWall(World &world, glm::vec2 position, glm::vec2 size) {
    Entity wall = world.createEntity();
    trackEntity(wall);

    TransformComponent transform;
    transform.position = glm::vec3(position, 0.0f);
    transform.scale = glm::vec3(1.0f);
    world.addComponent(wall, transform);

    auto &resources = ResourceManager::instance();
    MeshData wallMesh = resources

        // AABB collider - Wall layer, collides with Ball layer
        Collider2D collider =
            Collider2D::makeAABB(size, CollisionLayer::Wall,
                                 static_cast<uint32_t>(CollisionLayer::Ball));
    world.addComponent(wall, collider);
  }
};
