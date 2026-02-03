#pragma once

#include "../gl_common.hpp"
#include "../resources/ResourceManager.hpp"
#include "RenderCommon.hpp"

#include "../ecs/System.hpp"
#include "../ecs/World.hpp"
#include "../ecs/utils/CameraUtils.hpp"

#include <unordered_set>

extern World gWorld;

// TODO: even after splitting up the rendering systems they are still a bit too
// heavy, this one could be split into outlines
// Also, the framebuffer management is a bit messy
class OpaqueRenderSystem : public System {
private:
  unsigned int screenWidth = 800;
  unsigned int screenHeight = 600;

public:
  OpaqueRenderSystem(unsigned int width = 800, unsigned int height = 600)
      : screenWidth(width), screenHeight(height) {}

  void setScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
  }

  Framebuffer *getFramebuffer(const std::string &sceneName) {
    auto &resources = ResourceManager::instance();
    return resources.getFramebuffer(sceneName);
  }

  void render() override {
    float aspectRatio =
        static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    auto camera = getActiveCamera(gWorld, aspectRatio);

    std::string activeSceneName = RenderUtils::getActiveSceneName(gWorld);
    if (activeSceneName.empty()) {
      std::cout << "Must have an active scene" << std::endl;
      return;
    }

    glm::vec3 clearColor = RenderUtils::getActiveSceneClearColor(gWorld);
    if (glm::length(clearColor) <= 0.0f) {
      std::cout << "No clear color set by the scene, setting a default one"
                << std::endl;
      clearColor = glm::vec3(0.2f, 0.2f, 0.2f);
    }

    auto &resources = ResourceManager::instance();
    Framebuffer *fb = resources.getFramebuffer(activeSceneName);

    // Render opaque scene to framebuffer
    if (fb) {
      fb->bind();
    }

    glEnable(GL_DEPTH_TEST);

    glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    std::vector<RenderableEntity> renderables;
    bool hasOutlined = false;

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

          if (renderable.tag && renderable.tag->has(OUTLINED)) {
            hasOutlined = true;
          }

          renderables.emplace_back(renderable);
        });

    std::vector<RenderableEntity> singleSided;
    singleSided.reserve(renderables.size() / 2);
    std::vector<RenderableEntity> doubleSided;
    doubleSided.reserve(renderables.size() / 2);

    for (const auto &renderable : renderables) {
      if (renderable.material->doubleSided) {
        doubleSided.emplace_back(renderable);
      } else {
        singleSided.emplace_back(renderable);
      }
    }

    glEnable(GL_CULL_FACE);
    renderEntitiesWithCulling(camera, resources, singleSided, hasOutlined);
    glDisable(GL_CULL_FACE);
    renderEntitiesWithCulling(camera, resources, doubleSided, hasOutlined);

    // Keep framebuffer bound for next system (SkyboxSystem, then
    // TransparentRenderSystem)
  }

private:
  void renderEntitiesWithCulling(
      const ActiveCameraData camera, ResourceManager &resources,
      const std::vector<RenderableEntity> &renderables, bool hasOutlined) {
    if (hasOutlined) {
      glStencilMask(0x00);
      renderEntities(camera, resources, renderables, false, false);

      // Stencil outlining
      // Mask - 0xFF -> each bit is written as is
      //      - 0x00 -> each bit ends up as 0
      // Step 1: Render outlined entities with stencil writing enabled
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilMask(0xFF);
      renderEntities(camera, resources, renderables, true, false);

      // Step 2: Render outlines (scaled up, single color, where stencil != 1)
      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00);
      // glDisable(GL_DEPTH_TEST);
      renderOutlines(camera, resources, renderables);

      glStencilMask(0xFF);
      glStencilFunc(GL_ALWAYS, 0, 0xFF);
      // glEnable(GL_DEPTH_TEST);
    } else {
      renderEntities(camera, resources, renderables, false, false);
    }

    // Transparent rendering is now handled by TransparentRenderSystem
  }

  void renderEntities(const ActiveCameraData &camera,
                      ResourceManager &resources,
                      const std::vector<RenderableEntity> &renderables,
                      bool onlyOutlined, bool renderTransparent) {
    std::unordered_set<uint32_t> configuredShaders;

    for (const auto &renderable : renderables) {
      if (renderable.material->hasTransparency != renderTransparent)
        continue;

      bool hasOutlineTag = renderable.tag && renderable.tag->has(OUTLINED);
      if (hasOutlineTag != onlyOutlined)
        continue;

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
        // Unlit entity (light source shader)
        shader->setVec3("objectColor", renderable.material->diffuse);
      }

      RenderUtils::drawMesh(*renderable.mesh);
    }
  }

  void renderOutlines(const ActiveCameraData &camera,
                      ResourceManager &resources,
                      const std::vector<RenderableEntity> &renderables) {
    Shader *outlineShader = resources.getShader("singleColor");
    if (!outlineShader)
      return;

    outlineShader->use();
    outlineShader->setMat4("view", camera.view);
    outlineShader->setMat4("projection", camera.projection);
    outlineShader->setFloat("outlineWidth", 0.05);

    const float outlineScale = 1.05;

    for (const auto &renderable : renderables) {
      if (!renderable.tag || !renderable.tag->has(OUTLINED))
        continue;

      TransformComponent scaledTransform = *renderable.transform;
      scaledTransform.scale *= outlineScale;

      outlineShader->setMat4("model", scaledTransform.getModelMatrix());
      outlineShader->setBool("useTex", renderable.material->useTextures);

      if (renderable.material->useTextures &&
          renderable.material->textures[0] != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderable.material->textures[0]);
        outlineShader->setInt("texture_diffuse1", 0);
      }

      RenderUtils::drawMesh(*renderable.mesh);
    }
  }
};
