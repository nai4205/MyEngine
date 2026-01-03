#include <glad/gl.h>
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
    for (const auto &p : params) {
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

  void bind(unsigned int slot = 0) const {

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);

#ifdef DEBUG
    if (boundTextureUnits[slot] != ID) {
      boundTextureUnits[slot] = ID;
    }
#endif
  }

  ~Texture2D() { glDeleteTextures(1, &ID); }

private:
  unsigned int ID{};
};
