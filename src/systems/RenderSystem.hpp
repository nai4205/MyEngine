#pragma once

#include "../gl_common.hpp"
#include "../resources/ResourceManager.hpp"

#include "../components/MaterialComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/Tag.hpp"
#include "../ecs/World.hpp"
#include "../ecs/utils/CameraUtils.hpp"
#include <map>

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

    bool hasOutlined = false;
    gWorld.forEachWith<TagComponent>([&](Entity entity, TagComponent &tag) {
      if (tag.has(OUTLINED)) {
        hasOutlined = true;
        return;
      }
    });
    if (hasOutlined) {
      // Render non-outlined entities with stencil writing disabled
      glStencilMask(0x00);
      renderLitEntities(camera, resources, false, false);
      renderUnlitEntities(camera, resources, false);

      // Stencil outlining
      // Mask - 0xFF -> each bit is written as is
      //      - 0x00 -> each bit ends up as 0
      // Step 1: Render outlined entities with stencil writing enabled
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilMask(0xFF);
      renderLitEntities(camera, resources, true, false);

      // Step 2: Render outlines (scaled up, single color, where stencil != 1)
      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00);
      // glDisable(GL_DEPTH_TEST);
      renderOutlines(camera, resources);

      glStencilMask(0xFF);
      glStencilFunc(GL_ALWAYS, 0, 0xFF);
      // glEnable(GL_DEPTH_TEST);
    } else {

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      renderLitEntities(camera, resources, false, false);
      renderUnlitEntities(camera, resources, false);
      glDisable(GL_CULL_FACE);
    }

    // Disable depth writing so transparent objects don't block objects behind
    // them
    glDepthMask(GL_FALSE);
    renderTransparentEntities(camera, resources);
    glDepthMask(GL_TRUE);
  }

private:
  void renderLitEntities(const ActiveCameraData &camera,
                         ResourceManager &resources, bool onlyOutlined,
                         bool renderTransparent) {
    std::unordered_set<uint32_t> configuredShaders;

    gWorld.forEachWith<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity entity, TransformComponent &transform, MeshComponent &mesh,
            MaterialComponent &material) {
          if (!mesh.isValid() || !material.receivesLighting)
            return;

          // Skip transparent objects when rendering opaque and vice versa
          if (material.hasTransparency != renderTransparent)
            return;

          bool hasOutlineTag =
              gWorld.hasComponent<TagComponent>(entity) &&
              gWorld.getComponent<TagComponent>(entity)->has(OUTLINED);
          if (hasOutlineTag != onlyOutlined)
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
                           ResourceManager &resources, bool renderTransparent) {
    std::unordered_set<uint32_t> configuredShaders;

    gWorld.forEachWith<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity entity, TransformComponent &transform, MeshComponent &mesh,
            MaterialComponent &material) {
          if (!mesh.isValid() || material.receivesLighting)
            return;

          // Skip transparent objects when rendering opaque and vice versa
          if (material.hasTransparency != renderTransparent)
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

  void renderOutlines(const ActiveCameraData &camera,
                      ResourceManager &resources) {
    Shader *outlineShader = resources.getShader("singleColor");
    if (!outlineShader)
      return;

    outlineShader->use();
    outlineShader->setMat4("view", camera.view);
    outlineShader->setMat4("projection", camera.projection);
    outlineShader->setFloat("outlineWidth", 0.05);

    const float outlineScale = 1.05;

    gWorld.forEachWith<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity entity, TransformComponent &transform, MeshComponent &mesh,
            MaterialComponent &material) {
          if (!gWorld.hasComponent<TagComponent>(entity))
            return;

          TagComponent *tag = gWorld.getComponent<TagComponent>(entity);
          if (!tag->has(OUTLINED))
            return;

          if (!mesh.isValid())
            return;

          TransformComponent scaledTransform = transform;
          scaledTransform.scale *= outlineScale;

          outlineShader->setMat4("model", scaledTransform.getModelMatrix());
          outlineShader->setBool("useTex", material.useTextures);

          if (material.useTextures && material.textures[0] != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material.textures[0]);
            outlineShader->setInt("texture_diffuse1", 0);
          }

          drawMesh(mesh);
        });
  }

  void renderTransparentEntities(const ActiveCameraData &camera,
                                 ResourceManager &resources) {
    std::unordered_set<uint32_t> configuredShaders;

    gWorld.forEachWith<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity entity, TransformComponent &transform, MeshComponent &mesh,
            MaterialComponent &material) {
          if (!mesh.isValid() || !material.hasTransparency)
            return;

          if (material.receivesLighting) {
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
          } else {
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
          }
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
