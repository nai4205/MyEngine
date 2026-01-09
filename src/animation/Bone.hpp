#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/anim.h>
#include <string>
#include <vector>

struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

class Bone {
  public:
    Bone(const std::string &name, int id, const aiNodeAnim *channel);

    void update(float animationTime);
    glm::mat4 getLocalTransform() const { return m_LocalTransform; }
    const std::string &getName() const { return m_Name; }
    int getBoneID() const { return m_ID; }

  private:
    int getPositionIndex(float animationTime);
    int getRotationIndex(float animationTime);
    int getScaleIndex(float animationTime);

    float getScaleFactor(float lastTimeStamp, float nextTimeStamp,
                         float animationTime);

    glm::mat4 interpolatePosition(float animationTime);
    glm::mat4 interpolateRotation(float animationTime);
    glm::mat4 interpolateScaling(float animationTime);

    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;
};
