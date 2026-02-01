#pragma once
#include "../../../ecs/System.hpp"
#include "../../../ecs/World.hpp"
#include "../../../resources/ResourceManager.hpp"
#include "../../components/TransformComponent.hpp"
#include "../../gl_common.hpp"
#include "../components/ParticleEmitterComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include <cstdlib>

extern World gWorld;

class ParticleSystem : public System {
public:
  ParticleSystem(float screenWidth, float screenHeight)
      : screenWidth(screenWidth), screenHeight(screenHeight) {}

  void update(float &deltaTime) override {
    gWorld.forEachWith<ParticleEmitterComponent, TransformComponent>(
        [&](Entity entity, ParticleEmitterComponent &emitter,
            TransformComponent &transform) {
          // Resize particle pool if needed
          if (emitter.particles.size() < emitter.maxParticles) {
            emitter.particles.resize(emitter.maxParticles);
          }

          // Handle emitter duration
          if (emitter.durationRemaining > 0.0f) {
            emitter.durationRemaining -= deltaTime;
            if (emitter.durationRemaining <= 0.0f) {
              emitter.spawnRate = 0;
            }
          }

          // Get velocity from emitter entity (e.g., ball)
          VelocityComponent *vel =
              gWorld.getComponent<VelocityComponent>(entity);
          glm::vec2 emitterVelocity = vel ? vel->velocity : glm::vec2(0.0f);
          glm::vec2 emitterPos = glm::vec2(transform.position);

          // Spawn new particles
          for (unsigned int i = 0; i < emitter.spawnRate; ++i) {
            unsigned int idx = findUnusedParticle(emitter);
            respawnParticle(emitter.particles[idx], emitterPos, emitterVelocity,
                            emitter);
          }

          // Update all particles
          for (unsigned int i = 0; i < emitter.maxParticles; ++i) {
            Particle &p = emitter.particles[i];
            p.life -= deltaTime;
            if (p.life > 0.0f) {
              p.velocity += emitter.gravity * deltaTime;
              if (emitter.trailMode) {
                p.position -= p.velocity * deltaTime;
              } else {
                p.position += p.velocity * deltaTime;
              }
              p.color.a -= deltaTime / emitter.particleLifetime;
            }
          }
        });
  }

  void render() override {
    auto &resources = ResourceManager::instance();
    glm::mat4 projection =
        glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -1.0f, 1.0f);

    // Additive blending for glow effect
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    gWorld.forEachWith<ParticleEmitterComponent>(
        [&](Entity entity, ParticleEmitterComponent &emitter) {
          if (emitter.particles.empty())
            return;

          Shader *shader = resources.getShader(emitter.shaderID);
          if (!shader)
            return;

          shader->use();
          shader->setMat4("projection", projection);

          if (emitter.textureID != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, emitter.textureID);
            shader->setInt("image", 0);
          }

          for (const Particle &p : emitter.particles) {
            if (p.life > 0.0f) {
              glm::mat4 model = glm::mat4(1.0f);
              model = glm::translate(model, glm::vec3(p.position, 0.0f));
              model = glm::scale(model, glm::vec3(emitter.particleSize, 1.0f));

              shader->setMat4("model", model);
              shader->setVec3("spriteColor", glm::vec3(p.color));
              shader->setFloat("spriteAlpha", p.color.a);

              glBindVertexArray(emitter.meshVAO);
              glDrawArrays(GL_TRIANGLES, 0, emitter.meshVertexCount);
            }
          }
        });

    glBindVertexArray(0);

    // Restore default blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

private:
  float screenWidth;
  float screenHeight;

  unsigned int findUnusedParticle(ParticleEmitterComponent &emitter) {
    // Search from last used particle (usually finds one quickly)
    for (unsigned int i = emitter.lastUsedParticle; i < emitter.maxParticles;
         ++i) {
      if (emitter.particles[i].life <= 0.0f) {
        emitter.lastUsedParticle = i;
        return i;
      }
    }
    // Linear search from beginning
    for (unsigned int i = 0; i < emitter.lastUsedParticle; ++i) {
      if (emitter.particles[i].life <= 0.0f) {
        emitter.lastUsedParticle = i;
        return i;
      }
    }
    // All alive - override first
    emitter.lastUsedParticle = 0;
    return 0;
  }

  void respawnParticle(Particle &p, const glm::vec2 &pos, const glm::vec2 &vel,
                       const ParticleEmitterComponent &emitter) {
    float randomX = ((std::rand() % 100) - 50) / 10.0f;
    float randomY = ((std::rand() % 100) - 50) / 10.0f;
    float rColor = 0.5f + ((std::rand() % 100) / 100.0f);

    p.position = pos + glm::vec2(randomX, randomY) + emitter.offset;
    p.color = glm::vec4(rColor, rColor, rColor, 1.0f);
    p.life = emitter.particleLifetime;

    if (emitter.trailMode) {
      // Trail behind emitter
      p.velocity = vel * 0.1f;
    } else {
      // Scatter in random directions (explosion/debris)
      int scatterX = ((std::rand() % 100) - 50);
      int scatterY = ((std::rand() % 100) - 50);
      p.velocity = glm::vec2(scatterX, scatterY);
    }
  }
};
