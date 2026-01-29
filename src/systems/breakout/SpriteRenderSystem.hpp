#pragma once
#include "../../components/CameraComponent.hpp"
#include "../../components/TransformComponent.hpp"
#include "../../ecs/System.hpp"
#include "../../ecs/Tag.hpp"
#include "../../ecs/World.hpp"
#include "../../gl_common.hpp"
#include "../../resources/ResourceManager.hpp"
#include "../RenderCommon.hpp"
#include <vector>

extern World gWorld;

class SpriteRenderSystem : public System {
private:
  float screenWidth;
  float screenHeight;

public:
  SpriteRenderSystem(float width = 800.0f, float height = 600.0f)
      : screenWidth(width), screenHeight(height) {}

  void render() override {
    // Render to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear screen
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disable depth test for 2D sprites
    glDisable(GL_DEPTH_TEST);

    // Set up projection matrix like the tutorial:
    // ortho(left, right, bottom, top, near, far)
    // bottom=height, top=0 flips Y so (0,0) is top-left
    glm::mat4 projection =
        glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -1.0f, 1.0f);

    std::vector<RenderableEntity> sprites;
    gWorld.forEachWith<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity entity, TransformComponent &transform, MeshComponent &mesh,
            MaterialComponent &material) {
          if (!mesh.isValid())
            return;

          RenderableEntity renderable;
          renderable.entity = entity;
          renderable.transform = &transform;
          renderable.mesh = &mesh;
          renderable.material = &material;
          renderable.tag = gWorld.getComponent<TagComponent>(entity);

          sprites.emplace_back(renderable);
        });

    auto &resources = ResourceManager::instance();

    for (const auto &sprite : sprites) {
      Shader *shader = resources.getShader(sprite.material->shaderProgram);
      if (!shader)
        continue;

      shader->use();
      shader->setMat4("projection", projection);
      shader->setMat4("model", sprite.transform->getModelMatrix());
      shader->setVec3("spriteColor", sprite.material->diffuse);

      if (sprite.material->useTextures && sprite.material->textures[0] != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sprite.material->textures[0]);
        shader->setInt("image", 0);
      }

      RenderUtils::drawMesh(*sprite.mesh);
    }
  }
};
