
#include "components/CameraFollowComponent.hpp"
#include "components/DirectionalLightComponent.hpp"
#include "components/NameComponent.hpp"
#include "components/PointLightComponent.hpp"
#include "components/SceneComponent.hpp"
#include "components/SpotLightComponent.hpp"
#include "ecs/Tag.hpp"
#include "ecs/World.hpp"
#include "gl_common.hpp"
#include "systems/CameraControllerSystem.hpp"
#include <iostream>
#include <string>

#include "ecs/World.hpp"
#include "systems/CameraControllerSystem.hpp"
#include "systems/CameraFollowSystem.hpp"
#include "systems/CameraSystem.hpp"
#include "systems/CompositeRenderSystem.hpp"
#include "systems/LightingSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/PlayerControllerSystem.hpp"
#include "systems/RenderSystem.hpp" // Now OpaqueRenderSystem
#include "systems/SkyboxSystem.hpp"
#include "systems/TransparentRenderSystem.hpp"

// Extern declarations for globals defined in main.cpp
extern World gWorld;
extern GLFWwindow *window;

class EngineManager {
public:
  // EngineManager(std::function<void(int, int)> frameBufferCallback) {}
  int init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Application", NULL,
                              NULL);
    if (!window) {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGL()) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return -1;
    }

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return 1;
  }

  void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
      float currentFrame = static_cast<float>(glfwGetTime());
      float deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;
      frameCount++;
      if (currentFrame - lastTitleUpdate >= 0.1f) {
        float fps = frameCount / (currentFrame - lastTitleUpdate);
        std::string title = "OpenGL Application - FPS: " +
                            std::to_string(static_cast<int>(fps));
        glfwSetWindowTitle(window, title.c_str());
        frameCount = 0;
        lastTitleUpdate = currentFrame;
      }
      gWorld.getInput().newFrame();
      glfwPollEvents();
      gWorld.update(deltaTime);
      gWorld.render();
      glfwSwapBuffers(window);
    }
  }

  void initComponents() {
    gWorld.registerComponent<TransformComponent>();
    gWorld.registerComponent<MeshComponent>();
    gWorld.registerComponent<MaterialComponent>();
    gWorld.registerComponent<PhysicsComponent>();
    gWorld.registerComponent<CameraComponent>();
    gWorld.registerComponent<CameraControllerComponent>();
    gWorld.registerComponent<PlayerControllerComponent2D>();
    gWorld.registerComponent<TagComponent>();
    gWorld.registerComponent<NameComponent>();
    gWorld.registerComponent<DirectionalLightComponent>();
    gWorld.registerComponent<PointLightComponent>();
    gWorld.registerComponent<SpotLightComponent>();
    gWorld.registerComponent<SceneComponent>();
    gWorld.registerComponent<CameraFollowComponent>();
  }

  void initSystems() {
    gWorld.addSystem<CameraControllerSystem>();
    gWorld.addSystem<PlayerControllerSystem>();
    gWorld.addSystem<PhysicsSystem>();
    gWorld.addSystem<CameraFollowSystem>();
    gWorld.addSystem<CameraSystem>();
    gWorld.addSystem<LightingSystem>();
    gWorld.addSystem<OpaqueRenderSystem>(SCR_WIDTH, SCR_HEIGHT);
    gWorld.addSystem<SkyboxSystem>(SCR_WIDTH, SCR_HEIGHT);
    gWorld.addSystem<TransparentRenderSystem>(SCR_WIDTH, SCR_HEIGHT);
    gWorld.addSystem<CompositeRenderSystem>(SCR_WIDTH, SCR_HEIGHT);
  }

private:
  const unsigned int SCR_WIDTH = 800;
  const unsigned int SCR_HEIGHT = 600;
  bool wireframe = false;
  float lastFrame = 0.0f;
  float lastTitleUpdate = 0.0f;
  int frameCount = 0;

  static void framebuffer_size_callback(GLFWwindow *window, int width,
                                        int height) {
    auto *engine =
        static_cast<EngineManager *>(glfwGetWindowUserPointer(window));
    if (engine) {
      engine->onFramebufferResize(width, height);
    }
  }

  void onFramebufferResize(int width, int height) {
    glViewport(0, 0, width, height);

    auto &resources = ResourceManager::instance();
    std::string activeSceneName;
    gWorld.forEachWith<SceneComponent, TagComponent>(
        [&](Entity &entity, SceneComponent &scene, TagComponent &tag) {
          if (tag.has(ACTIVESCENE)) {
            activeSceneName = scene.name;
          }
        });
    if (activeSceneName.length() <= 0)
      std::cout << "No active scenes" << std::endl;
    resources.resizeFramebuffer(activeSceneName, width, height);

    if (auto *opaqueSystem = gWorld.getSystem<OpaqueRenderSystem>()) {
      opaqueSystem->setScreenSize(width, height);
    }
    if (auto *skyboxSystem = gWorld.getSystem<SkyboxSystem>()) {
      skyboxSystem->setScreenSize(width, height);
    }
    if (auto *transparentSystem = gWorld.getSystem<TransparentRenderSystem>()) {
      transparentSystem->setScreenSize(width, height);
    }
    if (auto *compositeSystem = gWorld.getSystem<CompositeRenderSystem>()) {
      compositeSystem->setScreenSize(width, height);
    }
  }

  static void key_callback(GLFWwindow *window, int key, int scancode,
                           int action, int mods) {
    auto *engine =
        static_cast<EngineManager *>(glfwGetWindowUserPointer(window));
    if (engine) {
      engine->onKeyPress(window, key, scancode, action, mods);
    }
  }

  void onKeyPress(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
      gWorld.getInput().setKeyPressed(key, action == GLFW_PRESS);
    }

    // TODO: this doesnt work with the framebuffer because there is just a quad
    // blocking the view
    if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
      wireframe = !wireframe;
      glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }

    // Post-processing effect controls (number keys 0-5)
    if (action == GLFW_PRESS) {
      if (auto *compositeSystem = gWorld.getSystem<CompositeRenderSystem>()) {
        if (key == GLFW_KEY_0)
          compositeSystem->setPostProcessEffect(0); // Normal
        else if (key == GLFW_KEY_1)
          compositeSystem->setPostProcessEffect(1); // Invert
        else if (key == GLFW_KEY_2)
          compositeSystem->setPostProcessEffect(2); // Grayscale
        else if (key == GLFW_KEY_3)
          compositeSystem->setPostProcessEffect(3); // Sharpen
        else if (key == GLFW_KEY_4)
          compositeSystem->setPostProcessEffect(4); // Blur
        else if (key == GLFW_KEY_5)
          compositeSystem->setPostProcessEffect(5); // Edge detection
      }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, true);
    }
  }
};
