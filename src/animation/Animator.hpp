#pragma once

#include "Animation.hpp"
#include <glm/glm.hpp>
#include <vector>

#define MAX_BONES 150

class Animator {
  public:
    Animator(Animation *animation);

    void updateAnimation(float deltaTime);
    void playAnimation(Animation *animation);
    void calculateBoneTransform(const AssimpNodeData *node,
                                const glm::mat4 &parentTransform);

    const std::vector<glm::mat4> &getFinalBoneMatrices() const {
        return m_FinalBoneMatrices;
    }
    bool isPlaying() const { return m_CurrentAnimation != nullptr; }
    float getCurrentTime() const { return m_CurrentTime; }

  private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation *m_CurrentAnimation;
    float m_CurrentTime;
};
