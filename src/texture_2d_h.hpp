#ifndef TEXTURE_2D_HPP
#define TEXTURE_2D_HPP

#include "gl_common.hpp"
#include <initializer_list>
#include <iostream>
#include <stddef.h>
#include <string>

#include "stb_image.h"

struct TextureParam {
  GLenum name;
  GLint value;
};

#ifdef DEBUG
static int boundTextureUnits[16];
#endif // DEBUG

class Texture2D {
public:
  Texture2D(std::initializer_list<TextureParam> params) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    for (const TextureParam &p : params) {
      glTexParameteri(GL_TEXTURE_2D, p.name, p.value);
    }
  }

  Texture2D() {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
  }

  void loadImage(const std::string &path) {
    int width, height, nrChannels;

    unsigned char *data =
        stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
  }

  void loadImageFromMemory(const unsigned char *buffer, int len) {
    int width, height, nrChannels;
    unsigned char *data =
        stbi_load_from_memory(buffer, len, &width, &height, &nrChannels, 0);
    if (data) {
      GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
      stbi_image_free(data);
    } else {
      std::cerr << "Failed to load embedded texture from memory" << std::endl;
    }
  }

  void loadImageRaw(const unsigned char *data, int width, int height,
                    int channels) {
    if (data) {
      GLenum format;
      if (channels == 1)
        format = GL_RED;
      else if (channels == 3)
        format = GL_RGB;
      else if (channels == 4)
        format = GL_RGBA;
      else {
        std::cerr << "Unsupported channel count: " << channels << std::endl;
        return;
      }
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      std::cerr << "Failed to load raw embedded texture" << std::endl;
    }
  }

  unsigned int loadTexture(char const *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
      GLenum format;
      if (nrComponents == 1)
        format = GL_RED;
      else if (nrComponents == 3)
        format = GL_RGB;
      else if (nrComponents == 4)
        format = GL_RGBA;

      glBindTexture(GL_TEXTURE_2D, textureID);
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                   GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      stbi_image_free(data);
    } else {
      std::cout << "Texture failed to load at path: " << path << std::endl;
      stbi_image_free(data);
    }

    return textureID;
  }

  void bind(unsigned int slot = 0) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);

#ifdef DEBUG
    if (boundTextureUnits[slot] != ID) {
      boundTextureUnits[slot] = ID;
    }
#endif
  }

  unsigned int getID() const { return ID; }

  ~Texture2D() { glDeleteTextures(1, &ID); }

private:
  unsigned int ID{};
};

#endif // TEXTURE_2D_HPP
