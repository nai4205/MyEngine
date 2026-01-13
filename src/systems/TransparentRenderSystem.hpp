#pragma once

#include "../gl_common.hpp"
#include "../resources/ResourceManager.hpp"
#include "RenderCommon.hpp"

#include "../ecs/System.hpp"
#include "../ecs/World.hpp"
#include "../ecs/utils/CameraUtils.hpp"

#include <algorithm>
#include <unordered_set>

extern World gWorld;

class TransparentRenderSystem : public System {
private:
  unsigned int screenWidth = 800;
  unsigned int screenHeight = 600;

public:
  TransparentRenderSystem(unsigned int width = 800, unsigned int height = 600)
      : screenWidth(width), screenHeight(height) {}

  void setScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
  }

  void render() override {
    float aspectRatio =
        static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    auto camera = getActiveCamera(gWorld, aspectRatio);

    std::string activeSceneName = RenderUtils::getActiveSceneName(gWorld);
    if (activeSceneName.empty()) {
      return;
    }

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

    std::vector<RenderableEntity> transparentEntities;

    gWorld.forEachWith<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity entity, TransformComponent &transform, MeshComponent &mesh,
            MaterialComponent &material) {
          if (!mesh.isValid() || !material.hasTransparency)
            return;

          RenderableEntity renderable;
          renderable.entity = entity;
          renderable.transform = &transform;
          renderable.mesh = &mesh;
          renderable.material = &material;
          renderable.tag = gWorld.getComponent<TagComponent>(entity);

          transparentEntities.emplace_back(renderable);
        });

    // Sort transparent entities back-to-front (far to near)
    sortTransparentEntities(transparentEntities, camera.position);

    glDepthMask(GL_FALSE);
    renderTransparentEntities(camera, resources, transparentEntities);
    glDepthMask(GL_TRUE);

    // Keep framebuffer bound for next system
  }

private:
  void sortTransparentEntities(std::vector<RenderableEntity> &entities,
                               const glm::vec3 &cameraPos) {
    std::sort(
        entities.begin(), entities.end(),
        [&cameraPos](const RenderableEntity &a, const RenderableEntity &b) {
          float distA = glm::length(a.transform->position - cameraPos);
          float distB = glm::length(b.transform->position - cameraPos);
          return distA > distB; // Sort far to near (back-to-front)
        });
  }

  void
  renderTransparentEntities(const ActiveCameraData &camera,
                            ResourceManager &resources,
                            const std::vector<RenderableEntity> &renderables) {
    std::unordered_set<uint32_t> configuredShaders;

    for (const auto &renderable : renderables) {
      Shader *shader = resources.getShader(renderable.material->shaderProgram);
      if (!shader)
        continue;

      shader->use();

      // Configure shader once per unique shader program
      if (configuredShaders.find(renderable.material->shaderProgram) ==
          configuredShaders.end()) {
        shader->setMat4("view", camera.view);
        shader->setMat4("projection", camera.projection);
        if (renderable.material->receivesLighting) {
          shader->setVec3("viewPos", camera.position);
        }
        configuredShaders.insert(renderable.material->shaderProgram);
      }

      shader->setMat4("model", renderable.transform->getModelMatrix());

      if (renderable.material->receivesLighting) {
        shader->setVec3("material.vAmbient", renderable.material->ambient);
        shader->setVec3("material.vDiffuse", renderable.material->diffuse);
        shader->setVec3("material.vSpecular", renderable.material->specular);
        shader->setFloat("material.shininess", renderable.material->shininess);
        shader->setBool("material.useTex", renderable.material->useTextures);

        if (renderable.material->useTextures) {
          for (size_t i = 0; i < MAX_MATERIAL_TEXTURES; i++) {
            if (renderable.material->textures[i] != 0) {
              glActiveTexture(GL_TEXTURE0 + i);
              glBindTexture(GL_TEXTURE_2D, renderable.material->textures[i]);
            }
          }
          shader->setInt("material.texture_diffuse1", 0);
          shader->setInt("material.texture_specular1", 1);
        }
      } else {
        shader->setVec3("objectColor", renderable.material->diffuse);
      }

      RenderUtils::drawMesh(*renderable.mesh);
    }
  }
};
