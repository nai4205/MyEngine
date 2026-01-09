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
#include "../components/SpotLightComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../const_h.hpp"
#include "../ecs/Tag.hpp"
#include "../resources/ModelLoader.hpp"
#include "../resources/ResourceManager.hpp"
#include "Scene.hpp"

class MainScene : public Scene {
public:
  void load(World &world) override {
    auto &resources = ResourceManager::instance();

    uint32_t staticShaderID = resources.loadShader(
        "static", "../src/shaders/static/staticVertex.glsl",
        "../src/shaders/static/staticFragment.glsl");

    uint32_t lightSourceShaderID = resources.loadShader(
        "light", "../src/shaders/lightSource/lightSourceVertex.glsl",
        "../src/shaders/lightSource/lightSourceFragment.glsl");

    uint32_t singleColorShaderID = resources.loadShader(
        "singleColor", "../src/shaders/stencil/shaderSingleColorVertex.glsl",
        "../src/shaders/stencil/shaderSingleColorFrag.glsl");

    std::vector<VertexAttribute> cubeLayout = {
        {0, 3, GL_FLOAT, false, 8 * sizeof(float), (void *)0},
        {1, 3, GL_FLOAT, false, 8 * sizeof(float), (void *)(3 * sizeof(float))},
        {2, 2, GL_FLOAT, false, 8 * sizeof(float),
         (void *)(6 * sizeof(float))}};

    std::vector<VertexAttribute> cubeLayoutNoTexture = {
        {0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0},
        {1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
         (void *)(3 * sizeof(float))}};

    std::vector<VertexAttribute> planeLayout = {
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
    MeshData planeMesh = resources.createMesh(
        planeVertices, sizeof(planeVertices), planeLayout, 6);
    MeshData lightCubeMesh = resources.createMesh(
        lightVertices, sizeof(lightVertices), lightCubeLayout, 36);
    MeshData grassMesh = resources.createMesh(
        grassVertices, sizeof(grassVertices), cubeLayout, 6);

    uint32_t containerDiffuse =
        resources.loadTexture("../src/assets/container2.png");
    uint32_t containerSpecular =
        resources.loadTexture("../src/assets/container2_specular.png");

    uint32_t grassTexture = resources.loadTexture("../src/assets/grass.png");
    uint32_t windowTexture = resources.loadTexture("../src/assets/window.png");

    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    createFloor(world, planeMesh, staticShaderID);

    createCubes(world, cubeMesh, staticShaderID, containerDiffuse,
                containerSpecular);

    // createGrass(world, grassMesh, staticShaderID, grassTexture);

    createGrass(world, grassMesh, staticShaderID, windowTexture,
                true); // Windows are transparent

    createLights(world, lightCubeMesh, lightSourceShaderID);

    createBackpack(world, staticShaderID);

    createCamera(world);
  }

  const std::string &getName() const override {
    static std::string name = "MainScene";
    return name;
  }

  const glm::vec4 &getClearColor() const override { return clearColor; }

private:
  static constexpr LightingType sceneTheme = LightingType::DESERT;

  LightingType currentLighting = sceneTheme;
  glm::vec4 clearColor = LightingPresets::getClearColor(sceneTheme);

  void createFloor(World &world, const MeshData &mesh, uint32_t shaderID) {
    Entity floor = world.createEntity();
    trackEntity(floor);

    TransformComponent transform;
    transform.position = glm::vec3(0.0f);
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

  void createCubes(World &world, const MeshData &mesh, uint32_t shaderID,
                   uint32_t diffuseTex, uint32_t specularTex) {
    for (unsigned int i = 0; i < 10; i++) {
      Entity cube = world.createEntity();
      trackEntity(cube);

      TransformComponent transform;
      transform.position = cubePositions[i];
      float angle = 20.0f * i;
      transform.rotation = glm::vec3(angle * 0.3f, angle, angle * 0.5f);
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

      PhysicsComponent physics;
      world.addComponent(cube, physics);

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

  void createBackpack(World &world, uint32_t shaderID) {
    std::vector<Entity> backpackEntities = ModelLoader::load(
        world, "../src/assets/backpack/backpack.obj", shaderID,
        glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f));

    for (Entity e : backpackEntities) {
      // TagComponent tag;
      // tag.add(OUTLINED);
      // world.addComponent(e, tag);
      trackEntity(e);
    }
  }

  void createGrass(World &world, const MeshData &mesh, uint32_t shaderID,
                   uint32_t grassTexture, bool isTransparent = false) {
    for (unsigned int i = 0; i < 10; i++) {
      Entity grass = world.createEntity();
      trackEntity(grass);

      TransformComponent transform;
      transform.position = glm::vec3(i, -0.5, i);
      world.addComponent(grass, transform);

      MeshComponent meshComp;
      meshComp.vao = mesh.vao;
      meshComp.vertexCount = mesh.vertexCount;
      meshComp.indexCount = 0;
      world.addComponent(grass, meshComp);

      MaterialComponent material;
      material.shaderProgram = shaderID;
      material.textures[0] = grassTexture;
      material.useTextures = true;
      material.shininess = 32.0f;
      material.hasTransparency = isTransparent;
      world.addComponent(grass, material);

      // TagComponent tag;
      // tag.add(OUTLINED);
      // world.addComponent(grass, tag);
    }
  }

  void createCamera(World &world) {
    Entity camera = world.createEntity();
    trackEntity(camera);

    TransformComponent transform;
    transform.position = glm::vec3(0.0f, 0.0f, 3.0f);
    world.addComponent(camera, transform);

    CameraComponent cam(-90.0f, 0.0f, 45.0f);
    world.addComponent(camera, cam);

    CameraControllerComponent controller(2.5f, 0.1f, 5.0f, false);
    world.addComponent(camera, controller);

    PhysicsComponent physics(-9.81f, 0.0f);
    world.addComponent(camera, physics);

    TagComponent tag(ACTIVE);
    world.addComponent(camera, tag);
  }
};
