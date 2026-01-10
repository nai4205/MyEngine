
#pragma once

#include "../components/CameraComponent.hpp"
#include "../components/CameraControllerComponent.hpp"
#include "../components/DirectionalLightComponent.hpp"
#include "../components/LightingPresets.hpp"
#include "../components/MaterialComponent.hpp"
#include "../components/MaterialPresets.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/PhysicsComponent.hpp"
#include "../components/PointLightComponent.hpp"
#include "../components/SceneComponent.hpp"
#include "../components/SpotLightComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../const_h.hpp"
#include "../ecs/Tag.hpp"
#include "../resources/ModelLoader.hpp"
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

    MeshData cubeMesh =
        resources.createMesh(cubeVerticesWithTexture,
                             sizeof(cubeVerticesWithTexture), cubeLayout, 36);
    MeshData lightCubeMesh = resources.createMesh(
        lightVertices, sizeof(lightVertices), lightCubeLayout, 36);

    uint32_t containerDiffuse =
        resources.loadTexture("../src/assets/container2.png");
    uint32_t containerSpecular =
        resources.loadTexture("../src/assets/container2_specular.png");

    createCubes(world, cubeMesh, staticShaderID, containerDiffuse,
                containerSpecular);

    createLights(world, lightCubeMesh, lightSourceShaderID);

    createCamera(world);

    glm::vec3 clearColor = getClearColor();
    resources.setClearColorForFramebuffer("Scene2D", clearColor);
  }

  const std::string &getName() const override {
    static std::string name = "Scene2D";
    return name;
  }

  const glm::vec4 &getClearColor() const override { return clearColor; }

private:
  float screenWidth;
  float screenHeight;

  static constexpr LightingType sceneTheme = LightingType::DESERT;

  LightingType currentLighting = sceneTheme;
  glm::vec4 clearColor = LightingPresets::getClearColor(sceneTheme);

  void createCubes(World &world, const MeshData &mesh, uint32_t shaderID,
                   uint32_t diffuseTex, uint32_t specularTex) {
    for (unsigned int i = 0; i < 10; i++) {
      Entity cube = world.createEntity();
      trackEntity(cube);

      TransformComponent transform;
      // 2D platformer: fixed X (depth), varying Z (horizontal), fixed Y
      // (height)
      transform.position = glm::vec3(10.0f, 0.0f, i - 4.5f);
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
      material.useTextures = true;
      material.shininess = 32.0f;
      world.addComponent(cube, material);

      // PhysicsComponent physics;
      // world.addComponent(cube, physics);

      // TagComponent tag(OUTLINED);
      // tag.add(MODEL);
      // world.addComponent(cube, tag);
    }
  }

  void createLights(World &world, const MeshData &lightMesh,
                    uint32_t lightShaderID) {
    auto props = LightingPresets::getProperties(currentLighting);

    Entity dirLight = world.createEntity();
    trackEntity(dirLight);

    DirectionalLightComponent dirComp(
        props.dirLightDirection, props.dirLightAmbient, props.dirLightDiffuse,
        props.dirLightSpecular);
    world.addComponent(dirLight, dirComp);

    for (int i = 0; i < 4; i++) {
      Entity pointLight = world.createEntity();
      trackEntity(pointLight);

      TransformComponent transform;
      transform.position = pointLightPositions[i];
      transform.scale = glm::vec3(0.2f);
      world.addComponent(pointLight, transform);

      auto &plConfig = props.pointLights[i];
      PointLightComponent lightComp(plConfig.color * plConfig.ambientMultiplier,
                                    plConfig.color, plConfig.color,
                                    plConfig.constant, plConfig.linear,
                                    plConfig.quadratic);
      world.addComponent(pointLight, lightComp);

      MeshComponent mesh;
      mesh.vao = lightMesh.vao;
      mesh.vertexCount = 36;
      mesh.indexCount = 0;
      world.addComponent(pointLight, mesh);

      MaterialComponent material;
      material.shaderProgram = lightShaderID;
      material.diffuse = plConfig.color;
      material.useTextures = false;
      material.receivesLighting = false;
      material.doubleSided = true;
      world.addComponent(pointLight, material);
    }

    Entity spotlight = world.createEntity();
    trackEntity(spotlight);

    TransformComponent spotTransform;
    world.addComponent(spotlight, spotTransform);

    auto &slConfig = props.spotlight;
    SpotLightComponent spotComp(
        slConfig.ambient, slConfig.diffuse, slConfig.specular,
        slConfig.constant, slConfig.linear, slConfig.quadratic,
        slConfig.cutOffDegrees, slConfig.outerCutOffDegrees, true);
    // world.addComponent(spotlight, spotComp);
  }

  void createCamera(World &world) {
    Entity camera = world.createEntity();
    trackEntity(camera);

    TransformComponent transform;
    transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    world.addComponent(camera, transform);

    CameraComponent cam(0.0f, 0.0f, 45.0f);
    world.addComponent(camera, cam);

    CameraControllerComponent controller(2.5f, 0.1f, 5.0f, false);
    world.addComponent(camera, controller);

    TagComponent tag(ACTIVE);
    world.addComponent(camera, tag);
  }
};
