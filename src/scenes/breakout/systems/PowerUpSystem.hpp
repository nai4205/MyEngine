#pragma once

#include "../../../components/MaterialComponent.hpp"
#include "../../../components/MeshComponent.hpp"
#include "../../../components/TransformComponent.hpp"
#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../../../resources/ResourceManager.hpp"
#include "../components/BallComponent.hpp"
#include "../components/Collider2D.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/PostProcessingComponent.hpp"
#include "../components/PowerUpComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include <cstdlib>

extern World gWorld;

struct PowerUpTextures {
  uint32_t speed = 0;
  uint32_t sticky = 0;
  uint32_t passThrough = 0;
  uint32_t padSize = 0;
  uint32_t confuse = 0;
  uint32_t chaos = 0;
};

class PowerUpSystem : public System {
public:
  PowerUpSystem(float screenWidth, float screenHeight, uint32_t shaderID,
                const PowerUpTextures &textures, unsigned int poolSize = 10)
      : screenWidth(screenWidth), screenHeight(screenHeight),
        shaderID(shaderID), textures(textures) {
    initMesh();
    initPool(poolSize);
  }

  void update(float &deltaTime) override {
    for (Entity entity : pool) {
      auto *powerup = gWorld.getComponent<PowerUpComponent>(entity);
      auto *transform = gWorld.getComponent<TransformComponent>(entity);
      auto *material = gWorld.getComponent<MaterialComponent>(entity);

      if (!powerup || powerup->destroyed)
        continue;

      transform->position.y += FALL_SPEED * deltaTime;

      // Off screen - deactivate
      if (transform->position.y > screenHeight) {
        powerup->destroyed = true;
        material->alpha = 0.0f;
      }
    }

    updateActivePowerUps(deltaTime);
  }

  void trySpawnPowerUp(const glm::vec2 &position) {
    if (!shouldSpawn(3))
      return;

    // Find an inactive powerup in the pool
    for (Entity entity : pool) {
      auto *powerup = gWorld.getComponent<PowerUpComponent>(entity);
      if (powerup && powerup->destroyed) {
        spawnFromPool(entity, position);
        return;
      }
    }
  }

  void activatePowerUp(Entity powerupEntity) {
    auto *powerup = gWorld.getComponent<PowerUpComponent>(powerupEntity);
    auto *material = gWorld.getComponent<MaterialComponent>(powerupEntity);

    if (!powerup || powerup->activated || powerup->destroyed)
      return;

    powerup->activated = true;
    powerup->destroyed = true;
    material->alpha = 0.0f; // Hide

    switch (powerup->type) {
    case PowerUpType::Speed:
      gWorld.forEachWith<VelocityComponent, BallComponent>(
          [](Entity e, VelocityComponent &vel, BallComponent &) {
            vel.velocity *= 1.2f;
          });
      break;

    case PowerUpType::Sticky:
      activePowerUps.push_back({PowerUpType::Sticky, powerup->duration});
      break;

    case PowerUpType::PassThrough:
      activePowerUps.push_back({PowerUpType::PassThrough, powerup->duration});
      break;

    case PowerUpType::PadSizeInc:
      gWorld.forEachWith<PlayerComponent, TransformComponent, Collider2D>(
          [](Entity e, PlayerComponent &player, TransformComponent &transform,
             Collider2D &collider) {
            player.sizeX += 50.0f;
            transform.scale.x += 50.0f;
            collider.halfExtents.x += 25.0f; // Half of size increase
          });
      break;

    case PowerUpType::Confuse:
      gWorld.forEachWith<PostProcessingComponent>(
          [](Entity e, PostProcessingComponent &fx) { fx.confuse = true; });
      activePowerUps.push_back({PowerUpType::Confuse, powerup->duration});
      break;

    case PowerUpType::Chaos:
      gWorld.forEachWith<PostProcessingComponent>(
          [](Entity e, PostProcessingComponent &fx) { fx.chaos = true; });
      activePowerUps.push_back({PowerUpType::Chaos, powerup->duration});
      break;
    }
  }

  bool isPowerUpActive(PowerUpType type) {
    for (const auto &ap : activePowerUps) {
      if (ap.type == type)
        return true;
    }
    return false;
  }

private:
  static constexpr float FALL_SPEED = 150.0f;
  static constexpr glm::vec2 SIZE = glm::vec2(60.0f, 20.0f);

  float screenWidth, screenHeight;
  uint32_t shaderID;
  uint32_t meshVAO = 0;
  unsigned int meshVertexCount = 6;
  PowerUpTextures textures;
  std::vector<Entity> pool;

  struct ActivePowerUp {
    PowerUpType type;
    float timeRemaining;
  };
  std::vector<ActivePowerUp> activePowerUps;

  bool shouldSpawn(unsigned int chance) { return (rand() % chance) == 0; }

  void initMesh() {
    float vertices[] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};

    std::vector<VertexAttribute> layout = {
        {0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0}};

    auto &resources = ResourceManager::instance();
    MeshData mesh = resources.createMesh(vertices, sizeof(vertices), layout, 6);
    meshVAO = mesh.vao;
    meshVertexCount = mesh.vertexCount;
  }

  void initPool(unsigned int poolSize) {
    for (unsigned int i = 0; i < poolSize; ++i) {
      Entity entity = gWorld.createEntity();

      PowerUpComponent powerup;
      powerup.destroyed = true; // Start inactive
      gWorld.addComponent(entity, powerup);

      TransformComponent transform;
      transform.position = glm::vec3(-100.0f, -100.0f, 0.0f); // Off screen
      transform.scale = glm::vec3(SIZE, 1.0f);
      gWorld.addComponent(entity, transform);

      MaterialComponent material;
      material.shaderProgram = shaderID;
      material.alpha = 0.0f; // Invisible
      material.color = glm::vec3(1.0f);
      gWorld.addComponent(entity, material);

      MeshComponent mesh;
      mesh.vao = meshVAO;
      mesh.vertexCount = meshVertexCount;
      mesh.indexCount = 0;
      gWorld.addComponent(entity, mesh);

      gWorld.addComponent(
          entity,
          Collider2D::makeAABB(SIZE, CollisionLayer::PowerUp,
                               static_cast<uint32_t>(CollisionLayer::Player)));

      pool.push_back(entity);
    }
  }

  void spawnFromPool(Entity entity, const glm::vec2 &position) {
    auto *powerup = gWorld.getComponent<PowerUpComponent>(entity);
    auto *transform = gWorld.getComponent<TransformComponent>(entity);
    auto *material = gWorld.getComponent<MaterialComponent>(entity);

    int type = rand() % 6;
    PowerUpType powerUpType = static_cast<PowerUpType>(type);

    glm::vec3 color;
    float duration;
    uint32_t textureID;

    switch (powerUpType) {
    case PowerUpType::Speed:
      color = glm::vec3(0.5f, 0.5f, 1.0f);
      duration = 0.0f;
      textureID = textures.speed;
      break;
    case PowerUpType::Sticky:
      color = glm::vec3(1.0f, 0.5f, 1.0f);
      duration = 20.0f;
      textureID = textures.sticky;
      break;
    case PowerUpType::PassThrough:
      color = glm::vec3(0.5f, 1.0f, 0.5f);
      duration = 10.0f;
      textureID = textures.passThrough;
      break;
    case PowerUpType::PadSizeInc:
      color = glm::vec3(1.0f, 0.6f, 0.4f);
      duration = 0.0f;
      textureID = textures.padSize;
      break;
    case PowerUpType::Confuse:
      color = glm::vec3(1.0f, 0.3f, 0.3f);
      duration = 15.0f;
      textureID = textures.confuse;
      break;
    case PowerUpType::Chaos:
    default:
      color = glm::vec3(0.9f, 0.25f, 0.25f);
      duration = 15.0f;
      textureID = textures.chaos;
      break;
    }

    powerup->type = powerUpType;
    powerup->color = color;
    powerup->duration = duration;
    powerup->activated = false;
    powerup->destroyed = false;

    transform->position = glm::vec3(position, 0.0f);

    material->textures[0] = textureID;
    material->useTextures = (textureID != 0);
    material->color = color;
    material->alpha = 1.0f; // Visible
  }

  void updateActivePowerUps(float deltaTime) {
    for (auto it = activePowerUps.begin(); it != activePowerUps.end();) {
      it->timeRemaining -= deltaTime;

      if (it->timeRemaining <= 0.0f) {
        switch (it->type) {
        case PowerUpType::Confuse:
          gWorld.forEachWith<PostProcessingComponent>(
              [](Entity e, PostProcessingComponent &fx) {
                fx.confuse = false;
              });
          break;
        case PowerUpType::Chaos:
          gWorld.forEachWith<PostProcessingComponent>(
              [](Entity e, PostProcessingComponent &fx) { fx.chaos = false; });
          break;
        default:
          break;
        }
        it = activePowerUps.erase(it);
      } else {
        ++it;
      }
    }
  }
};
