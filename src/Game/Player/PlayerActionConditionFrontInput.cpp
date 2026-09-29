#include "Player/PlayerActionConditionFrontInput.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds while the stick points roughly where the player faces.
 * @param pInput the player's input
 * @param pProperty the player's physical state
 * @param isWide accept 50 degrees off instead of 45
 */
PlayerActionConditionFrontInput::PlayerActionConditionFrontInput(const IUsePlayerInput* pInput,
                                                                 const PlayerProperty* pProperty, bool isWide)
    : mInput(pInput), mProperty(pProperty), mIsWide(isWide) {}

/**
 * @return whether the stick is tilted within the angle of the player's front
 */
bool PlayerActionConditionFrontInput::check() {
    if (!mInput->isStickOn()) {
        return false;
    }
    sead::Vector3f moveDir = mInput->getMoveVec();
    al::normalize(&moveDir);
    f32 dot = moveDir.dot(mProperty->mFront);
    f32 minDot = mIsWide ? 0.64278763f : 0.70710678f;
    return dot > minDot;
}
