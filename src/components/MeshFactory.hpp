#ifndef MESH_FACTORY_HPP
#define MESH_FACTORY_HPP

#include "Mesh.hpp"
#include <memory>

// Helper functions to create common mesh layouts
namespace MeshFactory {

inline std::shared_ptr<Mesh> createPositionOnlyMesh(const float *vertices,
                                                    size_t sizeInBytes) {
  std::vector<VertexAttribute> layout = {
      {0, 3, GL_FLOAT, false, 3 * sizeof(float), (void *)0} // position
  };
  return std::make_shared<Mesh>(vertices, sizeInBytes, layout);
}

// Create a mesh with position + normal data (6 floats per vertex)
inline std::shared_ptr<Mesh> createPositionNormalMesh(const float *vertices,
                                                      size_t sizeInBytes) {
  std::vector<VertexAttribute> layout = {
      {0, 3, GL_FLOAT, false, 6 * sizeof(float), (void *)0}, // position
      {1, 3, GL_FLOAT, false, 6 * sizeof(float),
       (void *)(3 * sizeof(float))} // normal
  };
  return std::make_shared<Mesh>(vertices, sizeInBytes, layout);
}

// Create a mesh with position + normal + texCoord data (8 floats per vertex)
inline std::shared_ptr<Mesh>
createPositionNormalTexMesh(const float *vertices, size_t sizeInBytes) {
  std::vector<VertexAttribute> layout = {
      {0, 3, GL_FLOAT, false, 8 * sizeof(float), (void *)0}, // position
      {1, 3, GL_FLOAT, false, 8 * sizeof(float),
       (void *)(3 * sizeof(float))}, // normal
      {2, 2, GL_FLOAT, false, 8 * sizeof(float),
       (void *)(6 * sizeof(float))} // texCoord
  };
  return std::make_shared<Mesh>(vertices, sizeInBytes, layout);
}
} // namespace MeshFactory

#endif
