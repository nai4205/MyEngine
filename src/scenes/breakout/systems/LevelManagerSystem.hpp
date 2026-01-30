#pragma once

#include "../../../components/MaterialComponent.hpp"
#include "../../../components/MeshComponent.hpp"
#include "../../../components/TransformComponent.hpp"
#include "../../../ecs/System.hpp"
#include "../../../ecs/Tag.hpp"
#include "../../../ecs/World.hpp"
#include "../../../resources/ResourceManager.hpp"
#include "../components/BrickComponent.hpp"
#include "../components/Collider2D.hpp"
#include "../components/GameLevelComponent.hpp"
#include <fstream>
#include <sstream>

extern World gWorld;

class LevelManagerSystem : public System {
private:
  MeshData spriteMesh{};
  bool meshInitialized = false;

  void createSpriteMesh() {
    if (meshInitialized)
      return;

    float vertices[] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};

    std::vector<VertexAttribute> layout = {
        {0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0}};

    auto &resources = ResourceManager::instance();
    spriteMesh = resources.createMesh(vertices, sizeof(vertices), layout, 6);
    meshInitialized = true;
  }

public:
  void update(float &deltaTime) override {
    gWorld.forEachWith<GameLevelComponent>(
        [&](Entity entity, GameLevelComponent &level) {
          if (!level.loaded) {
            loadLevel(level);
          }

          TagComponent *tag = gWorld.getComponent<TagComponent>(entity);
          if (tag && tag->has(ACTIVELEVEL) && !level.initialized) {
            initLevel(level);
          }
        });
  }

private:
  void loadLevel(GameLevelComponent &level) {
    std::ifstream fstream(level.path);
    if (!fstream) {
      std::cerr << "Failed to load level: " << level.path << std::endl;
      return;
    }

    std::string line;
    while (std::getline(fstream, line)) {
      std::istringstream sstream(line);
      std::vector<unsigned int> row;
      unsigned int tileCode;
      while (sstream >> tileCode) {
        row.push_back(tileCode);
      }
      if (!row.empty()) {
        level.tileData.push_back(row);
      }
    }

    level.loaded = true;
  }

  void initLevel(GameLevelComponent &level) {
    if (level.tileData.empty()) {
      return;
    }

    createSpriteMesh();

    unsigned int height = level.tileData.size();
    unsigned int width = level.tileData[0].size();
    float unitWidth = level.levelWidth / static_cast<float>(width);
    float unitHeight = level.levelHeight / static_cast<float>(height);

    for (unsigned int y = 0; y < height; ++y) {
      for (unsigned int x = 0; x < width; ++x) {
        unsigned int tile = level.tileData[y][x];
        if (tile == 0) {
          continue;
        }

        glm::vec2 pos(unitWidth * x, unitHeight * y);
        glm::vec2 size(unitWidth, unitHeight);
        glm::vec3 color(1.0f);
        bool isSolid = false;

        if (tile == 1) {
          color = glm::vec3(0.8f, 0.8f, 0.7f);
          isSolid = true;
        } else if (tile == 2) {
          color = glm::vec3(0.2f, 0.6f, 1.0f);
        } else if (tile == 3) {
          color = glm::vec3(0.0f, 0.7f, 0.0f);
        } else if (tile == 4) {
          color = glm::vec3(0.8f, 0.8f, 0.4f);
        } else if (tile == 5) {
          color = glm::vec3(1.0f, 0.5f, 0.0f);
        }

        createBrick(pos, size, color, isSolid, level);
      }
    }

    level.initialized = true;
  }

  // Use our components to create a brick for the relevant tile number
  void createBrick(glm::vec2 pos, glm::vec2 size, glm::vec3 color, bool isSolid,
                   const GameLevelComponent &level) {
    Entity brick = gWorld.createEntity();

    TransformComponent transform;
    transform.position = glm::vec3(pos, 0.0f);
    transform.scale = glm::vec3(size, 1.0f);
    gWorld.addComponent(brick, transform);

    MaterialComponent material;
    material.shaderProgram = level.shaderID;
    uint32_t tex = isSolid ? level.blockSolidTexture : level.blockTexture;
    material.textures[0] = tex;
    material.useTextures = (tex != 0);
    material.color = color;
    gWorld.addComponent(brick, material);

    MeshComponent meshComp;
    meshComp.vao = spriteMesh.vao;
    meshComp.vertexCount = spriteMesh.vertexCount;
    meshComp.indexCount = 0;
    gWorld.addComponent(brick, meshComp);

    BrickComponent brickComp;
    brickComp.isSolid = isSolid;
    brickComp.destroyed = false;
    gWorld.addComponent(brick, brickComp);

    // AABB collider for the brick
    gWorld.addComponent(brick, Collider2D::makeAABB(size));
  }
};
