#pragma once

#include "../components/CameraComponent.hpp"
#include "../components/MaterialComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../resources/ResourceManager.hpp"
#include "../systems/breakout/SpriteRenderSystem.hpp"
#include "Scene.hpp"
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
  }
  void initSystems(World &world, float width, float height) {
    world.addSystem<SpriteRenderSystem>(width, height);
  }

public:
  Breakout(float width, float height)
      : screenWidth(width), screenHeight(height) {}

  void load(World &world) override {
    initComponents(world);
    initSystems(world, screenWidth, screenHeight);

    // ==== SCENE AND FRAMEBUFFER ====
    std::cout << "Loading Breakout..." << std::endl;
    Entity sceneEntity = world.createEntity();
    SceneComponent sceneComp = SceneComponent("Breakout");
    sceneComp.clearColor = getClearColor();
    TagComponent tagComp;
    tagComp.add(ACTIVESCENE);
    world.addComponent(sceneEntity, sceneComp);
    world.addComponent(sceneEntity, tagComp);

    float spriteVertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};

    auto &resources = ResourceManager::instance();
    // ==== SHADERS ==== //
    uint32_t spriteShaderID = resources.loadShader(
        "spriteShader", "../src/shaders/breakout/spriteShaderVertex.glsl",
        "../src/shaders/breakout/spriteShaderFragment.glsl");

    // ==== VERTEX LAYOUTS ==== //
    std::vector<VertexAttribute> spriteLayout = {
        {0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0}};

    // ==== MAKE MESHES ==== //
    MeshData spriteMesh = resources.createMesh(
        spriteVertices, sizeof(spriteVertices), spriteLayout, 24);

    // ==== MAKE TEXTURES ==== //
    uint32_t smileyTexture =
        resources.loadTexture("../src/assets/breakout/smiley.png");

    createSprite(world, spriteMesh, spriteShaderID, smileyTexture);
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

  void createSprite(World &world, const MeshData &mesh, uint32_t shaderID,
                    uint32_t textureID) {
    Entity sprite = world.createEntity();

    TransformComponent transform;
    transform.position = glm::vec3(200.0f, 200.0f, 0.0f);
    transform.scale = glm::vec3(300.0f, 400.0f, 0.0f);
    world.addComponent(sprite, transform);

    MaterialComponent material;
    material.shaderProgram = shaderID;
    material.textures[0] = textureID;
    material.useTextures = true;
    material.diffuse = glm::vec3(1.0f);
    world.addComponent(sprite, material);

    MeshComponent meshComp;
    meshComp.vao = mesh.vao;
    meshComp.vertexCount = mesh.vertexCount;
    meshComp.indexCount = 0;
    world.addComponent(sprite, meshComp);
  }

  void createCamera(World &world) {
    Entity camera = world.createEntity();
    trackEntity(camera);

    TransformComponent transform;
    transform.position = glm::vec3(-10.0f, 3.0f, 0.0f); // Back and above
    world.addComponent(camera, transform);

    CameraComponent cam;
    cam.isOrthographic = true;
    cam.orthoWidth = 800.0f;
    cam.orthoHeight = 600.0f;

    world.addComponent(camera, cam);

    TagComponent tag(ACTIVE);
    world.addComponent(camera, tag);
  }
};
