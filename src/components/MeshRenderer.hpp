#ifndef MESH_RENDERER_HPP
#define MESH_RENDERER_HPP

#include "Material.hpp"
#include "Mesh.hpp"
#include <memory>

// MeshRenderer component - combines mesh (geometry) and material (appearance)
// Handles the actual rendering
class MeshRenderer {
public:
  MeshRenderer(std::shared_ptr<Mesh> meshData, std::shared_ptr<Material> mat,
               unsigned int vertCount = 36)
      : mesh(meshData), material(mat), vertexCount(vertCount) {}

  // Render the mesh with the material
  void render() const {
    material->use();
    mesh->bind();
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    mesh->unbind();
  }

  // Getters for modifying components
  Material *getMaterial() const { return material.get(); }
  Mesh *getMesh() const { return mesh.get(); }

  void setVertexCount(unsigned int count) { vertexCount = count; }

private:
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Material> material;
  unsigned int vertexCount;
};

#endif
