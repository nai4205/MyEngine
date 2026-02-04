#pragma once

#include "../../../components/TransformComponent.hpp"
#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../components/FluidPhysicsComponent.hpp"
#include "../components/VelocityComponent.hpp"

extern World gWorld;

class FluidPhysicsSystem : public System {
public:
  void update(float &deltaTime) override {
    gWorld.forEachWith<FluidPhysicsComponent, TransformComponent,
                       WaterVelocityComponent>(
        [&](Entity entity, FluidPhysicsComponent &physics,
            TransformComponent &transform, WaterVelocityComponent &vel) {
          // Apply gravity to velocity
          vel.velocity.y -= physics.gravity * deltaTime;

          // Apply velocity to position
          transform.position.x += vel.velocity.x * deltaTime;
          transform.position.y += vel.velocity.y * deltaTime;
        });
  }
};
