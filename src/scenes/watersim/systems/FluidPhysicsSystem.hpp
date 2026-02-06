#pragma once

#include "../../../components/MaterialComponent.hpp"
#include "../../../components/MeshComponent.hpp"
#include "../../../components/TransformComponent.hpp"
#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../../../resources/ResourceManager.hpp"
#include "../../breakout/components/Collider2D.hpp"
#include "../components/FluidPhysicsComponent.hpp"
#include "../components/ParticleComponent.hpp"
#include "../components/SimulatorSettings.hpp"
#include "../components/VelocityComponent.hpp"
#include <cmath>

extern World gWorld;

class FluidPhysicsSystem : public System {
public:
  FluidPhysicsSystem(float width, float height, uint32_t shaderID,
                     const SimulatorSettings &settings)
      : screenWidth(width), screenHeight(height), spriteShaderID(shaderID),
        settings(settings) {}

  void createParticles() {
    float radius = settings.particleRadius;
    float spacing = radius * 2.0f + settings.particleSpacing;

    float left = settings.boxMargin + radius;
    float right = screenWidth - settings.boxMargin - radius;
    float bottom = settings.boxMargin + radius;
    float top = screenHeight - settings.boxMargin - radius;

    float centerX = (left + right) / 2.0f;
    float centerY = (bottom + top) / 2.0f;

    int numParticles = settings.numParticles;
    int particlesPerRow = static_cast<int>(std::sqrt(numParticles));

    for (int i = 0; i < numParticles; i++) {
      int col = i % particlesPerRow;
      int row = i / particlesPerRow;

      float x = centerX + (col - particlesPerRow / 2.0f + 0.5f) * spacing;
      float y = centerY + (row - particlesPerRow / 2.0f + 0.5f) * spacing;

      x = glm::clamp(x, left, right);
      y = glm::clamp(y, bottom, top);

      // Position is bottom-left corner for collision system
      createParticle(glm::vec2(x - radius, y - radius));
    }
  }

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

private:
  float screenWidth;
  float screenHeight;
  uint32_t spriteShaderID;
  SimulatorSettings settings;
  MeshData quadMesh{};
  bool meshCreated = false;

  void ensureMeshCreated() {
    if (meshCreated)
      return;

    auto &resources = ResourceManager::instance();
    float vertices[] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};
    std::vector<VertexAttribute> layout = {
        {0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0}};
    quadMesh = resources.createMesh(vertices, sizeof(vertices), layout, 6);
    meshCreated = true;
  }

  void createParticle(glm::vec2 pos) {
    ensureMeshCreated();

    Entity particle = gWorld.createEntity();

    gWorld.addComponent(particle, ParticleComponent{});

    TransformComponent transform;
    transform.position = glm::vec3(pos, 0.0f);
    float diameter = settings.particleRadius * 2.0f;
    transform.scale = glm::vec3(diameter, diameter, 1.0f);
    gWorld.addComponent(particle, transform);

    MeshComponent mesh;
    mesh.vao = quadMesh.vao;
    mesh.vertexCount = quadMesh.vertexCount;
    mesh.indexCount = quadMesh.indexCount;
    gWorld.addComponent(particle, mesh);

    MaterialComponent material;
    material.shaderProgram = spriteShaderID;
    material.useTextures = false;
    material.color = glm::vec3(1.0f, 0.0f, 0.0f);
    material.isCircle = true;
    gWorld.addComponent(particle, material);

    FluidPhysicsComponent physics;
    physics.gravity = settings.gravity;
    gWorld.addComponent(particle, physics);

    WaterVelocityComponent vel;
    vel.velocity = glm::vec2(0.0f, 0.0f);
    gWorld.addComponent(particle, vel);

    Collider2D collider =
        Collider2D::makeCircle(settings.particleRadius, CollisionLayer::Ball,
                               static_cast<uint32_t>(CollisionLayer::Wall));
    gWorld.addComponent(particle, collider);
  }
};
