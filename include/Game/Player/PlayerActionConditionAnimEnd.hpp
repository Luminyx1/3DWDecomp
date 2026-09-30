#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerActionCondition.hpp"

#include "Player/IUsePlayerAnimator.hpp"

/// Holds once an animation ends (or reaches a frame), or once another animation plays.
class PlayerActionConditionAnimEnd : public PlayerActionCondition {
public:
    PlayerActionConditionAnimEnd(const IUsePlayerAnimator*, const char*, s32);

    bool check() override;

private:
    bool isEnd() const {
        if (mAnimator->isAnimEnd()) {
            return true;
        }

        if (mEndFrame >= 0 && mAnimator->getAnimFrame() >= mEndFrame) {
            return true;
        }

        return false;
    }

    const char* mAnimName;                // 0x8
    const IUsePlayerAnimator* mAnimator;  // 0x10
    s32 mEndFrame;                        // 0x18
};
