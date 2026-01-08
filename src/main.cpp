#include <cstdint>
#include <cstdio>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "GLFW/glfw3.h"
#include "components/CameraComponent.hpp"
#include "components/CameraControllerComponent.hpp"
#include "components/LightingPresets.hpp"
#include "components/MaterialComponent.hpp"
#include "components/MaterialPresets.hpp"
#include "components/MeshComponent.hpp"
#include "components/PhysicsComponent.hpp"
#include "components/TransformComponent.hpp"
#include "const_h.hpp"
#include "ecs/Tag.hpp"
#include "ecs/World.hpp"
#include "glm/detail/type_vec.hpp"
#include "resources/ModelLoader.hpp"
#include "resources/ResourceManager.hpp"
#include "systems/CameraControllerSystem.hpp"
#include "systems/CameraSystem.hpp"
#include "systems/LightingSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "texture_2d_h.hpp"

#define MAX_ENTITIES 20

World gWorld;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);

void createLightEntities(LightingType type,
                         const glm::vec3 pointLightPositions[4],
                         uint32_t lightMeshVAO, uint32_t lightShaderID,
                         std::vector<Entity> &entities);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const glm::vec4 CLEAR_COL = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
bool wireframe = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Learning open gl", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGL()) {
    std::cout << "Failed to initalize GLAD" << std::endl;
    return -1;
  }

  stbi_set_flip_vertically_on_load(true);

  auto &resources = ResourceManager::instance();
  // ========== CREATE SHARED RESOURCES ==========
  uint32_t staticShaderID =
      resources.loadShader("static", "../src/shaders/static/staticVertex.glsl",
                           "../src/shaders/static/staticFragment.glsl");
  // std::shared_ptr<Shader> hdrShader =
  // std::make_shared<Shader>("../src/shaders/hdr/hdrVertex.glsl",
  // "../src/shaders/hdr/hdrFragment.glsl"); unsigned int hdrFBO;
  //  glGenFramebuffers(1, &hdrFBO);
  //  // create floating point color buffer
  //  unsigned int colorBuffer;
  //  glGenTextures(1, &colorBuffer);
  //  glBindTexture(GL_TEXTURE_2D, colorBuffer);
  //  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0,
  //  GL_RGBA, GL_FLOAT, NULL); glTexParameteri(GL_TEXTURE_2D,
  //  GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D,
  //  GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //  // create depth buffer (renderbuffer)
  //  unsigned int rboDepth;
  //  glGenRenderbuffers(1, &rboDepth);
  //  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  //  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH,
  //  SCR_HEIGHT);
  //  // attach buffers
  //  glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
  //  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
  //  GL_TEXTURE_2D, colorBuffer, 0); glFramebufferRenderbuffer(GL_FRAMEBUFFER,
  //  GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth); if
  //  (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  //      std::cout << "Framebuffer not complete!" << std::endl;
  //  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  //
  uint32_t lightSourceShaderID = resources.loadShader(
      "light", "../src/shaders/lightSource/lightSourceVertex.glsl",
      "../src/shaders/lightSource/lightSourceFragment.glsl");

  std::vector<VertexAttribute> cubeLayout = {
      {0, 3, GL_FLOAT, false, 8 * sizeof(float), (void *)0},
      {1, 3, GL_FLOAT, false, 8 * sizeof(float), (void *)(3 * sizeof(float))},
      {2, 2, GL_FLOAT, false, 8 * sizeof(float), (void *)(6 * sizeof(float))}};

  std::vector<VertexAttribute> planeLayout = {
      {0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0},
      {1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
       (void *)(3 * sizeof(float))}};

  MeshData cubeMesh = resources.createMesh(
      cubeVerticesWithTexture, sizeof(cubeVerticesWithTexture), cubeLayout, 36);
  MeshData planeMesh = resources.createMesh(
      planeVertices, sizeof(planeVertices), planeLayout, 6);

  std::vector<VertexAttribute> lightCubeLayout = {
      {0, 3, GL_FLOAT, false, 6 * sizeof(float), (void *)0},
      {1, 3, GL_FLOAT, false, 6 * sizeof(float), (void *)(3 * sizeof(float))}};

  MeshData lightCubeMesh = resources.createMesh(
      lightVertices, sizeof(lightVertices), lightCubeLayout, 36);

  // ========== CREATE TEXTURES ==========
  uint32_t containerDiffuse =
      resources.loadTexture("../src/assets/container2.png");
  uint32_t containerSpecular =
      resources.loadTexture("../src/assets/container2_specular.png");

  // std::shared_ptr<Texture2D> matrixEmission = std::make_shared<Texture2D>();
  // matrixEmission->loadImage("../src/assets/emission.png");
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
  //                 GL_LINEAR_MIPMAP_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //
  // ========== INITIALIZE ECS WORLD ==========

  gWorld.registerComponent<TransformComponent>();
  gWorld.registerComponent<MeshComponent>();
  gWorld.registerComponent<MaterialComponent>();
  gWorld.registerComponent<PhysicsComponent>();
  gWorld.registerComponent<CameraComponent>();
  gWorld.registerComponent<CameraControllerComponent>();
  gWorld.registerComponent<TagComponent>();
  gWorld.registerComponent<DirectionalLightComponent>();
  gWorld.registerComponent<PointLightComponent>();
  gWorld.registerComponent<SpotLightComponent>();

  gWorld.addSystem<CameraControllerSystem>(window);
  gWorld.addSystem<PhysicsSystem>();
  gWorld.addSystem<CameraSystem>();
  gWorld.addSystem<LightingSystem>();
  gWorld.addSystem<RenderSystem>(SCR_WIDTH, SCR_HEIGHT);

  // ========== CREATE ENTITIES ==========
  std::vector<Entity> entities;
  entities.reserve(MAX_ENTITIES);

  // ========== Floor ==========
  Entity floor = gWorld.createEntity();
  TransformComponent floorTransform;
  floorTransform = glm::vec3(0.0f, 0.0f, 0.0f);
  gWorld.addComponent(floor, floorTransform);
  MeshComponent floorMesh;
  floorMesh.vao = planeMesh.vao;
  floorMesh.vertexCount = planeMesh.vertexCount;
  floorMesh.indexCount = planeMesh.indexCount;
  gWorld.addComponent(floor, floorMesh);
  MaterialComponent floorMaterial =
      MaterialPresets::create(staticShaderID, MaterialType::OBSIDIAN);
  // floorMat.shaderProgram = staticShaderID;
  // floorMat.textures[0] = containerDiffuse;
  // floorMat.textures[1] = containerSpecular;
  // floorMat.useTextures = false;
  // floorMat.shininess = 5.0f;
  gWorld.addComponent(floor, floorMaterial);

  for (unsigned int i = 0; i < 10; i++) {
    Entity cube = gWorld.createEntity();

    TransformComponent transform;
    transform.position = cubePositions[i];
    float angle = 20.0f * i;
    transform.rotation = glm::vec3(angle * 0.3f, angle, angle * 0.5f);
    gWorld.addComponent(cube, transform);

    MeshComponent mesh;
    mesh.vao = cubeMesh.vao;
    mesh.vertexCount = cubeMesh.vertexCount;
    mesh.indexCount = 0;
    gWorld.addComponent(cube, mesh);

    MaterialComponent material;
    material.shaderProgram = staticShaderID;
    material.textures[0] = containerDiffuse;
    material.textures[1] = containerSpecular;
    material.useTextures = true;
    material.shininess = 32.0f;
    gWorld.addComponent(cube, material);

    PhysicsComponent physComp;
    gWorld.addComponent(cube, physComp);
  }

  // ========== CREATE LIGHTING SETUP ==========
  LightingType currentLighting = LightingType::HORROR;
  glm::vec4 clearColor = LightingPresets::getClearColor(currentLighting);

  // ========== LOAD MODEL ==========
  std::cout << "Loading model..." << std::endl;
  std::vector<Entity> backpackEntities = ModelLoader::load(
      gWorld, "../src/assets/backpack/backpack.obj", staticShaderID,
      glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(1.0f));

  for (auto entity : backpackEntities) {
    entities.emplace_back(entity);
  }
  std::cout << "Model loaded: " << backpackEntities.size() << " meshes"
            << std::endl;

  // ========== CREATE LIGHT ENTITIES ==========
  createLightEntities(currentLighting, pointLightPositions, lightCubeMesh.vao,
                      lightSourceShaderID, entities);

  // ========== CREATE CAMERA ENTITY ==========
  Entity cameraEntity = gWorld.createEntity();

  TransformComponent cameraTransform;
  cameraTransform.position = glm::vec3(0.0f, 0.0f, 3.0f);
  gWorld.addComponent(cameraEntity, cameraTransform);

  CameraComponent camera(-90.0f, 0.0f, 45.0f);
  gWorld.addComponent(cameraEntity, camera);

  CameraControllerComponent controller(2.5f, 0.1f, 5.0f, false); // FPS mode
  gWorld.addComponent(cameraEntity, controller);

  PhysicsComponent cameraPhysics(-9.81f, 0.0f);
  gWorld.addComponent(cameraEntity, cameraPhysics);

  TagComponent cameraTag(ACTIVE);
  gWorld.addComponent(cameraEntity, cameraTag);

  entities.emplace_back(cameraEntity);

  glEnable(GL_DEPTH_TEST);

  while (!glfwWindowShouldClose(window)) {
    // Calculate delta time
    float currentFrame = static_cast<float>(glfwGetTime());
    const float &deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    gWorld.getInput().newFrame();
    glfwPollEvents();

    gWorld.update(deltaTime);

    // Update viewPos for material (used by lighting calculations)
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gWorld.render();

    // static int frameCount = 0;
    // if (++frameCount % 60 == 0) {
    //   auto stats = gWorld.getCacheStats();
    //   std::cout << "Cache Stats - Queries: " << stats.totalQueries
    //             << ", Dirty: " << stats.dirtyQueries
    //             << ", Cached Entities: " << stats.totalCachedEntities <<
    //             "\n";
    // }

    // Swap buffers
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}

void createLightEntities(LightingType type,
                         const glm::vec3 pointLightPositions[4],
                         uint32_t lightMeshVAO, uint32_t lightShaderID,
                         std::vector<Entity> &entities) {
  auto props = LightingPresets::getProperties(type);

  Entity dirLight = gWorld.createEntity();

  DirectionalLightComponent dirComp(
      props.dirLightDirection, props.dirLightAmbient, props.dirLightDiffuse,
      props.dirLightSpecular);
  gWorld.addComponent(dirLight, dirComp);

  entities.emplace_back(dirLight);

  for (int i = 0; i < 4; i++) {
    Entity pointLight = gWorld.createEntity();

    TransformComponent transform;
    transform.position = pointLightPositions[i];
    transform.scale = glm::vec3(0.2f);
    gWorld.addComponent(pointLight, transform);

    auto &plConfig = props.pointLights[i];
    PointLightComponent lightComp(
        plConfig.color * plConfig.ambientMultiplier, plConfig.color,
        plConfig.color, plConfig.constant, plConfig.linear, plConfig.quadratic);
    gWorld.addComponent(pointLight, lightComp);

    MeshComponent mesh;
    mesh.vao = lightMeshVAO;
    mesh.vertexCount = 36;
    mesh.indexCount = 0;
    gWorld.addComponent(pointLight, mesh);

    MaterialComponent material;
    material.shaderProgram = lightShaderID;
    material.diffuse = plConfig.color;
    material.useTextures = false;
    material.receivesLighting = false; // Light sources don't receive lighting
    gWorld.addComponent(pointLight, material);

    entities.emplace_back(pointLight);
  }

  Entity spotlight = gWorld.createEntity();

  TransformComponent spotTransform;
  gWorld.addComponent(spotlight, spotTransform);

  auto &slConfig = props.spotlight;
  SpotLightComponent spotComp(
      slConfig.ambient, slConfig.diffuse, slConfig.specular, slConfig.constant,
      slConfig.linear, slConfig.quadratic, slConfig.cutOffDegrees,
      slConfig.outerCutOffDegrees, true);
  gWorld.addComponent(spotlight, spotComp);

  entities.emplace_back(spotlight);
}
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if ((action == GLFW_PRESS || action == GLFW_RELEASE)) {
    gWorld.getInput().setKeyPressed(key, action == GLFW_PRESS);
  }

  if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
    wireframe = !wireframe;
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
  }

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}
