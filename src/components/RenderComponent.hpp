#ifndef RENDER_COMPONENT_HPP
#define RENDER_COMPONENT_HPP

#include "../components/MeshRenderer.hpp"
#include "../components/MeshRendererIndexed.hpp"
#include <memory>
#include <variant>

// RenderComponent - wraps the variant of different renderer types
struct RenderComponent {
  std::variant<std::monostate, std::shared_ptr<MeshRenderer>,
               std::shared_ptr<MeshRendererIndexed>>
      renderer;

  RenderComponent() : renderer(std::monostate{}) {}

  bool hasRenderer() const {
    return !std::holds_alternative<std::monostate>(renderer);
  }

  MeshRenderer *getMeshRenderer() {
    if (auto *ptr = std::get_if<std::shared_ptr<MeshRenderer>>(&renderer)) {
      return ptr->get();
    }
    return nullptr;
  }

  MeshRendererIndexed *getMeshRendererIndexed() {
    if (auto *ptr =
            std::get_if<std::shared_ptr<MeshRendererIndexed>>(&renderer)) {
      return ptr->get();
    }
    return nullptr;
  }
};

#endif // RENDER_COMPONENT_HPP
