#pragma once
#include "../components/MaterialComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/World.hpp"
#include "../gl_common.hpp"
#include "../resources/ResourceManager.hpp"
#include "RenderCommon.hpp"

extern World gWorld;

class SpriteRenderSystem : public System {
private:
  float screenWidth;
  float screenHeight;

public:
  SpriteRenderSystem(float width = 800.0f, float height = 600.0f)
      : screenWidth(width), screenHeight(height) {}

  void render() override {
    // Clear the screen
    glm::vec3 clearColor = RenderUtils::getActiveSceneClearColor(gWorld);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 2D sprite rendering setup
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection =
        glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -1.0f, 1.0f);

    auto &resources = ResourceManager::instance();

    gWorld.forEachWith<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity entity, TransformComponent &transform, MeshComponent &mesh,
            MaterialComponent &material) {
          if (!mesh.isValid())
            return;

          if (material.alpha <= 0.0f)
            return;

          Shader *shader = resources.getShader(material.shaderProgram);
          if (!shader)
            return;

          shader->use();
          shader->setMat4("projection", projection);
          shader->setMat4("model", transform.getSpriteModelMatrix());
          shader->setVec3("spriteColor", material.color);
          shader->setFloat("spriteAlpha", material.alpha);
          shader->setBool("useTexture", material.useTextures);
          shader->setBool("isCircle", material.isCircle);

          if (material.useTextures && material.textures[0] != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material.textures[0]);
            shader->setInt("image", 0);
          }

          RenderUtils::drawMesh(mesh);
        });
  }

  void setScreenSize(float w, float h) {
    screenWidth = w;
    screenHeight = h;
  }
};
