#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "../shader_h.hpp"
#include "../texture_2d_h.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

class Material {
public:
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float shininess;
  glm::vec3 color;

  using UniformValue = std::variant<float, int, glm::vec3, glm::mat4>;

  Material(std::shared_ptr<Shader> shaderProgram,
           glm::vec3 amb = glm::vec3(0.1f), glm::vec3 diff = glm::vec3(0.8f),
           glm::vec3 spec = glm::vec3(1.0f), float shin = 32.0f)
      : shader(shaderProgram), ambient(amb), diffuse(diff), specular(spec),
        shininess(shin) {}

  void setFloat(const std::string &name, float value) {
    uniforms[name] = value;
  }

  void setInt(const std::string &name, int value) { uniforms[name] = value; }

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
  void setBool(const std::string &name, bool value) { uniforms[name] = value; }

  void use() const {
    shader->use();
    shader->setVec3("material.vAmbient", ambient);
    shader->setVec3("material.vDiffuse", diffuse);
    shader->setVec3("material.vSpecular", specular);
    shader->setFloat("material.shininess", shininess);

    for (const auto &[name, texInfo] : textures) {
      texInfo.texture->bind(texInfo.slot);
      shader->setInt(name.c_str(), texInfo.slot);
    }

    for (const auto &[name, value] : uniforms) {
      std::visit(
          [this, &name](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, float>) {
              shader->setFloat(name.c_str(), arg);
            } else if constexpr (std::is_same_v<T, int>) {
              shader->setInt(name.c_str(), arg);
            } else if constexpr (std::is_same_v<T, glm::vec3>) {
              shader->setVec3(name.c_str(), arg);
            } else if constexpr (std::is_same_v<T, glm::mat4>) {
              shader->setMat4(name.c_str(), arg);
            } else if constexpr (std::is_same_v<T, bool>) {
              shader->setBool(name.c_str(), arg);
            }
          },
          value);
    }
  }

  Shader *getShader() const { return shader.get(); }

private:
  struct TextureInfo {
    std::shared_ptr<Texture2D> texture;
    int slot;
  };

  std::shared_ptr<Shader> shader;
  std::unordered_map<std::string, UniformValue> uniforms;
  std::unordered_map<std::string, TextureInfo> textures;
};

#endif
