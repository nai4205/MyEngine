#include "Animator.hpp"
#include <cmath>
#include <iostream>

Animator::Animator(Animation *animation)
    : m_CurrentTime(0.0f), m_CurrentAnimation(animation) {
    m_FinalBoneMatrices.reserve(MAX_BONES);

    for (int i = 0; i < MAX_BONES; i++)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::updateAnimation(float deltaTime) {
    if (m_CurrentAnimation) {
        m_CurrentTime += m_CurrentAnimation->getTicksPerSecond() * deltaTime;
        m_CurrentTime =
            fmod(m_CurrentTime, m_CurrentAnimation->getDuration());
        calculateBoneTransform(&m_CurrentAnimation->getRootNode(),
                              glm::mat4(1.0f));
    }
}

void Animator::playAnimation(Animation *animation) {
    m_CurrentAnimation = animation;
    m_CurrentTime = 0.0f;
}

void Animator::calculateBoneTransform(const AssimpNodeData *node,
                                      const glm::mat4 &parentTransform) {
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone *bone = m_CurrentAnimation->findBone(nodeName);

    if (bone) {
        bone->update(m_CurrentTime);
        nodeTransform = bone->getLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = m_CurrentAnimation->getBoneInfoMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        calculateBoneTransform(&node->children[i], globalTransformation);
}
