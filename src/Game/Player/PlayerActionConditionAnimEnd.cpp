#include "Player/PlayerActionConditionAnimEnd.hpp"
#include "Player/IUsePlayerAnimator.hpp"

/**
 * Holds once an animation ends.
 * @param pAnimator the player's animator
 * @param pAnimName animation to watch, or nullptr for whatever plays
 * @param endFrame frame that already counts as the end, or negative for none
 */
PlayerActionConditionAnimEnd::PlayerActionConditionAnimEnd(const IUsePlayerAnimator* pAnimator, const char* pAnimName,
                                                           s32 endFrame)
    : mAnimName(pAnimName), mAnimator(pAnimator), mEndFrame(endFrame) {}

/**
 * @return whether the animation is no longer playing, has ended, or reached the end frame
 */
bool PlayerActionConditionAnimEnd::check() {
    if (mAnimName != nullptr) {
        if (!mAnimator->isAnim(mAnimName)) {
            return true;
        }
        return isEnd();
    }
    return isEnd();
}
