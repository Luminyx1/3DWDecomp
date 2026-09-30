#include "Player/PlayerActionConditionSwimJump.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Player/IUsePlayerWaterSurfaceInfo.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when the player may jump out of the water.
 * @param pWaterSurfaceInfo water surface finder
 * @param pProperty the player's physical state
 * @param isAcceptStill also allow it while barely moving up or down
 */
PlayerActionConditionSwimJump::PlayerActionConditionSwimJump(const IUsePlayerWaterSurfaceInfo* pWaterSurfaceInfo,
                                                             const PlayerProperty* pProperty, bool isAcceptStill)
    : mWaterSurfaceInfo(pWaterSurfaceInfo), mProperty(pProperty), mIsAcceptStill(isAcceptStill) {}

/**
 * Remembers once the player has moved upwards.
 * @return whether the player has moved upwards and the surface is at most 200 units above
 */
bool PlayerActionConditionSwimJump::check() {
    f32 upSpeed = mProperty->mVelocity.dot(mProperty->mUpDir);
    if (upSpeed >= 0.0f) {
        mIsRising = true;
    }

    if (mIsAcceptStill && !mIsRising) {
        mIsRising = al::isNearZero(upSpeed, 0.04f);
    }

    if (!mWaterSurfaceInfo->isWaterSurfaceExist()) {
        return false;
    }

    if (mWaterSurfaceInfo->getWaterSurfaceHeight() <= 200.0f) {
        return mIsRising;
    }

    return false;
}

/**
 * Forgets that the player moved upwards.
 */
void PlayerActionConditionSwimJump::setup() {
    mIsRising = false;
}
