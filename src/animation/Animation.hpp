#pragma once

#include "Bone.hpp"
#include "BoneInfo.hpp"
#include <assimp/scene.h>
#include <map>
#include <string>
#include <vector>

struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation {
  public:
    Animation(const std::string &animationPath, const aiScene *scene,
              int animationIndex = 0);

    Animation(const std::string &animationPath, const aiScene *scene,
              int animationIndex,
              const std::map<std::string, BoneInfo> &boneInfoMap,
              int boneCounter);

    Bone *findBone(const std::string &name);
    float getTicksPerSecond() const { return m_TicksPerSecond; }
    float getDuration() const { return m_Duration; }
    const AssimpNodeData &getRootNode() const { return m_RootNode; }
    const std::map<std::string, BoneInfo> &getBoneInfoMap() const {
        return m_BoneInfoMap;
    }

  private:
    void readMissingBones(const aiAnimation *animation,
                          std::map<std::string, BoneInfo> &boneInfoMap,
                          int &boneCounter);
    void readHierarchyData(AssimpNodeData &dest, const aiNode *src);

    float m_Duration;
    float m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;
};
