#include "Animation.hpp"
#include <cassert>
#include <iostream>

// Helper function to convert Assimp matrix to GLM
static glm::mat4 convertMatrixToGLMFormat(const aiMatrix4x4 &from) {
    glm::mat4 to;
    to[0][0] = from.a1;
    to[1][0] = from.a2;
    to[2][0] = from.a3;
    to[3][0] = from.a4;
    to[0][1] = from.b1;
    to[1][1] = from.b2;
    to[2][1] = from.b3;
    to[3][1] = from.b4;
    to[0][2] = from.c1;
    to[1][2] = from.c2;
    to[2][2] = from.c3;
    to[3][2] = from.c4;
    to[0][3] = from.d1;
    to[1][3] = from.d2;
    to[2][3] = from.d3;
    to[3][3] = from.d4;
    return to;
}

Animation::Animation(const std::string &animationPath, const aiScene *scene,
                     int animationIndex) {
    assert(scene && scene->mRootNode);
    assert(animationIndex < scene->mNumAnimations);

    auto animation = scene->mAnimations[animationIndex];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;

    readHierarchyData(m_RootNode, scene->mRootNode);

    // Initialize bone info map and counter
    int boneCounter = 0;
    readMissingBones(animation, m_BoneInfoMap, boneCounter);
}

Animation::Animation(const std::string &animationPath, const aiScene *scene,
                     int animationIndex,
                     const std::map<std::string, BoneInfo> &boneInfoMap,
                     int boneCounter) {
    assert(scene && scene->mRootNode);
    assert(animationIndex < scene->mNumAnimations);

    auto animation = scene->mAnimations[animationIndex];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;

    readHierarchyData(m_RootNode, scene->mRootNode);

    // Use the provided bone info map instead of creating a new one
    m_BoneInfoMap = boneInfoMap;
    readMissingBones(animation, m_BoneInfoMap, boneCounter);
}

Bone *Animation::findBone(const std::string &name) {
    auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
                             [&](const Bone &bone) {
                                 return bone.getName() == name;
                             });
    if (iter == m_Bones.end())
        return nullptr;
    else
        return &(*iter);
}

void Animation::readMissingBones(const aiAnimation *animation,
                                 std::map<std::string, BoneInfo> &boneInfoMap,
                                 int &boneCounter) {
    int size = animation->mNumChannels;

    // Read channels (bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            boneInfoMap[boneName].id = boneCounter;
            boneInfoMap[boneName].offset = glm::mat4(1.0f);  // Identity offset for animation-only bones
            boneCounter++;
        }
        m_Bones.push_back(Bone(channel->mNodeName.data,
                              boneInfoMap[channel->mNodeName.data].id, channel));
    }

    m_BoneInfoMap = boneInfoMap;
}

void Animation::readHierarchyData(AssimpNodeData &dest, const aiNode *src) {
    assert(src);

    dest.name = src->mName.data;
    dest.transformation = convertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (unsigned int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData;
        readHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}
