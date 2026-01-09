#pragma once

#include "../gl_common.hpp"
#include <iostream>

class Framebuffer {
public:
  unsigned int fbo = 0;
  unsigned int colorTexture = 0;
  unsigned int rbo = 0;
  unsigned int width = 0;
  unsigned int height = 0;

  Framebuffer() = default;

  Framebuffer(unsigned int w, unsigned int h) : width(w), height(h) {
    create();
  }

  ~Framebuffer() { cleanup(); }

  // Delete copy constructor and assignment
  Framebuffer(const Framebuffer &) = delete;
  Framebuffer &operator=(const Framebuffer &) = delete;

  // Move constructor and assignment
  Framebuffer(Framebuffer &&other) noexcept
      : fbo(other.fbo), colorTexture(other.colorTexture), rbo(other.rbo),
        width(other.width), height(other.height) {
    other.fbo = 0;
    other.colorTexture = 0;
    other.rbo = 0;
  }

  Framebuffer &operator=(Framebuffer &&other) noexcept {
    if (this != &other) {
      cleanup();
      fbo = other.fbo;
      colorTexture = other.colorTexture;
      rbo = other.rbo;
      width = other.width;
      height = other.height;
      other.fbo = 0;
      other.colorTexture = 0;
      other.rbo = 0;
    }
    return *this;
  }

  void create() {
    // Generate framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create color texture attachment
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           colorTexture, 0);

    // Create renderbuffer for depth and stencil
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, rbo);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
                << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void bind() const { glBindFramebuffer(GL_FRAMEBUFFER, fbo); }

  void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

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
    if (colorTexture) {
      glDeleteTextures(1, &colorTexture);
      colorTexture = 0;
    }
    if (rbo) {
      glDeleteRenderbuffers(1, &rbo);
      rbo = 0;
    }
  }

  bool isValid() const { return fbo != 0; }
};
