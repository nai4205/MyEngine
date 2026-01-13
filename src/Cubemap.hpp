#pragma once
#include "gl_common.hpp"
#include "texture_2d_h.hpp"

#include <initializer_list>
class Cubemap {
public:
  Cubemap(std::initializer_list<TextureParam> params) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    for (const TextureParam &p : params) {
      glTexParameteri(GL_TEXTURE_CUBE_MAP, p.name, p.value);
    }
  };
  Cubemap() {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
  }

  void loadImage(int &width, int &height, int &nrChannels,
                 const std::string &path, int index) {
    unsigned char *data =
        stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 0, GL_RGB, width,
                   height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    } else {
      std::cout << "Cubemap texture failed to load at path " << path
                << std::endl;
      stbi_image_free(data);
    }
  }
  unsigned int getID() const { return ID; }

private:
  unsigned int ID{};
};
