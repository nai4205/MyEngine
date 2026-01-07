#ifndef MATERIAL_PRESETS_HPP
#define MATERIAL_PRESETS_HPP

#include "MaterialComponent.hpp"
#include <glm/glm.hpp>
#include <cstdint>
#include <unordered_map>

enum class MaterialType {
  // Gems
  EMERALD,
  JADE,
  OBSIDIAN,
  PEARL,
  RUBY,
  TURQUOISE,

  // Metals
  BRASS,
  BRONZE,
  CHROME,
  COPPER,
  GOLD,
  SILVER,

  // Plastics
  BLACK_PLASTIC,
  CYAN_PLASTIC,
  GREEN_PLASTIC,
  RED_PLASTIC,
  WHITE_PLASTIC,
  YELLOW_PLASTIC,

  // Rubber
  BLACK_RUBBER,
  CYAN_RUBBER,
  GREEN_RUBBER,
  RED_RUBBER,
  WHITE_RUBBER,
  YELLOW_RUBBER
};

struct MaterialProperties {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float shininess;
};

class MaterialPresets {
public:
  static MaterialProperties getProperties(MaterialType type) {
    static const std::unordered_map<MaterialType, MaterialProperties> presets =
        {// Gems
         {MaterialType::EMERALD,
          {{0.0215f, 0.1745f, 0.0215f},
           {0.07568f, 0.61424f, 0.07568f},
           {0.633f, 0.727811f, 0.633f},
           0.6f * 128.0f}},
         {MaterialType::JADE,
          {{0.135f, 0.2225f, 0.1575f},
           {0.54f, 0.89f, 0.63f},
           {0.316228f, 0.316228f, 0.316228f},
           0.1f * 128.0f}},
         {MaterialType::OBSIDIAN,
          {{0.05375f, 0.05f, 0.06625f},
           {0.18275f, 0.17f, 0.22525f},
           {0.332741f, 0.328634f, 0.346435f},
           0.3f * 128.0f}},
         {MaterialType::PEARL,
          {{0.25f, 0.20725f, 0.20725f},
           {1.0f, 0.829f, 0.829f},
           {0.296648f, 0.296648f, 0.296648f},
           0.088f * 128.0f}},
         {MaterialType::RUBY,
          {{0.1745f, 0.01175f, 0.01175f},
           {0.61424f, 0.04136f, 0.04136f},
           {0.727811f, 0.626959f, 0.626959f},
           0.6f * 128.0f}},
         {MaterialType::TURQUOISE,
          {{0.1f, 0.18725f, 0.1745f},
           {0.396f, 0.74151f, 0.69102f},
           {0.297254f, 0.30829f, 0.306678f},
           0.1f * 128.0f}},

         // Metals
         {MaterialType::BRASS,
          {{0.329412f, 0.223529f, 0.027451f},
           {0.780392f, 0.568627f, 0.113725f},
           {0.992157f, 0.941176f, 0.807843f},
           0.21794872f * 128.0f}},
         {MaterialType::BRONZE,
          {{0.2125f, 0.1275f, 0.054f},
           {0.714f, 0.4284f, 0.18144f},
           {0.393548f, 0.271906f, 0.166721f},
           0.2f * 128.0f}},
         {MaterialType::CHROME,
          {{0.25f, 0.25f, 0.25f},
           {0.4f, 0.4f, 0.4f},
           {0.774597f, 0.774597f, 0.774597f},
           0.6f * 128.0f}},
         {MaterialType::COPPER,
          {{0.19125f, 0.0735f, 0.0225f},
           {0.7038f, 0.27048f, 0.0828f},
           {0.256777f, 0.137622f, 0.086014f},
           0.1f * 128.0f}},
         {MaterialType::GOLD,
          {{0.24725f, 0.1995f, 0.0745f},
           {0.75164f, 0.60648f, 0.22648f},
           {0.628281f, 0.555802f, 0.366065f},
           0.4f * 128.0f}},
         {MaterialType::SILVER,
          {{0.19225f, 0.19225f, 0.19225f},
           {0.50754f, 0.50754f, 0.50754f},
           {0.508273f, 0.508273f, 0.508273f},
           0.4f * 128.0f}},

         // Plastics
         {MaterialType::BLACK_PLASTIC,
          {{0.0f, 0.0f, 0.0f},
           {0.01f, 0.01f, 0.01f},
           {0.50f, 0.50f, 0.50f},
           0.25f * 128.0f}},
         {MaterialType::CYAN_PLASTIC,
          {{0.0f, 0.1f, 0.06f},
           {0.0f, 0.50980392f, 0.50980392f},
           {0.50196078f, 0.50196078f, 0.50196078f},
           0.25f * 128.0f}},
         {MaterialType::GREEN_PLASTIC,
          {{0.0f, 0.0f, 0.0f},
           {0.1f, 0.35f, 0.1f},
           {0.45f, 0.55f, 0.45f},
           0.25f * 128.0f}},
         {MaterialType::RED_PLASTIC,
          {{0.0f, 0.0f, 0.0f},
           {0.5f, 0.0f, 0.0f},
           {0.7f, 0.6f, 0.6f},
           0.25f * 128.0f}},
         {MaterialType::WHITE_PLASTIC,
          {{0.0f, 0.0f, 0.0f},
           {0.55f, 0.55f, 0.55f},
           {0.70f, 0.70f, 0.70f},
           0.25f * 128.0f}},
         {MaterialType::YELLOW_PLASTIC,
          {{0.0f, 0.0f, 0.0f},
           {0.5f, 0.5f, 0.0f},
           {0.60f, 0.60f, 0.50f},
           0.25f * 128.0f}},

         // Rubber
         {MaterialType::BLACK_RUBBER,
          {{0.02f, 0.02f, 0.02f},
           {0.01f, 0.01f, 0.01f},
           {0.4f, 0.4f, 0.4f},
           0.078125f * 128.0f}},
         {MaterialType::CYAN_RUBBER,
          {{0.0f, 0.05f, 0.05f},
           {0.4f, 0.5f, 0.5f},
           {0.04f, 0.7f, 0.7f},
           0.078125f * 128.0f}},
         {MaterialType::GREEN_RUBBER,
          {{0.0f, 0.05f, 0.0f},
           {0.4f, 0.5f, 0.4f},
           {0.04f, 0.7f, 0.04f},
           0.078125f * 128.0f}},
         {MaterialType::RED_RUBBER,
          {{0.05f, 0.0f, 0.0f},
           {0.5f, 0.4f, 0.4f},
           {0.7f, 0.04f, 0.04f},
           0.078125f * 128.0f}},
         {MaterialType::WHITE_RUBBER,
          {{0.05f, 0.05f, 0.05f},
           {0.5f, 0.5f, 0.5f},
           {0.7f, 0.7f, 0.7f},
           0.078125f * 128.0f}},
         {MaterialType::YELLOW_RUBBER,
          {{0.05f, 0.05f, 0.0f},
           {0.5f, 0.5f, 0.4f},
           {0.7f, 0.7f, 0.04f},
           0.078125f * 128.0f}}};

    return presets.at(type);
  }

  static MaterialComponent create(uint32_t shaderProgram, MaterialType type) {
    auto props = getProperties(type);
    MaterialComponent material;
    material.shaderProgram = shaderProgram;
    material.ambient = props.ambient;
    material.diffuse = props.diffuse;
    material.specular = props.specular;
    material.shininess = props.shininess;
    material.useTextures = false;
    material.receivesLighting = true;
    return material;
  }

  static void applyTo(MaterialComponent &material, MaterialType type) {
    auto props = getProperties(type);
    material.ambient = props.ambient;
    material.diffuse = props.diffuse;
    material.specular = props.specular;
    material.shininess = props.shininess;
  }
};

#endif
