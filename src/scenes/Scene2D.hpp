
#pragma once

#include "../components/CameraComponent.hpp"
#include "../components/CameraControllerComponent.hpp"
#include "../components/DirectionalLightComponent.hpp"
#include "../components/LightingPresets.hpp"
#include "../components/MaterialComponent.hpp"
#include "../components/MaterialPresets.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/PhysicsComponent.hpp"
#include "../components/PlayerControllerComponent.hpp"
#include "../components/PointLightComponent.hpp"
#include "../components/SceneComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../const_h.hpp"
#include "../ecs/Tag.hpp"
#include "../resources/ResourceManager.hpp"
#include "Scene.hpp"
#include <iostream>

class Scene2D : public Scene {
public:
  Scene2D(float width, float height)
      : screenWidth(width), screenHeight(height) {}

  void load(World &world) override {
    std::cout << "Loading Scene2D..." << std::endl;
    Entity sceneEntity = world.createEntity();
    SceneComponent sceneComp = SceneComponent("Scene2D");
    sceneComp.clearColor = getClearColor();
    TagComponent tagComp;
    tagComp.add(ACTIVESCENE);
    world.addComponent(sceneEntity, sceneComp);
    world.addComponent(sceneEntity, tagComp);

    auto &resources = ResourceManager::instance();

    // Create framebuffer for post-processing
    resources.createFramebuffer("Scene2D", screenWidth, screenHeight);

    // Load shaders
    uint32_t staticShaderID = resources.loadShader(
        "static", "../src/shaders/static/staticVertex.glsl",
        "../src/shaders/static/staticFragment.glsl");

    uint32_t lightSourceShaderID = resources.loadShader(
        "light", "../src/shaders/lightSource/lightSourceVertex.glsl",
        "../src/shaders/lightSource/lightSourceFragment.glsl");

    uint32_t singleColorShaderID = resources.loadShader(
        "singleColor", "../src/shaders/stencil/shaderSingleColorVertex.glsl",
        "../src/shaders/stencil/shaderSingleColorFrag.glsl");

    uint32_t postProcessShaderID = resources.loadShader(
        "postprocess", "../src/shaders/postprocess/screenVertex.glsl",
        "../src/shaders/postprocess/screenFragment.glsl");

    std::vector<VertexAttribute> cubeLayout = {
        {0, 3, GL_FLOAT, false, 8 * sizeof(float), (void *)0},
        {1, 3, GL_FLOAT, false, 8 * sizeof(float), (void *)(3 * sizeof(float))},
        {2, 2, GL_FLOAT, false, 8 * sizeof(float),
         (void *)(6 * sizeof(float))}};

    std::vector<VertexAttribute> cubeLayoutNoTexture = {
        {0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0},
        {1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
         (void *)(3 * sizeof(float))}};

    std::vector<VertexAttribute> lightCubeLayout = {
        {0, 3, GL_FLOAT, false, 6 * sizeof(float), (void *)0},
        {1, 3, GL_FLOAT, false, 6 * sizeof(float),
         (void *)(3 * sizeof(float))}};

    std::vector<VertexAttribute> planeLayout = {
        {0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0},
        {1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
         (void *)(3 * sizeof(float))}};
    MeshData cubeMesh =
        resources.createMesh(cubeVerticesWithTexture,
                             sizeof(cubeVerticesWithTexture), cubeLayout, 36);
    MeshData lightCubeMesh = resources.createMesh(
        lightVertices, sizeof(lightVertices), lightCubeLayout, 36);
    MeshData planeMesh = resources.createMesh(
        planeVertices, sizeof(planeVertices), planeLayout, 6);

    uint32_t containerDiffuse =
        resources.loadTexture("../src/assets/container2.png");
    uint32_t containerSpecular =
        resources.loadTexture("../src/assets/container2_specular.png");

    createLights(world, lightCubeMesh, lightSourceShaderID);
    createCubes(world, cubeMesh, staticShaderID, containerDiffuse,
                containerSpecular);

    createBackdrop(world, planeMesh, staticShaderID);

    // createLights(world, lightCubeMesh, lightSourceShaderID);

    createCamera(world);
  }

  const std::string &getName() const override {
    static std::string name = "Scene2D";
    return name;
  }

  const glm::vec4 &getClearColor() const override { return clearColor; }

private:
  float screenWidth;
  float screenHeight;

  static constexpr LightingType sceneTheme = LightingType::BIOCHEMICAL_LAB;

  LightingType currentLighting = sceneTheme;
  glm::vec4 clearColor = LightingPresets::getClearColor(sceneTheme);

  void createCubes(World &world, const MeshData &mesh, uint32_t shaderID,
                   uint32_t diffuseTex, uint32_t specularTex) {
    // Different colors for the point lights
    std::vector<glm::vec3> colors = {
        glm::vec3(1.0f, 0.0f, 0.0f), // Red
        glm::vec3(0.0f, 1.0f, 0.0f), // Green
        glm::vec3(0.0f, 0.0f, 1.0f), // Blue
        glm::vec3(1.0f, 1.0f, 0.0f), // Yellow
        glm::vec3(1.0f, 0.0f, 1.0f), // Magenta
        glm::vec3(0.0f, 1.0f, 1.0f), // Cyan
        glm::vec3(1.0f, 0.5f, 0.0f), // Orange
        glm::vec3(0.5f, 0.0f, 1.0f), // Purple
        glm::vec3(1.0f, 1.0f, 1.0f), // White
        glm::vec3(1.0f, 0.75f, 0.8f) // Pink
    };

    for (unsigned int i = 0; i < 10; i++) {
      Entity cube = world.createEntity();

      trackEntity(cube);

      TransformComponent transform;
      transform.position = glm::vec3(0.0f, -2.0f, (i - 4.5f) * 2.5f);
      transform.scale = glm::vec3(0.3f, 0.5f, 2.0f);
      world.addComponent(cube, transform);

      MeshComponent meshComp;
      meshComp.vao = mesh.vao;
      meshComp.vertexCount = mesh.vertexCount;
      meshComp.indexCount = 0;
      world.addComponent(cube, meshComp);

      MaterialComponent material;
      material.shaderProgram = shaderID;
      material.textures[0] = diffuseTex;
      material.textures[1] = specularTex;
      material.useTextures = false;
      material.shininess = 32.0f;
      world.addComponent(cube, material);

      // Create point light 1 unit above this cube
      Entity pointLight = world.createEntity();
      trackEntity(pointLight);

      TransformComponent lightTransform;
      lightTransform.position = glm::vec3(0.0f, -0.5f, (i - 4.5f) * 2.5f);
      lightTransform.scale = glm::vec3(0.2f);
      world.addComponent(pointLight, lightTransform);

      PointLightComponent lightComp(colors[i] * 0.2f, // Ambient
                                    colors[i],        // Diffuse
                                    colors[i],        // Specular
                                    1.0f,             // Constant
                                    0.1f,             // Linear
                                    0.032f);          // Quadratic
      world.addComponent(pointLight, lightComp);
    }

    // Create player character on platform index 5 (middle platform at z=1.25)
    Entity player = world.createEntity();
    trackEntity(player);

    TransformComponent transformPlayer;
    // Platforms: x=0, y=-2, scale=(0.3, 0.5, 2.0)
    // Platform top: y = -2 + 0.5/2 = -1.75
    // Player cube (scale 0.5): center at y = -1.75 + 0.5/2 = -1.5
    // Platform 5 is at z = (5 - 4.5) * 2.5 = 1.25
    transformPlayer.position = glm::vec3(0.0f, -1.0f, 1.25f);
    transformPlayer.scale = glm::vec3(0.5f, 0.5f, 0.5f);
    world.addComponent(player, transformPlayer);

    MeshComponent playerMeshComp;
    playerMeshComp.vao = mesh.vao;
    playerMeshComp.vertexCount = mesh.vertexCount;
    playerMeshComp.indexCount = 0;
    world.addComponent(player, playerMeshComp);

    MaterialComponent playerMaterial;
    playerMaterial.shaderProgram = shaderID;
    playerMaterial.textures[0] = diffuseTex;
    playerMaterial.textures[1] = specularTex;
    playerMaterial.useTextures = false; // Use textures to make it visible
    playerMaterial.shininess = 32.0f;
    world.addComponent(player, playerMaterial);

    PhysicsComponent physComp(-15.0f, -1.5f);
    world.addComponent(player, physComp);
    // Add player controller for keyboard input
    PlayerControllerComponent2D playerController(5.0f, 8.0f, false);
    world.addComponent(player, playerController);
  }

  void createBackdrop(World &world, const MeshData &mesh, uint32_t shaderID) {
    Entity floor = world.createEntity();
    trackEntity(floor);

    TransformComponent transform;
    transform.position = glm::vec3(5.0f, 0.0f, 0.0f);
    transform.rotation = glm::vec3(-90, 0, -90);
    transform.scale *= 2;
    world.addComponent(floor, transform);

    MeshComponent meshComp;
    meshComp.vao = mesh.vao;
    meshComp.vertexCount = mesh.vertexCount;
    meshComp.indexCount = mesh.indexCount;
    world.addComponent(floor, meshComp);

    MaterialComponent material =
        MaterialPresets::create(shaderID, MaterialType::OBSIDIAN);
    material.doubleSided = true;
    world.addComponent(floor, material);
  }

  void createLights(World &world, const MeshData &lightMesh,
                    uint32_t lightShaderID) {
    Entity dirLight = world.createEntity();
    trackEntity(dirLight);

    glm::vec3 lightDirection = glm::normalize(glm::vec3(1.0f, -0.3f, 0.0f));

    DirectionalLightComponent dirLightComp(
        lightDirection, glm::vec3(0.1f, 0.1f, 0.1f),
        glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.1f, 0.1f, 0.1f));
    world.addComponent(dirLight, dirLightComp);
  }

  void createCamera(World &world) {
    Entity camera = world.createEntity();
    trackEntity(camera);

    TransformComponent transform;
    transform.position = glm::vec3(-10.0f, 3.0f, 0.0f); // Back and above
    world.addComponent(camera, transform);

    // 2.5D Perspective camera: yaw=0 (look along +X), pitch=-15 (slight
    // downward angle)
    CameraComponent cam(0.0f, -15.0f, 45.0f);
    world.addComponent(camera, cam);

    // CameraControllerComponent controller(2.5f, 0.1f, 5.0f, false);
    // world.addComponent(camera, controller);

    TagComponent tag(ACTIVE);
    world.addComponent(camera, tag);
  }
};
