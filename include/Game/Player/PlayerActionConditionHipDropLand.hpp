#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerAnimator;
class IUsePlayerCollision;

/// Holds when the ground pound's landing animation has started and the player is on the floor.
class PlayerActionConditionHipDropLand : public PlayerActionCondition {
public:
    PlayerActionConditionHipDropLand(const IUsePlayerAnimator*, const IUsePlayerCollision*);

    bool check() override;

private:
    const IUsePlayerAnimator* mAnimator;    // 0x8
    const IUsePlayerCollision* mCollision;  // 0x10
};
