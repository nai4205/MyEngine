#ifndef MESH_COMPONENT_HPP
#define MESH_COMPONENT_HPP

#include "Mesh.hpp"
#include "MeshIndexed.hpp"
#include <memory>
#include <variant>

// Mesh Component - holds geometry data for rendering
struct MeshComponent {
  std::variant<std::monostate, std::shared_ptr<Mesh>,
               std::shared_ptr<MeshIndexed>>
      mesh;
  unsigned int vertexCount;   // For non-indexed meshes
  unsigned int indexCount;    // For indexed meshes
  bool indexed;

  // Constructor for non-indexed mesh
  MeshComponent(std::shared_ptr<Mesh> meshData, unsigned int vCount)
      : mesh(meshData), vertexCount(vCount), indexCount(0), indexed(false) {}

  // Constructor for indexed mesh
  MeshComponent(std::shared_ptr<MeshIndexed> meshData, unsigned int iCount)
      : mesh(meshData), vertexCount(0), indexCount(iCount), indexed(true) {}

  MeshComponent()
      : mesh(std::monostate{}), vertexCount(0), indexCount(0), indexed(false) {}

  bool hasMesh() const {
    return !std::holds_alternative<std::monostate>(mesh);
  }

  Mesh *getMesh() {
    if (auto *ptr = std::get_if<std::shared_ptr<Mesh>>(&mesh)) {
      return ptr->get();
    }
    return nullptr;
  }

  MeshIndexed *getMeshIndexed() {
    if (auto *ptr = std::get_if<std::shared_ptr<MeshIndexed>>(&mesh)) {
      return ptr->get();
    }
    return nullptr;
  }
};

#endif // MESH_COMPONENT_HPP
