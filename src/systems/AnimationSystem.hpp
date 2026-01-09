#pragma once

#include "../components/AnimationComponent.hpp"
#include "../ecs/System.hpp"
#include "../ecs/World.hpp"
#include <iostream>

extern World gWorld;

class AnimationSystem : public System {
  public:
    void update(float &deltaTime) override {
        gWorld.forEachWith<AnimationComponent>(
            [&](Entity entity, AnimationComponent &animComp) {
                if (animComp.isPlaying && animComp.animator) {
                    animComp.animator->updateAnimation(deltaTime);

                    // Stop if not looping and animation finished
                    if (!animComp.looping) {
                        auto it = animComp.animations.find(animComp.currentAnimation);
                        if (it != animComp.animations.end()) {
                            Animation *currentAnim = it->second.get();
                            if (animComp.animator->getCurrentTime() >=
                                currentAnim->getDuration()) {
                                animComp.isPlaying = false;
                            }
                        }
                    }
                }
            });
    }
};
