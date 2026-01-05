#ifndef MESH_INDEXED_HPP
#define MESH_INDEXED_HPP

#include "Mesh.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

struct Texture {
  unsigned int id;
  std::string type;
};

class MeshIndexed {
public:
  std::vector<Texture> textures;

  MeshIndexed(const std::vector<Vertex> &vertices,
              const std::vector<unsigned int> &indices,
              const std::vector<Texture> &textures = {})
      : textures(textures), indexCount(indices.size()) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 &vertices[0], GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    // Vertex positions (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

    // Vertex normals (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Normal));

    // Vertex texture coords (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
  }

  MeshIndexed(const float *vertices, size_t vertexSizeInBytes,
              const unsigned int *indices, size_t indexCount,
              const std::vector<VertexAttribute> &attributes)
      : indexCount(indexCount) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexSizeInBytes, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int),
                 indices, GL_STATIC_DRAW);

    for (const VertexAttribute &attr : attributes) {
      glVertexAttribPointer(attr.location, attr.componentCount, attr.type,
                            attr.normalized ? GL_TRUE : GL_FALSE, attr.stride,
                            attr.offset);
      glEnableVertexAttribArray(attr.location);
    }

    glBindVertexArray(0);
  }

  void bind() const { glBindVertexArray(VAO); }

  void unbind() const { glBindVertexArray(0); }

  unsigned int getIndexCount() const { return indexCount; }

  ~MeshIndexed() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
  }

  // Prevent copying (VAO/VBO/EBO are unique resources)
  MeshIndexed(const MeshIndexed &) = delete;
  MeshIndexed &operator=(const MeshIndexed &) = delete;

private:
  unsigned int VAO{}, VBO{}, EBO{};
  unsigned int indexCount;
};

#endif
