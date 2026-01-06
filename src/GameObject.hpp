#pragma once

#include "components/MeshRenderer.hpp"
#include "components/MeshRendererIndexed.hpp"
#include "components/TransformComponent.hpp"
#include <memory>
#include <string>
#include <variant>

// Minimal GameObject class for Model loading compatibility
// This is only used by Model.hpp to load models, which are then converted to
// entities
class GameObject {
public:
  std::string name;
  TransformComponent transform;

  GameObject(const std::string &objectName = "GameObject")
      : name(objectName), transform(), renderer(std::monostate{}) {}

  // Add MeshRendererIndexed (used by Model loader)
  template <typename... Args> void addComponent(Args &&...args) {
    renderer =
        std::make_shared<MeshRendererIndexed>(std::forward<Args>(args)...);
  }

  bool hasMeshRenderer() const {
    return std::holds_alternative<std::shared_ptr<MeshRenderer>>(renderer) ||
           std::holds_alternative<std::shared_ptr<MeshRendererIndexed>>(
               renderer);
  }

  MeshRenderer *getMeshRenderer() const {
    if (auto *ptr = std::get_if<std::shared_ptr<MeshRenderer>>(&renderer)) {
      return ptr->get();
    }
    return nullptr;
  }

  MeshRendererIndexed *getMeshRendererIndexed() const {
    if (auto *ptr =
            std::get_if<std::shared_ptr<MeshRendererIndexed>>(&renderer)) {
      return ptr->get();
    }
    return nullptr;
  }

private:
  std::variant<std::monostate, std::shared_ptr<MeshRenderer>,
               std::shared_ptr<MeshRendererIndexed>>
      renderer;
};
