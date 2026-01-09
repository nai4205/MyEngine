#pragma once

#include <glm/glm.hpp>

struct BoneInfo {
    int id;                 // Bone index in final transform array
    glm::mat4 offset;       // Transform from mesh space to bone space
};
