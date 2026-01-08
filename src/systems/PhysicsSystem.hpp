#ifndef PHYSICS_SYSTEM_HPP
#define PHYSICS_SYSTEM_HPP

#include "../gl_common.hpp"
#include "../components/PhysicsComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/Tag.hpp"
#include "../ecs/World.hpp"
#include <random>

extern World gWorld;

// System that updates physics for all entities with Transform and Physics
// components
class PhysicsSystem : public System {
private:
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_real_distribution<float> dist;

public:
  PhysicsSystem() : gen(rd()), dist(-1.0f, 1.0f) {}

  void update(float &deltaTime) override {
    Input &input = gWorld.getInput();

    bool hJustPressed = input.isKeyJustPressed(GLFW_KEY_H);

    glm::vec3 modelImpulse(0.0f);
    if (hJustPressed) {
      modelImpulse = glm::vec3(dist(gen), dist(gen), dist(gen));
      if (glm::length(modelImpulse) > 0.0f) {
        modelImpulse = glm::normalize(modelImpulse) * 8.0f;
      }
    }

    gWorld.forEachWith<TransformComponent, PhysicsComponent>(
        [&](Entity entity, TransformComponent &transform,
            PhysicsComponent &physics) {
          if (hJustPressed) {
            auto *tag = gWorld.getComponent<TagComponent>(entity);
            if (!tag) {
              glm::vec3 randomDirection(dist(gen), dist(gen), dist(gen));
              if (glm::length(randomDirection) > 0.0f) {
                randomDirection = glm::normalize(randomDirection) * 8.0f;
              }
              physics.applyImpulse(randomDirection);
            } else if (tag->type == MODEL) {
              physics.applyImpulse(modelImpulse);
            }
          }

          physics.update(transform.position, deltaTime);
        });
  }
};

#endif // PHYSICS_SYSTEM_HPP
