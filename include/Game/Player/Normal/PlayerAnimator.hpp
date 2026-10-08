#pragma once

#include <basis/seadTypes.h>

#include "Player/IUsePlayerAnimator.hpp"

/// Plays the player's skeletal, sub and material animations.
class PlayerAnimator : public IUsePlayerAnimator {
public:
    /** @brief Tests whether the cat's ClimbMove animation is played as a normal walk. @return True if so. */
    bool isClimbMoveAsWalk() const { return mIsClimbMoveAsWalk; }

private:
    u8 _8[0x162 - 0x8];
    bool mIsClimbMoveAsWalk;  // 0x162
};
