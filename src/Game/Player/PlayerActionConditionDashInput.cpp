#include "Player/PlayerActionConditionDashInput.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/Normal/PlayerConstParam.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds while the dash input is recent enough.
 * @param pInput the player's input
 * @param pProperty the player's physical state
 * @param pConstParam the player's tuning values
 */
PlayerActionConditionDashInput::PlayerActionConditionDashInput(const IUsePlayerInput* pInput,
                                                               const PlayerProperty* pProperty,
                                                               const PlayerConstParam* pConstParam)
    : mInput(pInput), mProperty(pProperty), mConstParam(pConstParam) {}

/**
 * Counts the frames since the dash button was last held with the stick within 45 degrees of the front.
 * @return whether that was at most DashInputSuccessFrame frames ago
 */
bool PlayerActionConditionDashInput::check() {
    sead::Vector3f moveDir = mInput->getMoveVec();
    al::normalizeOrZero(&moveDir);

    if (mInput->isStickOn() && mInput->isDashButtonOn() && moveDir.dot(mProperty->mFront) > 0.70710678f) {
        mFrame = 0;
    } else {
        mFrame++;
    }

    return mFrame <= static_cast<u32>(mConstParam->getDashInputSuccessFrame());
}

/**
 * Starts out as if the last dash input was too long ago.
 */
void PlayerActionConditionDashInput::setup() {
    mFrame = mConstParam->getDashInputSuccessFrame() + 1;
}
