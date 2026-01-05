#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

#include "components/Camera.hpp"
#include "components/CameraController.hpp"
#include "components/MeshRenderer.hpp"
#include "components/MeshRendererIndexed.hpp"
#include "components/Physics.hpp"
#include "components/Transform.hpp"
#include <memory>
#include <string>
#include <variant>

class GameObject {
public:
  std::string name;
  Transform transform;

  GameObject(const std::string &objectName = "GameObject")
      : name(objectName), transform(), renderer(std::monostate{}),
        camera(nullptr), cameraController(nullptr), physics(nullptr) {}

  // ========== MESH RENDERER ==========
  void addMeshRenderer(std::shared_ptr<Mesh> mesh,
                       std::shared_ptr<Material> material,
                       unsigned int vertexCount = 36) {
    renderer = std::make_shared<MeshRenderer>(mesh, material, vertexCount);
  }

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

  // ========== CAMERA AND PHYSICS ==========
  void addCamera(float yaw = -90.0f, float pitch = 0.0f, float zoom = 45.0f) {
    camera = std::make_shared<Camera>(yaw, pitch, zoom);
  }

  void addCameraController(float speed = 2.5f, float sensitivity = 0.1f,
                           float jumpForce = 5.0f) {
    cameraController =
        std::make_shared<CameraController>(speed, sensitivity, jumpForce);
  }

  void addPhysics(float gravity = -9.81f, float groundLevel = 0.0f) {
    physics = std::make_shared<Physics>(gravity, groundLevel);
    // Check if object is above ground - if so, make it fall
    if (transform.position.y > groundLevel) {
      physics->isGrounded = false;
    }
  }

  void updatePhysics(float deltaTime) {
    if (physics) {
      physics->update(transform.position, deltaTime);
    }
  }

  bool hasPhysics() const { return physics != nullptr; }
  Physics *getPhysics() const { return physics.get(); }

  bool hasCamera() const { return camera != nullptr; }
  Camera *getCamera() const { return camera.get(); }

  bool hasCameraController() const { return cameraController != nullptr; }
  CameraController *getCameraController() const {
    return cameraController.get();
  }

  void render() const {
    std::visit(
        [this](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::shared_ptr<MeshRenderer>>) {
            arg->getMaterial()->setMat4("model", transform.getModelMatrix());
            arg->render();
          } else if constexpr (std::is_same_v<
                                   T, std::shared_ptr<MeshRendererIndexed>>) {
            arg->getMaterial()->setMat4("model", transform.getModelMatrix());
            arg->render();
          }
        },
        renderer);
  }

  // ========== CONVENIENCE METHODS ==========
  void addFPSCamera(float speed = 2.5f, float sensitivity = 0.1f,
                    float jumpForce = 5.0f, float yaw = -90.0f,
                    float pitch = 0.0f, float zoom = 45.0f) {
    addCamera(yaw, pitch, zoom);
    cameraController = std::make_shared<CameraController>(speed, sensitivity,
                                                          jumpForce, false);
    addPhysics(-9.81f, 0.0f);
  }

  void addFreeCamera(float speed = 5.0f, float sensitivity = 0.1f,
                     float yaw = -90.0f, float pitch = 0.0f,
                     float zoom = 45.0f) {
    addCamera(yaw, pitch, zoom);
    cameraController =
        std::make_shared<CameraController>(speed, sensitivity, 0.0f, true);
  }

private:
  std::variant<std::monostate, std::shared_ptr<MeshRenderer>,
               std::shared_ptr<MeshRendererIndexed>>
      renderer;
  std::shared_ptr<Camera> camera;
  std::shared_ptr<CameraController> cameraController;
  std::shared_ptr<Physics> physics;
};

#endif
