#pragma once

#include "../gl_common.hpp"
#include "../resources/ResourceManager.hpp"
#include "RenderCommon.hpp"

#include "../ecs/System.hpp"
#include "../ecs/World.hpp"

extern World gWorld;

class CompositeRenderSystem : public System {
private:
  unsigned int screenWidth = 800;
  unsigned int screenHeight = 600;
  unsigned int screenQuadVAO = 0;
  int postProcessEffect = 0;

public:
  CompositeRenderSystem(unsigned int width = 800, unsigned int height = 600)
      : screenWidth(width), screenHeight(height) {
    setupScreenQuad();
  }

  ~CompositeRenderSystem() {
    if (screenQuadVAO) {
      glDeleteVertexArrays(1, &screenQuadVAO);
    }
  }

  void setPostProcessEffect(int effect) { postProcessEffect = effect; }

  void setScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
  }

  void render() override {
    std::string activeSceneName = RenderUtils::getActiveSceneName(gWorld);
    if (activeSceneName.empty()) {
      return;
    }

    glm::vec3 clearColor = RenderUtils::getActiveSceneClearColor(gWorld);
    if (glm::length(clearColor) <= 0.0f) {
      clearColor = glm::vec3(0.2f, 0.2f, 0.2f);
    }

    auto &resources = ResourceManager::instance();
    Framebuffer *fb = resources.getFramebuffer(activeSceneName);

    if (fb) {
      fb->unbind();
      glDisable(GL_DEPTH_TEST);

      glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      Shader *screenShader = resources.getShader("postprocess");
      if (screenShader) {
        screenShader->use();
        screenShader->setInt("screenTexture", 0);
        screenShader->setInt("effect", postProcessEffect);

        glBindVertexArray(screenQuadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fb->colorTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
      }

      glEnable(GL_DEPTH_TEST);
    }
  }

private:
  void setupScreenQuad() {
    float quadVertices[] = {-1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                            1.0f,  -1.0f, 1.0f, 0.0f, -1.0f, 1.0f,  0.0f, 1.0f,
                            1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f};

    unsigned int quadVBO;
    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
  }
};
