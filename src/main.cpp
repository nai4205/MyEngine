#include "components/DirectionalLightComponent.hpp"
#include "components/PointLightComponent.hpp"
#include "components/SpotLightComponent.hpp"
#include "gl_common.hpp"
#include <iostream>

#include "ecs/World.hpp"
#include "scenes/MainScene.hpp"
#include "scenes/SceneManager.hpp"
#include "systems/CameraControllerSystem.hpp"
#include "systems/CameraSystem.hpp"
#include "systems/LightingSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/RenderSystem.hpp"

World gWorld;
RenderSystem *gRenderSystem = nullptr;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool wireframe = false;
float lastFrame = 0.0f;

int main() {
  // GLFW/GLAD setup (unchanged)
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Learning OpenGL", NULL, NULL);
  if (!window) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGL()) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  stbi_set_flip_vertically_on_load(true);

  // Register components
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
  gRenderSystem = gWorld.addSystem<RenderSystem>(SCR_WIDTH, SCR_HEIGHT);

  auto &sceneManager = SceneManager::instance();
  sceneManager.registerScene<MainScene>("main", SCR_WIDTH, SCR_HEIGHT);
  sceneManager.loadScene("main", gWorld);

  auto *mainScene = sceneManager.getCurrentScene();
  glm::vec4 clearColor = mainScene ? mainScene->getClearColor()
                                   : glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(window)) {
    float currentFrame = static_cast<float>(glfwGetTime());
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    gWorld.getInput().newFrame();
    glfwPollEvents();

    gWorld.update(deltaTime);

    // Clear is now handled inside RenderSystem for the framebuffer
    gWorld.render();

    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (action == GLFW_PRESS || action == GLFW_RELEASE) {
    gWorld.getInput().setKeyPressed(key, action == GLFW_PRESS);
  }

  if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
    wireframe = !wireframe;
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
  }

  // Post-processing effect controls (number keys 0-5)
  if (action == GLFW_PRESS && gRenderSystem) {
    if (key == GLFW_KEY_0)
      gRenderSystem->setPostProcessEffect(0); // Normal
    else if (key == GLFW_KEY_1)
      gRenderSystem->setPostProcessEffect(1); // Invert
    else if (key == GLFW_KEY_2)
      gRenderSystem->setPostProcessEffect(2); // Grayscale
    else if (key == GLFW_KEY_3)
      gRenderSystem->setPostProcessEffect(3); // Sharpen
    else if (key == GLFW_KEY_4)
      gRenderSystem->setPostProcessEffect(4); // Blur
    else if (key == GLFW_KEY_5)
      gRenderSystem->setPostProcessEffect(5); // Edge detection
  }

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);

  // Resize the framebuffer to match new window size
  auto &resources = ResourceManager::instance();
  resources.resizeFramebuffer("main", width, height);

  // Update render system screen size
  if (gRenderSystem) {
    gRenderSystem->setScreenSize(width, height);
  }
}
