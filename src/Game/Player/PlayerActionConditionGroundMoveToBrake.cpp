#include "Player/PlayerActionConditionGroundMoveToBrake.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Player/IUsePlayerDashChecker.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/Normal/PlayerConstParam.hpp"
#include "Player/Normal/PlayerProperty.hpp"
#include "Player/PlayerActionFunc.hpp"

/**
 * Holds after the stick is pulled back while running fast.
 * @param pProperty the player's physical state
 * @param pInput the player's input
 * @param pDashChecker dash state
 * @param pConstParam the player's tuning values
 */
PlayerActionConditionGroundMoveToBrake::PlayerActionConditionGroundMoveToBrake(
    const PlayerProperty* pProperty, const IUsePlayerInput* pInput, const IUsePlayerDashChecker* pDashChecker,
    const PlayerConstParam* pConstParam)
    : mProperty(pProperty), mInput(pInput), mDashChecker(pDashChecker), mConstParam(pConstParam) {}

/**
 * While the stick is held forward the brake window counts down; once it is let go or points back
 * within the window, the brake starts.
 * @return whether to brake
 */
bool PlayerActionConditionGroundMoveToBrake::check() {
    f32 stickLength = mInput->getMoveVec().length();
    sead::Vector3f moveDir = mInput->getMoveVec();
    al::verticalizeVec(&moveDir, mProperty->mGroundUp, moveDir);
    al::normalizeOrZero(&moveDir);
    bool isOpposite = PlayerActionFunc::isOppositeSide(moveDir, mProperty->mFront);

    if (stickLength > 0.9f && !isOpposite) {
        checkStickOn(moveDir);
    }

    checkCancel();

    if (stickLength < 0.3f || isOpposite) {
        return mBrakeCommandFrame != 0;
    }

    if (mBrakeCommandFrame != 0) {
        mBrakeCommandFrame--;
    }

    return false;
}

/**
 * Opens the brake window while the stick points forward during a fast run.
 * @param rStick stick direction
 */
void PlayerActionConditionGroundMoveToBrake::checkStickOn(const sead::Vector3f& rStick) {
    if (al::isNearZero(rStick, 0.001f)) {
        return;
    }

    sead::Vector3f hVel;
    al::verticalizeVec(&hVel, mProperty->mGroundUp, mProperty->mVelocity);
    sead::Vector3f hDir = hVel;
    al::normalizeOrZero(&hDir);
    const sead::Vector3f& rFront = mProperty->mFront;

    if (hVel.length() < mConstParam->getDashBrakeSpeed() || hVel.dot(rStick) < 0.70710678f ||
        rStick.dot(rFront) < 0.70710678f) {
        mBrakeCommandFrame = 0;
        return;
    }

    mBrakeCommandFrame = mConstParam->getDashBrakeCommandFrame();
}

/**
 * Closes the brake window once the player neither runs nor dashes.
 */
void PlayerActionConditionGroundMoveToBrake::checkCancel() {
    if (!mDashChecker->isRunningOnGround() && !mDashChecker->isDashing()) {
        mBrakeCommandFrame = 0;
    }
}

/**
 * Closes the brake window.
 */
void PlayerActionConditionGroundMoveToBrake::setup() {
    mBrakeCommandFrame = 0;
}
