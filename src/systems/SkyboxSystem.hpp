#pragma once
#include "../resources/ResourceManager.hpp"
#include "RenderCommon.hpp"

#include "../ecs/System.hpp"
#include "../ecs/World.hpp"
#include "../ecs/utils/CameraUtils.hpp"

extern World gWorld;

class SkyboxSystem : public System {
private:
  struct SkyboxEntity {
    Entity entity;
    TransformComponent *transform;
    MeshComponent *mesh;
    MaterialComponent *material;
    TagComponent *tag;
  };

  unsigned int screenWidth = 800;
  unsigned int screenHeight = 600;
  bool shaderInitialized = false;

public:
  SkyboxSystem(unsigned int width = 800, unsigned int height = 600)
      : screenWidth(width), screenHeight(height) {}

  void setScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
  }

  void render() override {
    // Get active scene name
    std::string activeSceneName = RenderUtils::getActiveSceneName(gWorld);
    if (activeSceneName.empty()) {
      return;
    }

    // Get framebuffer from OpaqueRenderSystem
    auto *opaqueSystem = gWorld.getSystem<class OpaqueRenderSystem>();
    if (!opaqueSystem) {
      return;
    }

    auto &resources = ResourceManager::instance();
    Framebuffer *fb = resources.getFramebuffer(activeSceneName);

    // Ensure framebuffer is bound
    if (fb) {
      fb->bind();
    }

    // Find skybox entity
    SkyboxEntity skybox;
    bool foundSkybox = false;
    gWorld.forEachWith<MeshComponent, MaterialComponent, TagComponent>(
        [&](Entity entity, MeshComponent &mesh, MaterialComponent &material,
            TagComponent &tag) {
          if (!tag.has(SKYBOX))
            return;
          skybox.entity = entity;
          skybox.mesh = &mesh;
          skybox.material = &material;
          skybox.tag = &tag;
          foundSkybox = true;
          return;
        });

    if (!foundSkybox)
      return;

    // Render skybox to framebuffer
    float aspectRatio =
        static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    auto camera = getActiveCamera(gWorld, aspectRatio);

    glDepthFunc(GL_LEQUAL);

    Shader *shader = resources.getShader(skybox.material->shaderProgram);
    if (!shader)
      return;

    shader->use();
    if (!shaderInitialized) {
      shader->setInt("skybox", 0);
      shaderInitialized = true;
    }

    glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.view));
    shader->setMat4("view", skyboxView);
    shader->setMat4("projection", camera.projection);

    glBindVertexArray(skybox.mesh->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.material->textures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);

    // Keep framebuffer bound for next system (TransparentRenderSystem)
  }
};
