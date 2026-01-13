#pragma once

#include "../gl_common.hpp"
#include "../components/MaterialComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/SceneComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/Tag.hpp"
#include "../ecs/World.hpp"

// Shared data structure for renderable entities
struct RenderableEntity {
  Entity entity;
  TransformComponent *transform;
  MeshComponent *mesh;
  MaterialComponent *material;
  TagComponent *tag;
};

namespace RenderUtils {

// Helper function to draw a mesh
inline void drawMesh(const MeshComponent &mesh) {
  glBindVertexArray(mesh.vao);
  if (mesh.isIndexed()) {
    glDrawElements(GL_TRIANGLES, mesh.indexCount, mesh.indexType, nullptr);
  } else {
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
  }
  glBindVertexArray(0);
}

// Helper function to get the active scene name
inline std::string getActiveSceneName(World &world) {
  std::string activeSceneName;
  world.forEachWith<SceneComponent, TagComponent>(
      [&](Entity entity, SceneComponent &scene, TagComponent &tag) {
        if (tag.has(ACTIVESCENE)) {
          activeSceneName = scene.name;
        }
      });
  return activeSceneName;
}

// Helper function to get clear color from active scene
inline glm::vec3 getActiveSceneClearColor(World &world) {
  glm::vec3 clearColor(0.0f);
  world.forEachWith<SceneComponent, TagComponent>(
      [&](Entity entity, SceneComponent &scene, TagComponent &tag) {
        if (tag.has(ACTIVESCENE)) {
          clearColor = scene.clearColor;
        }
      });
  return clearColor;
}

} // namespace RenderUtils
