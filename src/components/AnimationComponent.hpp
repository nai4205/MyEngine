#pragma once

#include "../animation/Animation.hpp"
#include "../animation/Animator.hpp"
#include "../animation/BoneInfo.hpp"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

struct AnimationComponent {
    std::shared_ptr<Animator> animator;
    std::unordered_map<std::string, std::shared_ptr<Animation>> animations;
    std::string currentAnimation;
    bool isPlaying = false;
    bool looping = true;

    // Store bone info map from model loading
    std::map<std::string, BoneInfo> boneInfoMap;
    int boneCounter = 0;

    AnimationComponent() = default;

    void addAnimation(const std::string &name,
                      std::shared_ptr<Animation> animation) {
        animations[name] = animation;
    }

    void play(const std::string &name) {
        auto it = animations.find(name);
        if (it != animations.end()) {
            currentAnimation = name;
            if (!animator) {
                animator = std::make_shared<Animator>(it->second.get());
            } else {
                animator->playAnimation(it->second.get());
            }
            isPlaying = true;
        }
    }

    void stop() { isPlaying = false; }
};
