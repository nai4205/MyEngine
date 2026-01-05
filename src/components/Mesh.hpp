#ifndef MESH_HPP
#define MESH_HPP

#include <glad/gl.h>
#include <vector>

// Describes a single vertex attribute (position, normal, texcoord, etc.)
struct VertexAttribute {
  unsigned int location; // Shader location (0, 1, 2, etc.)
  int componentCount;    // Number of components (3 for vec3, 2 for vec2)
  unsigned int type;     // GL_FLOAT, GL_INT, etc.
  bool normalized;       // Should values be normalized?
  int stride;            // Bytes between consecutive vertices
  const void *offset;    // Offset of this attribute in the vertex data

  VertexAttribute(unsigned int loc, int count, unsigned int t = GL_FLOAT,
                  bool norm = false, int str = 0, const void *off = nullptr)
      : location(loc), componentCount(count), type(t), normalized(norm),
        stride(str), offset(off) {}
};

// Generic Mesh class - handles geometry data (VAO/VBO)
// Separates geometry from materials and transforms
class Mesh {
public:
  Mesh(const float *vertices, size_t sizeInBytes,
       const std::vector<VertexAttribute> &attributes)
      : vertexCount(0) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertices, GL_STATIC_DRAW);

    // Set up all vertex attributes based on the layout
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

  ~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
  }

  // Prevent copying (VAO/VBO are unique resources)
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

private:
  unsigned int VAO{}, VBO{};
  unsigned int vertexCount;
};

#endif
