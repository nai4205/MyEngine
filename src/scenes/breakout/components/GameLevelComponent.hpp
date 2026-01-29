#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct GameLevelComponent {
  std::string path;
  unsigned int levelWidth;
  unsigned int levelHeight;

  uint32_t shaderID = 0;
  uint32_t blockTexture = 0;
  uint32_t blockSolidTexture = 0;

  std::vector<std::vector<unsigned int>> tileData;
  bool loaded = false;
  bool initialized = false;
};
