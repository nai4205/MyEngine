#pragma once

#include "../gl_common.hpp"
#include <iostream>

class ShadowMap {
public:
  unsigned int fbo = 0;
  unsigned int depthTexture = 0;
  unsigned int width = 1024;
  unsigned int height = 1024;

  ShadowMap() = default;

  ShadowMap(unsigned int w, unsigned int h) : width(w), height(h) { create(); }

  ~ShadowMap() { cleanup(); }

  // Delete copy constructor and assignment
  ShadowMap(const ShadowMap &) = delete;
  ShadowMap &operator=(const ShadowMap &) = delete;

  // Move constructor and assignment
  ShadowMap(ShadowMap &&other) noexcept
      : fbo(other.fbo), depthTexture(other.depthTexture), width(other.width),
        height(other.height) {
    other.fbo = 0;
    other.depthTexture = 0;
  }

  ShadowMap &operator=(ShadowMap &&other) noexcept {
    if (this != &other) {
      cleanup();
      fbo = other.fbo;
      depthTexture = other.depthTexture;
      width = other.width;
      height = other.height;
      other.fbo = 0;
      other.depthTexture = 0;
    }
    return *this;
  }

  void create() {
    // Generate framebuffer
    glGenFramebuffers(1, &fbo);

    // Create depth texture
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Use clamp to border with white (1.0) to avoid shadow outside frustum
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Attach depth texture to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           depthTexture, 0);
    // No color buffer needed
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr << "ERROR::SHADOWMAP:: Shadow map framebuffer is not complete!"
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
    glBindTexture(GL_TEXTURE_2D, depthTexture);
  }

  void resize(unsigned int w, unsigned int h) {
    if (width == w && height == h)
      return;

    width = w;
    height = h;
    cleanup();
    create();
  }

  void cleanup() {
    if (fbo) {
      glDeleteFramebuffers(1, &fbo);
      fbo = 0;
    }
    if (depthTexture) {
      glDeleteTextures(1, &depthTexture);
      depthTexture = 0;
    }
  }

  bool isValid() const { return fbo != 0 && depthTexture != 0; }
};
