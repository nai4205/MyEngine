#pragma once
#include "../resources/ResourceManager.hpp"

#include "../components/MaterialComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/Tag.hpp"
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

  void render() override {
    SkyboxEntity skybox;
    gWorld.forEachWith<MeshComponent, MaterialComponent, TagComponent>(
        [&](Entity entity, MeshComponent &mesh, MaterialComponent &material,
            TagComponent &tag) {
          if (!tag.has(SKYBOX))
            return;
          skybox.entity = entity;
          skybox.mesh = &mesh;
          skybox.material = &material;
          skybox.tag = &tag;
          return;
        });
    float aspectRatio =
        static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    auto camera = getActiveCamera(gWorld, aspectRatio);
    glDepthFunc(GL_LEQUAL);
    auto &resources = ResourceManager::instance();
    Shader *shader = resources.getShader(skybox.material->shaderProgram);
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
  }
};
