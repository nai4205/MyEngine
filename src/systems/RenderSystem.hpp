#pragma once

#include "../resources/ResourceManager.hpp"

#include "../components/MaterialComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/World.hpp"
#include "../ecs/utils/CameraUtils.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>

extern World gWorld;

class RenderSystem : public System {
private:
  unsigned int screenWidth = 800;
  unsigned int screenHeight = 600;

public:
  RenderSystem(unsigned int width = 800, unsigned int height = 600)
      : screenWidth(width), screenHeight(height) {}

  void render() override {
    float aspectRatio =
        static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    auto camera = getActiveCamera(gWorld, aspectRatio);

    auto &resources = ResourceManager::instance();

    renderLitEntities(camera, resources);

    renderUnlitEntities(camera, resources);
  }

private:
  void renderLitEntities(const ActiveCameraData &camera,
                         ResourceManager &resources) {
    std::unordered_set<uint32_t> configuredShaders;

    gWorld.forEachWith<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity entity, TransformComponent &transform, MeshComponent &mesh,
            MaterialComponent &material) {
          if (!mesh.isValid() || !material.receivesLighting)
            return;

          Shader *shader = resources.getShader(material.shaderProgram);
          if (!shader)
            return;

          shader->use();

          if (configuredShaders.find(material.shaderProgram) ==
              configuredShaders.end()) {
            shader->setMat4("view", camera.view);
            shader->setMat4("projection", camera.projection);
            shader->setVec3("viewPos", camera.position);
            configuredShaders.insert(material.shaderProgram);
          }

          shader->setMat4("model", transform.getModelMatrix());
          shader->setVec3("material.vAmbient", material.ambient);
          shader->setVec3("material.vDiffuse", material.diffuse);
          shader->setVec3("material.vSpecular", material.specular);
          shader->setFloat("material.shininess", material.shininess);
          shader->setBool("material.useTex", material.useTextures);

          if (material.useTextures) {
            for (size_t i = 0; i < MAX_MATERIAL_TEXTURES; i++) {
              if (material.textures[i] != 0) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, material.textures[i]);
              }
            }
            shader->setInt("material.texture_diffuse1", 0);
            shader->setInt("material.texture_specular1", 1);
          }

          drawMesh(mesh);
        });
  }

  void renderUnlitEntities(const ActiveCameraData &camera,
                           ResourceManager &resources) {
    std::unordered_set<uint32_t> configuredShaders;

    gWorld.forEachWith<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity entity, TransformComponent &transform, MeshComponent &mesh,
            MaterialComponent &material) {
          if (!mesh.isValid() || material.receivesLighting)
            return;

          Shader *shader = resources.getShader(material.shaderProgram);
          if (!shader)
            return;

          shader->use();

          if (configuredShaders.find(material.shaderProgram) ==
              configuredShaders.end()) {
            shader->setMat4("view", camera.view);
            shader->setMat4("projection", camera.projection);
            configuredShaders.insert(material.shaderProgram);
          }

          shader->setMat4("model", transform.getModelMatrix());
          shader->setVec3("objectColor", material.diffuse);

          drawMesh(mesh);
        });
  }

  void drawMesh(const MeshComponent &mesh) {
    glBindVertexArray(mesh.vao);
    if (mesh.isIndexed()) {
      glDrawElements(GL_TRIANGLES, mesh.indexCount, mesh.indexType, nullptr);
    } else {
      glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
    }
    glBindVertexArray(0);
  }

  void setScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
  }
};
