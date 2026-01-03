#include <cstdio>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "camera/Camera.hpp"
#include "const_h.hpp"
#include "glm/detail/func_trigonometric.hpp"
#include "glm/detail/type_vec.hpp"
#include "mesh_h.hpp"
#include "shader_h.hpp"
#include "texture_2d_h.hpp"

#include "GLFW/glfw3.h"

void processInput(GLFWwindow *window);
void update();
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
int cleanup();

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 100.0f;
const float FOV = 45;
const glm::vec4 CLEAR_COL = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
bool wireframe = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
Camera camera;

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

  Shader lightSourceShader("../src/shaders/lightSource/lightSourceVertex.glsl",
                           "../src/shaders/lightSource/lightSourceFragment.glsl");
  Shader lightingShader("../src/shaders/lighting/lightingVertex.glsl",
                        "../src/shaders/lighting/lightingFragment.glsl");
  Mesh mesh(lightVertices, sizeof(lightVertices));
  mesh.setupObjectVAO();

  /************TEXTURES************/
  Texture2D texture1({{GL_TEXTURE_WRAP_S, GL_REPEAT},
                      {GL_TEXTURE_WRAP_T, GL_REPEAT},
                      {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                      {GL_TEXTURE_MAG_FILTER, GL_LINEAR}});
  texture1.loadImage("../src/assets/container.jpg");

  Texture2D texture2({{GL_TEXTURE_WRAP_S, GL_REPEAT},
                      {GL_TEXTURE_WRAP_T, GL_REPEAT},
                      {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
                      {GL_TEXTURE_MAG_FILTER, GL_LINEAR}});

  texture2.loadImage("../src/assets/awesomeface.png");
  /****************************************/

  // lightCubeShader.use();
  // lightCubeShader.setInt("texture1", 0);
  // lightCubeShader.setInt("texture2", 1);
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
    camera.UpdatePhysics(deltaTime);

    // render
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // be sure to activate shader when setting uniforms/drawing objects
    lightingShader.use();
    lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    lightingShader.setVec3("lightPos", lightPos);

    // view/projection transformations
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.Zoom),
                         (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    lightingShader.setMat4("projection", projection);
    lightingShader.setMat4("view", view);
    // world transformation
    glm::mat4 model = glm::mat4(1.0f);
    lightingShader.setMat4("model", model);
    // render the cube
    mesh.bindObject();
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // also draw the lamp object
    lightSourceShader.use();
    lightSourceShader.setMat4("projection", projection);
    lightSourceShader.setMat4("view", view);
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
    lightSourceShader.setMat4("model", model);
    mesh.bindLightSource();

    glDrawArrays(GL_TRIANGLES, 0, 36);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void update() {}

void processInput(GLFWwindow *window) {
  static bool spaceWasPressed = false;
  bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, deltaTime);
  if (spacePressed && !spaceWasPressed) {
    camera.Jump();
  }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
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

  camera.ProcessMouseMovement(xOffset, yOffset);
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
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
