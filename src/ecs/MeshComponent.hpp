#ifndef MESH_COMPONENT_HPP
#define MESH_COMPONENT_HPP

#include "../components/Mesh.hpp"
#include <memory>

// Mesh Component - holds geometry data for rendering
struct MeshComponent {
  std::shared_ptr<Mesh> mesh;
  unsigned int vertexCount;
  bool indexed;        // True if using indexed drawing (glDrawElements)
  unsigned int indexCount; // Only used if indexed = true

  // Constructor for non-indexed mesh
  MeshComponent(std::shared_ptr<Mesh> meshData, unsigned int vCount)
      : mesh(meshData), vertexCount(vCount), indexed(false), indexCount(0) {}

  // Constructor for indexed mesh
  MeshComponent(std::shared_ptr<Mesh> meshData, unsigned int iCount, bool isIndexed)
      : mesh(meshData), vertexCount(0), indexed(isIndexed), indexCount(iCount) {}

  MeshComponent() : mesh(nullptr), vertexCount(0), indexed(false), indexCount(0) {}
};

#endif // MESH_COMPONENT_HPP
