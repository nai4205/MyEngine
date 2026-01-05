#include <cstdio>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include "GLFW/glfw3.h"
#include "GameObject.hpp"
#include "components/LightManager.hpp"
#include "components/LightingPresets.hpp"
#include "components/MaterialPresets.hpp"
#include "components/Mesh.hpp"
#include "components/MeshFactory.hpp"
#include "components/Model.hpp"
#include "const_h.hpp"
#include "glm/detail/type_vec.hpp"
#include "shader_h.hpp"
#include "texture_2d_h.hpp"

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
int cleanup();

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const glm::vec4 CLEAR_COL = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
bool wireframe = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
GameObject *cameraObject = nullptr;

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
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoaderLoadGL()) {
    std::cout << "Failed to initalize GLAD" << std::endl;
    return -1;
  }

  stbi_set_flip_vertically_on_load(true);

  // ========== CREATE SHARED RESOURCES ==========
  std::shared_ptr<Shader> staticShader =
      std::make_shared<Shader>("../src/shaders/static/staticVertex.glsl",
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
  std::shared_ptr<Shader> lightSourceShader = std::make_shared<Shader>(
      "../src/shaders/lightSource/lightSourceVertex.glsl",
      "../src/shaders/lightSource/lightSourceFragment.glsl");

  std::shared_ptr<Mesh> cubeMeshWithTexture =
      MeshFactory::createPositionNormalTexMesh(cubeVerticesWithTexture,
                                               sizeof(cubeVerticesWithTexture));

  // Simple cube mesh for light sources (position + normals, shader only uses
  // position)
  std::shared_ptr<Mesh> lightCubeMesh = MeshFactory::createPositionNormalMesh(
      lightVertices, sizeof(lightVertices));

  // ========== CREATE TEXTURES ==========
  std::shared_ptr<Texture2D> containerTexture = std::make_shared<Texture2D>();
  containerTexture->loadImage("../src/assets/container2.png");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  std::shared_ptr<Texture2D> containerSpecular = std::make_shared<Texture2D>();
  containerSpecular->loadImage("../src/assets/container2_specular.png");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  std::shared_ptr<Texture2D> matrixEmission = std::make_shared<Texture2D>();
  matrixEmission->loadImage("../src/assets/emission.png");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // ========== CREATE MATERIALS ==========
  std::shared_ptr<Material> staticMaterial =
      MaterialPresets::create(staticShader, MaterialType::CHROME);

  staticMaterial->setTexture("material.texture_diffuse1", containerTexture, 0);
  staticMaterial->setTexture("material.texture_specular1", containerSpecular,
                             1);
  staticMaterial->setTexture("material.emission", matrixEmission, 2);
  staticMaterial->setBool("material.useTex", true);

  // ========== CREATE GAME OBJECTS ==========
  std::vector<GameObject> objects;
  for (unsigned int i = 0; i < 10; i++) {
    GameObject cube("Cube" + std::to_string(i));
    cube.transform.position = cubePositions[i];

    float angle = 20.0f * i;
    cube.transform.rotation = glm::vec3(angle * 0.3f, angle, angle * 0.5f);

    cube.addMeshRenderer(cubeMeshWithTexture, staticMaterial, 36);
    // Cube is a heavy object, move prevents copying the whole thing to a new
    // function
    objects.push_back(std::move(cube));
  }

  // ========== CREATE LIGHTING SETUP ==========
  LightingType currentLighting = LightingType::DESERT;
  glm::vec4 clearColor = LightingPresets::getClearColor(currentLighting);

  LightManager lightManager;
  LightingPresets::applyToLightManager(lightManager, currentLighting,
                                       pointLightPositions);

  // Get the lighting properties to access point light colors
  auto lightingProps = LightingPresets::getProperties(currentLighting);

  // ========== LOAD MODEL ==========
  std::cout << "Loading model..." << std::endl;
  Model model("../src/assets/backpack/backpack.obj", staticShader);
  std::vector<GameObject> modelObjects = model.moveGameObjects();

  for (auto &obj : modelObjects) {
    obj.transform.position = glm::vec3(5.0f, 0.0f, 0.0f);
    obj.transform.scale = glm::vec3(1.0f);
    obj.transform.rotation = glm::vec3(0, 0, 0);
    // add to objects to be rendered
    objects.push_back(obj);
  }
  std::cout << "Model loaded: " << modelObjects.size() << " meshes"
            << std::endl;

  // ========== CREATE LIGHT SOURCE VISUALIZATIONS ==========
  for (int i = 0; i < 4; i++) {
    GameObject lightCube("PointLight" + std::to_string(i));
    lightCube.transform.position = pointLightPositions[i];
    lightCube.transform.scale = glm::vec3(0.2f);

    auto lightMaterial = std::make_shared<Material>(lightSourceShader);
    lightMaterial->setVec3("objectColor", lightingProps.pointLights[i].color);

    lightCube.addMeshRenderer(lightCubeMesh, lightMaterial, 36);
    objects.push_back(std::move(lightCube));
  }

  GameObject cameraGameObject("MainCamera");
  cameraGameObject.transform.position = glm::vec3(0.0f, 0.0f, 3.0f);
  cameraGameObject.addFPSCamera();

  cameraObject = &cameraGameObject;

  std::vector<GameObject *> gameObjects;
  for (auto &object : objects) {
    gameObjects.push_back(&object);
  }
  // // Add backpack to scene
  // for (auto &backpackObj : backpackObjects) {
  //   gameObjects.push_back(&backpackObj);
  // }
  //
  glEnable(GL_DEPTH_TEST);

  while (!glfwWindowShouldClose(window)) {
    // per-frame time logic
    // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    // -----
    processInput(window);

    // render
    // ------
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate view/projection matrices (same for all objects)
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);

    if (cameraObject && cameraObject->hasCamera()) {
      Camera *cam = cameraObject->getCamera();
      cameraObject->updatePhysics(deltaTime);
      float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
      projection = cam->getProjectionMatrix(aspectRatio, 0.1f, 100.0f);
      view = cam->getViewMatrix(cameraObject->transform.position);
      staticMaterial->setVec3("viewPos", cameraObject->transform.position);

      // Update spotlight to follow camera (flashlight effect)
      auto spotlight = lightManager.getSpotLight();
      if (spotlight) {
        spotlight->active = false;
        spotlight->position = cameraObject->transform.position;
        spotlight->direction = cam->front;
      }
    }

    lightManager.applyAllToShader(staticShader);

    for (size_t i = 0; i < gameObjects.size(); i++) {
      GameObject *obj = gameObjects[i];
      if (obj->hasPhysics()) {
        obj->updatePhysics(deltaTime);
      }
      if (obj && obj->hasMeshRenderer()) {
        // Set projection and view matrices
        auto meshRenderer = obj->getMeshRenderer();
        auto meshRendererIndexed = obj->getMeshRendererIndexed();

        if (meshRenderer) {
          meshRenderer->getMaterial()->setMat4("projection", projection);
          meshRenderer->getMaterial()->setMat4("view", view);
        } else if (meshRendererIndexed) {
          meshRendererIndexed->getMaterial()->setMat4("projection", projection);
          meshRendererIndexed->getMaterial()->setMat4("view", view);
        }

        obj->render();
      }
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (!cameraObject || !cameraObject->hasCameraController())
    return;

  CameraController *controller = cameraObject->getCameraController();
  Camera *camera = cameraObject->getCamera();
  Physics *physics = cameraObject->getPhysics();

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    controller->processKeyboard(CAM_FORWARD, cameraObject->transform, *camera,
                                physics, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    controller->processKeyboard(CAM_BACKWARD, cameraObject->transform, *camera,
                                physics, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    controller->processKeyboard(CAM_LEFT, cameraObject->transform, *camera,
                                physics, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    controller->processKeyboard(CAM_RIGHT, cameraObject->transform, *camera,
                                physics, deltaTime);

  static bool spaceWasPressed = false;
  bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
  if (spacePressed && !spaceWasPressed) {
    controller->processJump(physics);
  }
  spaceWasPressed = spacePressed;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (!cameraObject || !cameraObject->hasCameraController())
    return;

  static bool firstMouse = true;
  static float lastX = 400, lastY = 300;
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }
  float xOffset = xpos - lastX;
  float yOffset = lastY - ypos;

  lastX = xpos;
  lastY = ypos;

  cameraObject->getCameraController()->processMouseMovement(
      *cameraObject->getCamera(), xOffset, yOffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
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

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  if (!cameraObject || !cameraObject->hasCameraController())
    return;

  cameraObject->getCameraController()->processMouseScroll(
      *cameraObject->getCamera(), static_cast<float>(yoffset));
}
