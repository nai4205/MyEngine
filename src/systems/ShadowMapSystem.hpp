#pragma once

#include "../components/DirectionalLightComponent.hpp"
#include "../components/MaterialComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/PointLightComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/World.hpp"
#include "../resources/PointShadowMap.hpp"
#include "../resources/ResourceManager.hpp"
#include "../resources/ShadowMap.hpp"
#include "LightingSystem.hpp"
#include "RenderCommon.hpp"
#include <array>
#include <vector>

extern World gWorld;

class ShadowMapSystem : public System {
public:
  static constexpr unsigned int SHADOW_WIDTH = 2048;
  static constexpr unsigned int SHADOW_HEIGHT = 2048;
  static constexpr unsigned int POINT_SHADOW_SIZE = 1024;
  static constexpr int MAX_POINT_LIGHTS = 10;

private:
  // Directional light shadow map
  ShadowMap shadowMap;
  uint32_t depthShaderID = 0;
  glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

  // Point light shadow maps
  std::vector<PointShadowMap> pointShadowMaps;
  uint32_t pointDepthShaderID = 0;
  float pointLightFarPlane = 25.0f;

  // Orthographic projection bounds for directional light
  float orthoSize = 20.0f;
  float nearPlane = 1.0f;
  float farPlane = 50.0f;
  float lightDistance = 20.0f;

  unsigned int screenWidth = 800;
  unsigned int screenHeight = 600;

public:
  ShadowMapSystem(unsigned int width = 800, unsigned int height = 600)
      : screenWidth(width), screenHeight(height) {}

  void init() {
    // Create directional shadow map
    shadowMap = ShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT);

    auto &resources = ResourceManager::instance();

    // Load directional light depth shader
    depthShaderID = resources.loadShader(
        "shadowDepth", "../src/shaders/shadow/shadowDepthVertex.glsl",
        "../src/shaders/shadow/shadowDepthFragment.glsl");

    // Load point light depth shader (with geometry shader)
    pointDepthShaderID = resources.loadShader(
        "pointShadowDepth", "../src/shaders/shadow/pointShadowDepthVertex.glsl",
        "../src/shaders/shadow/pointShadowDepthFragment.glsl",
        "../src/shaders/shadow/pointShadowDepthGeometry.glsl");

    // Create point light shadow maps
    pointShadowMaps.reserve(MAX_POINT_LIGHTS);
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
      pointShadowMaps.emplace_back(POINT_SHADOW_SIZE);
    }
  }

  void setScreenSize(unsigned int width, unsigned int height) {
    screenWidth = width;
    screenHeight = height;
  }

  // Getters
  unsigned int getShadowMapTexture() const { return shadowMap.depthTexture; }
  const glm::mat4 &getLightSpaceMatrix() const { return lightSpaceMatrix; }
  float getPointLightFarPlane() const { return pointLightFarPlane; }

  // Configuration setters
  void setOrthoSize(float size) { orthoSize = size; }
  void setNearFarPlanes(float near, float far) {
    nearPlane = near;
    farPlane = far;
  }
  void setLightDistance(float distance) { lightDistance = distance; }
  void setPointLightFarPlane(float far) { pointLightFarPlane = far; }

  void render() override {
    renderDirectionalShadowMap();
    renderPointShadowMaps();

    // Restore viewport for main rendering
    glViewport(0, 0, screenWidth, screenHeight);
  }

private:
  void renderDirectionalShadowMap() {
    if (!shadowMap.isValid() || depthShaderID == 0)
      return;

    // Find directional light
    DirectionalLightComponent *dirLight = nullptr;
    gWorld.forEachWith<DirectionalLightComponent>(
        [&](Entity entity, DirectionalLightComponent &light) {
          if (!dirLight) {
            dirLight = &light;
          }
        });

    if (!dirLight)
      return;

    // Calculate light space matrix
    calculateLightSpaceMatrix(dirLight->direction);

    auto &resources = ResourceManager::instance();
    Shader *depthShader = resources.getShader(depthShaderID);
    if (!depthShader)
      return;

    // Render to shadow map
    shadowMap.bind();
    glCullFace(GL_FRONT);

    depthShader->use();
    depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    renderSceneDepth(depthShader);

    glCullFace(GL_BACK);
    shadowMap.unbind();

    // Pass directional shadow data to LightingSystem
    if (auto *lightingSystem = gWorld.getSystem<LightingSystem>()) {
      lightingSystem->setDirectionalShadowData(lightSpaceMatrix,
                                               shadowMap.depthTexture);
    }
  }

  void renderPointShadowMaps() {
    if (pointDepthShaderID == 0)
      return;

    auto &resources = ResourceManager::instance();
    Shader *pointDepthShader = resources.getShader(pointDepthShaderID);
    if (!pointDepthShader)
      return;

    // Collect point lights
    struct PointLightData {
      glm::vec3 position;
      int index;
    };
    std::vector<PointLightData> pointLights;

    int index = 0;
    gWorld.forEachWith<PointLightComponent, TransformComponent>(
        [&](Entity entity, PointLightComponent &light,
            TransformComponent &transform) {
          if (index >= MAX_POINT_LIGHTS)
            return;

          PointLightData data;
          data.position = transform.position;
          data.index = index;
          pointLights.push_back(data);
          index++;
        });

    // Render shadow map for each point light
    float aspect = 1.0f; // Cubemap faces are square
    float near = 0.1f;
    float far = pointLightFarPlane;
    glm::mat4 shadowProj =
        glm::perspective(glm::radians(90.0f), aspect, near, far);

    for (const auto &light : pointLights) {
      if (light.index >= (int)pointShadowMaps.size())
        continue;

      PointShadowMap &psm = pointShadowMaps[light.index];
      if (!psm.isValid())
        continue;

      // Calculate 6 view matrices for cubemap faces
      std::array<glm::mat4, 6> shadowTransforms;
      shadowTransforms[0] =
          shadowProj * glm::lookAt(light.position,
                                   light.position + glm::vec3(1.0f, 0.0f, 0.0f),
                                   glm::vec3(0.0f, -1.0f, 0.0f)); // +X
      shadowTransforms[1] =
          shadowProj *
          glm::lookAt(light.position,
                      light.position + glm::vec3(-1.0f, 0.0f, 0.0f),
                      glm::vec3(0.0f, -1.0f, 0.0f)); // -X
      shadowTransforms[2] =
          shadowProj * glm::lookAt(light.position,
                                   light.position + glm::vec3(0.0f, 1.0f, 0.0f),
                                   glm::vec3(0.0f, 0.0f, 1.0f)); // +Y
      shadowTransforms[3] =
          shadowProj *
          glm::lookAt(light.position,
                      light.position + glm::vec3(0.0f, -1.0f, 0.0f),
                      glm::vec3(0.0f, 0.0f, -1.0f)); // -Y
      shadowTransforms[4] =
          shadowProj * glm::lookAt(light.position,
                                   light.position + glm::vec3(0.0f, 0.0f, 1.0f),
                                   glm::vec3(0.0f, -1.0f, 0.0f)); // +Z
      shadowTransforms[5] =
          shadowProj *
          glm::lookAt(light.position,
                      light.position + glm::vec3(0.0f, 0.0f, -1.0f),
                      glm::vec3(0.0f, -1.0f, 0.0f)); // -Z

      psm.bind();

      pointDepthShader->use();
      for (int i = 0; i < 6; ++i) {
        pointDepthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]",
                                  shadowTransforms[i]);
      }
      pointDepthShader->setVec3("lightPos", light.position);
      pointDepthShader->setFloat("far_plane", far);

      renderSceneDepth(pointDepthShader);

      psm.unbind();
    }

    // Pass point shadow data to LightingSystem
    if (auto *lightingSystem = gWorld.getSystem<LightingSystem>()) {
      std::vector<unsigned int> cubemaps;
      for (size_t i = 0; i < pointLights.size() && i < pointShadowMaps.size();
           ++i) {
        cubemaps.push_back(pointShadowMaps[i].depthCubemap);
      }
      lightingSystem->setPointShadowData(cubemaps, pointLightFarPlane);
    }
  }

  void renderSceneDepth(Shader *shader) {
    gWorld.forEachWith<TransformComponent, MeshComponent, MaterialComponent>(
        [&](Entity entity, TransformComponent &transform, MeshComponent &mesh,
            MaterialComponent &material) {
          if (!mesh.isValid())
            return;

          // Skip transparent objects for shadow mapping
          if (material.hasTransparency)
            return;

          shader->setMat4("model", transform.getModelMatrix());
          RenderUtils::drawMesh(mesh);
        });
  }

  void calculateLightSpaceMatrix(const glm::vec3 &lightDirection) {
    glm::vec3 lightDir = glm::normalize(lightDirection);
    glm::vec3 lightPos = -lightDir * lightDistance;

    glm::mat4 lightProjection =
        glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane,
                   farPlane);
    glm::mat4 lightView =
        glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    lightSpaceMatrix = lightProjection * lightView;
  }
};
