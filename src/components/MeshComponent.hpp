#pragma once
#include <GL/gl.h>
#include <stdint.h>

struct MeshComponent {
  uint32_t vao = 0;
  uint32_t vertexCount = 0;
  uint32_t indexCount = 0;
  GLenum indexType = GL_UNSIGNED_INT;

  bool isIndexed() const { return indexCount > 0; }
  bool isValid() const { return vao != 0; }
};
