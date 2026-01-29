#pragma once

#include "../gl_common.hpp"
#include <iostream>

class PointShadowMap {
public:
  unsigned int fbo = 0;
  unsigned int depthCubemap = 0;
  unsigned int width = 1024;
  unsigned int height = 1024;

  PointShadowMap() = default;

  PointShadowMap(unsigned int size) : width(size), height(size) { create(); }

  ~PointShadowMap() { cleanup(); }

  // Delete copy constructor and assignment
  PointShadowMap(const PointShadowMap &) = delete;
  PointShadowMap &operator=(const PointShadowMap &) = delete;

  // Move constructor and assignment
  PointShadowMap(PointShadowMap &&other) noexcept
      : fbo(other.fbo), depthCubemap(other.depthCubemap), width(other.width),
        height(other.height) {
    other.fbo = 0;
    other.depthCubemap = 0;
  }

  PointShadowMap &operator=(PointShadowMap &&other) noexcept {
    if (this != &other) {
      cleanup();
      fbo = other.fbo;
      depthCubemap = other.depthCubemap;
      width = other.width;
      height = other.height;
      other.fbo = 0;
      other.depthCubemap = 0;
    }
    return *this;
  }

  void create() {
    // Generate framebuffer
    glGenFramebuffers(1, &fbo);

    // Create depth cubemap
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    // Allocate storage for each face
    for (unsigned int i = 0; i < 6; ++i) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                   width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Attach cubemap to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr
          << "ERROR::POINTSHADOWMAP:: Point shadow map framebuffer is not complete!"
          << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void bind() const {
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
  }

  void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

  void bindTexture(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
  }

  void cleanup() {
    if (fbo) {
      glDeleteFramebuffers(1, &fbo);
      fbo = 0;
    }
    if (depthCubemap) {
      glDeleteTextures(1, &depthCubemap);
      depthCubemap = 0;
    }
  }

  bool isValid() const { return fbo != 0 && depthCubemap != 0; }
};
