#ifndef MATERIAL_COMPONENT_HPP
#define MATERIAL_COMPONENT_HPP

#include "../shader_h.hpp"
#include "../texture_2d_h.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

// Material Component - holds appearance data (shader, uniforms, textures)
struct MaterialComponent {
  using UniformValue = std::variant<float, int, bool, glm::vec3, glm::mat4>;

  struct TextureInfo {
    std::shared_ptr<Texture2D> texture;
    int slot;
  };

  std::shared_ptr<Shader> shader;
  std::unordered_map<std::string, UniformValue> uniforms;
  std::unordered_map<std::string, TextureInfo> textures;

  // Material properties
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float shininess;

  MaterialComponent(std::shared_ptr<Shader> shaderProgram,
                    glm::vec3 amb = glm::vec3(0.1f),
                    glm::vec3 diff = glm::vec3(0.8f),
                    glm::vec3 spec = glm::vec3(1.0f), float shin = 32.0f)
      : shader(shaderProgram), ambient(amb), diffuse(diff), specular(spec),
        shininess(shin) {}

  MaterialComponent() : shader(nullptr), ambient(0.1f), diffuse(0.8f),
                        specular(1.0f), shininess(32.0f) {}

  // Helper methods to set uniforms
  void setFloat(const std::string &name, float value) { uniforms[name] = value; }
  void setInt(const std::string &name, int value) { uniforms[name] = value; }
  void setBool(const std::string &name, bool value) { uniforms[name] = value; }
  void setVec3(const std::string &name, const glm::vec3 &value) {
    uniforms[name] = value;
  }
  void setMat4(const std::string &name, const glm::mat4 &value) {
    uniforms[name] = value;
  }

  void setTexture(const std::string &name, std::shared_ptr<Texture2D> texture,
                  int slot = 0) {
    textures[name] = {texture, slot};
  }

  // Apply material to shader (bind textures and set uniforms)
  void use() const {
    if (!shader)
      return;

    shader->use();
    shader->setVec3("material.vAmbient", ambient);
    shader->setVec3("material.vDiffuse", diffuse);
    shader->setVec3("material.vSpecular", specular);
    shader->setFloat("material.shininess", shininess);

    // Bind textures
    for (const auto &[name, texInfo] : textures) {
      texInfo.texture->bind(texInfo.slot);
      shader->setInt(name.c_str(), texInfo.slot);
    }

    // Set custom uniforms
    for (const auto &[name, value] : uniforms) {
      std::visit(
          [this, &name](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, float>) {
              shader->setFloat(name.c_str(), arg);
            } else if constexpr (std::is_same_v<T, int>) {
              shader->setInt(name.c_str(), arg);
            } else if constexpr (std::is_same_v<T, bool>) {
              shader->setBool(name.c_str(), arg);
            } else if constexpr (std::is_same_v<T, glm::vec3>) {
              shader->setVec3(name.c_str(), arg);
            } else if constexpr (std::is_same_v<T, glm::mat4>) {
              shader->setMat4(name.c_str(), arg);
            }
          },
          value);
    }
  }

  Shader *getShader() const { return shader.get(); }
};

#endif // MATERIAL_COMPONENT_HPP
