#pragma once

#include "Cubemap.hpp"
#include "Framebuffer.hpp"
#include "shader_h.hpp"
#include "texture_2d_h.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct MeshData {
  uint32_t vao = 0;
  uint32_t vbo = 0;
  uint32_t ebo = 0;
  uint32_t vertexCount = 0;
  uint32_t indexCount = 0;
};

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

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

class ResourceManager {
public:
  static ResourceManager &instance() {
    static ResourceManager inst;
    return inst;
  }

  // ========== SHADERS ==========
  uint32_t loadShader(const std::string &name, const char *vertexPath,
                      const char *fragmentPath) {
    auto shader = std::make_shared<Shader>(vertexPath, fragmentPath);
    uint32_t id = shader->ID;
    shaders[name] = shader;
    shadersByID[id] = shader;
    return id;
  }

  Shader *getShader(uint32_t id) {
    auto it = shadersByID.find(id);
    return (it != shadersByID.end()) ? it->second.get() : nullptr;
  }

  Shader *getShader(const std::string &name) {
    auto it = shaders.find(name);
    return (it != shaders.end()) ? it->second.get() : nullptr;
  }

  uint32_t getShaderID(const std::string &name) {
    auto it = shaders.find(name);
    return (it != shaders.end()) ? it->second->ID : 0;
  }

  // ========== TEXTURES ==========
  uint32_t loadTexture(const std::string &path, bool flipY = true) {
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
      return it->second->getID();
    }

    stbi_set_flip_vertically_on_load(flipY);

    auto texture = std::make_shared<Texture2D>();
    texture->loadImage(path);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    uint32_t id = texture->getID();
    textureCache[path] = texture;
    texturesByID[id] = texture;
    return id;
  }

  // Cubemap
  uint32_t loadCubemapTexture(const std::string &path) {
    int width, height, nrChannels;
    std::vector<std::string> faces = {
        path + "right.jpg",  path + "left.jpg",  path + "top.jpg",
        path + "bottom.jpg", path + "front.jpg", path + "back.jpg",
    };

    auto cubemap = std::make_shared<Cubemap>();
    auto it = cubemapCache.find(path);
    if (it != cubemapCache.end()) {
      return it->second->getID();
    }
    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < faces.size(); i++) {
      cubemap->loadImage(width, height, nrChannels, faces[i], i);
    }

    stbi_set_flip_vertically_on_load(false);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    uint32_t id = cubemap->getID();
    cubemapCache[path] = cubemap;
    return id;
  }

  // ========== MESHES ==========
  MeshData createMesh(const float *positions, size_t positionsSize,
                      const float *normals, size_t normalsSize,
                      const float *texCoords, size_t texCoordsSize,
                      uint32_t vertexCount) {
    MeshData data;
    data.vertexCount = vertexCount;
    data.indexCount = 0;

    size_t totalSize = positionsSize + normalsSize + texCoordsSize;

    glGenVertexArrays(1, &data.vao);
    glGenBuffers(1, &data.vbo);

    glBindVertexArray(data.vao);
    glBindBuffer(GL_ARRAY_BUFFER, data.vbo);

    // Allocate buffer with total size
    glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);

    // Upload each attribute array to its region
    glBufferSubData(GL_ARRAY_BUFFER, 0, positionsSize, positions);
    glBufferSubData(GL_ARRAY_BUFFER, positionsSize, normalsSize, normals);
    glBufferSubData(GL_ARRAY_BUFFER, positionsSize + normalsSize, texCoordsSize,
                    texCoords);

    // Position (location 0) - 3 floats, stride 0 (tightly packed)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // Normal (location 1) - 3 floats
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)positionsSize);

    // TexCoords (location 2) - 2 floats
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0,
                          (void *)(positionsSize + normalsSize));

    glBindVertexArray(0);

    meshes.emplace_back(data);
    return data;
  }

  // Creates a mesh with interleaved vertex data and custom attributes
  MeshData createMesh(const float *vertices, size_t sizeInBytes,
                      const std::vector<VertexAttribute> &attributes,
                      uint32_t vertexCount) {
    MeshData data;
    data.vertexCount = vertexCount;
    data.indexCount = 0;

    glGenVertexArrays(1, &data.vao);
    glGenBuffers(1, &data.vbo);

    glBindVertexArray(data.vao);
    glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertices, GL_STATIC_DRAW);

    for (const VertexAttribute &attr : attributes) {
      glVertexAttribPointer(attr.location, attr.componentCount, attr.type,
                            attr.normalized ? GL_TRUE : GL_FALSE, attr.stride,
                            attr.offset);
      glEnableVertexAttribArray(attr.location);
    }

    glBindVertexArray(0);

    meshes.emplace_back(data);
    return data;
  }

  // Creates an indexed mesh with separate attribute arrays (non-interleaved
  // layout) using glBufferSubData to upload each attribute to a different
  // region
  MeshData createIndexedMesh(const float *positions, size_t positionsSize,
                             const float *normals, size_t normalsSize,
                             const float *texCoords, size_t texCoordsSize,
                             const unsigned int *indices, size_t indicesCount,
                             uint32_t vertexCount) {
    MeshData data;
    data.vertexCount = vertexCount;
    data.indexCount = static_cast<uint32_t>(indicesCount);

    size_t totalSize = positionsSize + normalsSize + texCoordsSize;

    glGenVertexArrays(1, &data.vao);
    glGenBuffers(1, &data.vbo);
    glGenBuffers(1, &data.ebo);

    glBindVertexArray(data.vao);

    glBindBuffer(GL_ARRAY_BUFFER, data.vbo);

    // Allocate buffer with total size
    glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);

    // Upload each attribute array to its region
    glBufferSubData(GL_ARRAY_BUFFER, 0, positionsSize, positions);
    glBufferSubData(GL_ARRAY_BUFFER, positionsSize, normalsSize, normals);
    glBufferSubData(GL_ARRAY_BUFFER, positionsSize + normalsSize, texCoordsSize,
                    texCoords);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(unsigned int),
                 indices, GL_STATIC_DRAW);

    // Position (location 0) - 3 floats, stride 0 (tightly packed)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // Normal (location 1) - 3 floats
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)positionsSize);

    // TexCoords (location 2) - 2 floats
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0,
                          (void *)(positionsSize + normalsSize));

    glBindVertexArray(0);

    meshes.emplace_back(data);
    return data;
  }

  // Creates an indexed mesh with interleaved vertex data and custom attributes
  MeshData createIndexedMesh(const float *vertices, size_t verticesSizeInBytes,
                             const unsigned int *indices, size_t indicesCount,
                             const std::vector<VertexAttribute> &attributes,
                             uint32_t vertexCount) {
    MeshData data;
    data.vertexCount = vertexCount;
    data.indexCount = static_cast<uint32_t>(indicesCount);

    glGenVertexArrays(1, &data.vao);
    glGenBuffers(1, &data.vbo);
    glGenBuffers(1, &data.ebo);

    glBindVertexArray(data.vao);

    glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
    glBufferData(GL_ARRAY_BUFFER, verticesSizeInBytes, vertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(unsigned int),
                 indices, GL_STATIC_DRAW);

    for (const VertexAttribute &attr : attributes) {
      glVertexAttribPointer(attr.location, attr.componentCount, attr.type,
                            attr.normalized ? GL_TRUE : GL_FALSE, attr.stride,
                            attr.offset);
      glEnableVertexAttribArray(attr.location);
    }

    glBindVertexArray(0);

    meshes.emplace_back(data);
    return data;
  }

  MeshData createIndexedMesh(const std::vector<Vertex> &vertices,
                             const std::vector<uint32_t> &indices) {
    MeshData data;
    data.vertexCount = static_cast<uint32_t>(vertices.size());
    data.indexCount = static_cast<uint32_t>(indices.size());

    glGenVertexArrays(1, &data.vao);
    glGenBuffers(1, &data.vbo);
    glGenBuffers(1, &data.ebo);

    glBindVertexArray(data.vao);

    glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                 indices.data(), GL_STATIC_DRAW);

    // Position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

    // Normal (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, Normal));

    // TexCoords (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);

    meshes.emplace_back(data);
    return data;
  }

  // ========== FRAMEBUFFERS ==========
  Framebuffer *createFramebuffer(const std::string &name, unsigned int width,
                                 unsigned int height) {
    auto fb = std::make_unique<Framebuffer>(width, height);
    Framebuffer *ptr = fb.get();
    framebuffers[name] = std::move(fb);
    return ptr;
  }

  Framebuffer *getFramebuffer(const std::string &name) {
    auto it = framebuffers.find(name);
    return (it != framebuffers.end()) ? it->second.get() : nullptr;
  }

  void resizeFramebuffer(const std::string &name, unsigned int width,
                         unsigned int height) {
    auto it = framebuffers.find(name);
    if (it != framebuffers.end()) {
      it->second->resize(width, height);
    }
  }

  void setClearColorForFramebuffer(const std::string &name,
                                   glm::vec3 clearColor) {
    auto it = framebuffers.find(name);
    if (it == framebuffers.end())
      return;
    auto frameBuffer = it->second.get();
    frameBuffer->clearColor = clearColor;
  }

  void cleanup() {
    for (auto &mesh : meshes) {
      if (mesh.vao)
        glDeleteVertexArrays(1, &mesh.vao);
      if (mesh.vbo)
        glDeleteBuffers(1, &mesh.vbo);
      if (mesh.ebo)
        glDeleteBuffers(1, &mesh.ebo);
    }
    meshes.clear();
    shaders.clear();
    shadersByID.clear();
    textureCache.clear();
    texturesByID.clear();
    framebuffers.clear();
  }

  ~ResourceManager() { cleanup(); }

private:
  ResourceManager() = default;
  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator=(const ResourceManager &) = delete;

  std::vector<MeshData> meshes;
  std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
  std::unordered_map<uint32_t, std::shared_ptr<Shader>> shadersByID;
  std::unordered_map<std::string, std::shared_ptr<Texture2D>> textureCache;
  std::unordered_map<std::string, std::shared_ptr<Cubemap>> cubemapCache;
  std::unordered_map<uint32_t, std::shared_ptr<Texture2D>> texturesByID;
  std::unordered_map<std::string, std::unique_ptr<Framebuffer>> framebuffers;
};
