#pragma once

#include "../components/PhysicsComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/Tag.hpp"
#include "../ecs/World.hpp"

extern World gWorld;

class PhysicsSystem : public System {
private:
public:
  PhysicsSystem() {}

  void update(float &deltaTime) override {
    Input &input = gWorld.getInput();

    bool hJustPressed = input.isKeyJustPressed(GLFW_KEY_H);

    gWorld.forEachWith<TransformComponent, PhysicsComponent>(
        [&](Entity entity, TransformComponent &transform,
            PhysicsComponent &physics) {
          if (hJustPressed) {
            auto *tag = gWorld.getComponent<TagComponent>(entity);
            if (!tag || !tag->has(ACTIVE)) {
              glm::vec3 direction = glm::vec3(0, 2, 0);
              physics.applyImpulse(direction);
            }
          }

          physics.update(transform.position, deltaTime);
        });
  }
};
