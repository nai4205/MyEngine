#pragma once

#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../../../resources/Framebuffer.hpp"
#include "../../../resources/ResourceManager.hpp"
#include "../components/PostProcessingComponent.hpp"

extern World gWorld;

class PostProcessingSystem : public System {
public:
  PostProcessingSystem(unsigned int width, unsigned int height)
      : width(width), height(height) {
    framebuffer = Framebuffer(width, height);
    initScreenQuad();
    initShader();
  }

  // Call before scene rendering so the rest of the renderers rendfer to this
  // framebuffer
  void beginRender() {
    framebuffer.bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  // Call after scene rendering to apply effects and draw to screen
  void render() override {
    framebuffer.unbind();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // Find post-processing settings
    PostProcessingComponent *fx = nullptr;
    gWorld.forEachWith<PostProcessingComponent>(
        [&](Entity entity, PostProcessingComponent &comp) { fx = &comp; });

    auto &resources = ResourceManager::instance();
    Shader *shader = resources.getShader(shaderID);
    if (!shader)
      return;

    shader->use();
    shader->setFloat("time", time);
    shader->setBool("confuse", fx ? fx->confuse : false);
    shader->setBool("chaos", fx ? fx->chaos : false);
    shader->setBool("shake", fx ? fx->shake : false);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer.colorTexture);
    shader->setInt("scene", 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
  }

  void update(float &deltaTime) override {
    time += deltaTime;

    // Update shake timer
    gWorld.forEachWith<PostProcessingComponent>(
        [&](Entity entity, PostProcessingComponent &fx) {
          if (fx.shakeTime > 0.0f) {
            fx.shakeTime -= deltaTime;
            if (fx.shakeTime <= 0.0f) {
              fx.shake = false;
            }
          }
        });

    beginRender();
  }

  void setScreenSize(unsigned int w, unsigned int h) {
    if (width == w && height == h)
      return;
    width = w;
    height = h;
    framebuffer.resize(w, h);
  }

private:
  Framebuffer framebuffer;
  unsigned int width, height;
  unsigned int quadVAO = 0;
  uint32_t shaderID = 0;
  float time = 0.0f;

  void initScreenQuad() {
    float vertices[] = {// pos        // tex
                        -1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 1.0f,
                        1.0f,  1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,

                        -1.0f, -1.0f, 0.0f,  0.0f, 1.0f, -1.0f,
                        1.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f};

    unsigned int VBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  void initShader() {
    auto &resources = ResourceManager::instance();
    shaderID = resources.loadShader(
        "postProcessShader",
        "../src/scenes/breakout/shaders/postProcessVertex.glsl",
        "../src/scenes/breakout/shaders/postProcessFragment.glsl");

    Shader *shader = resources.getShader(shaderID);
    if (shader) {
      shader->use();

      // Set up kernel offsets
      float offset = 1.0f / 300.0f;
      float offsets[9][2] = {
          {-offset, offset},  {0.0f, offset},  {offset, offset},
          {-offset, 0.0f},    {0.0f, 0.0f},    {offset, 0.0f},
          {-offset, -offset}, {0.0f, -offset}, {offset, -offset}};
      glUniform2fv(glGetUniformLocation(shader->ID, "offsets"), 9,
                   (float *)offsets);

      int edgeKernel[9] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
      glUniform1iv(glGetUniformLocation(shader->ID, "edge_kernel"), 9,
                   edgeKernel);

      float blurKernel[9] = {1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
                             2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
                             1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f};
      glUniform1fv(glGetUniformLocation(shader->ID, "blur_kernel"), 9,
                   blurKernel);
    }
  }
};
