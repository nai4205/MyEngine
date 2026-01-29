#include "EngineManager.hpp"
#include "scenes/MainScene.hpp"
#include "scenes/Scene2D.hpp"
#include "scenes/SceneManager.hpp"
#include "scenes/breakout/Breakout.hpp"

World gWorld;
GLFWwindow *window = nullptr;

EngineManager engineManager;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool wireframe = false;
float lastFrame = 0.0f;
float lastTitleUpdate = 0.0f;
int frameCount = 0;
int main() {
  engineManager.init();

  auto &sceneManager = SceneManager::instance();
  // sceneManager.registerScene<MainScene>("main", SCR_WIDTH, SCR_HEIGHT);
  sceneManager.registerScene<Scene2D>("Scene2D", SCR_WIDTH, SCR_HEIGHT);
  sceneManager.registerScene<Breakout>("Breakout", SCR_WIDTH, SCR_HEIGHT);
  sceneManager.loadScene("Breakout", gWorld);

  engineManager.mainLoop();

  glfwTerminate();
  return 0;
}
