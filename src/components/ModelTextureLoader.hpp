#ifndef MODEL_TEXTURE_LOADER_HPP
#define MODEL_TEXTURE_LOADER_HPP

#include "../texture_2d_h.hpp"
#include <memory>
#include <string>
#include <unordered_map>

// Helper for loading and caching textures when loading models
// Prevents loading the same texture multiple times
class ModelTextureLoader {
public:
  // Load a texture from file, with caching to avoid duplicates
  static std::shared_ptr<Texture2D> loadTexture(const std::string &path,
                                                const std::string &directory) {
    std::string filename = directory + '/' + path;
    auto &loadedTextures = getLoadedTextures();

    // Check if already loaded
    if (loadedTextures.find(filename) != loadedTextures.end()) {
      return loadedTextures[filename];
    }

    // Load new texture
    auto texture = std::make_shared<Texture2D>();
    texture->loadImage(filename.c_str());

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    loadedTextures[filename] = texture;
    return texture;
  }

  // Clear texture cache
  static void clearCache() { getLoadedTextures().clear(); }

private:
  static std::unordered_map<std::string, std::shared_ptr<Texture2D>> &
  getLoadedTextures() {
    static std::unordered_map<std::string, std::shared_ptr<Texture2D>>
        loadedTextures;
    return loadedTextures;
  }
};

#endif
