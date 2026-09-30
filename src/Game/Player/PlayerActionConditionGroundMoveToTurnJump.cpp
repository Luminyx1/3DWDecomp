#include "Player/PlayerActionConditionGroundMoveToTurnJump.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Player/IUsePlayerDashChecker.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/Normal/PlayerConstParam.hpp"
#include "Player/Normal/PlayerProperty.hpp"
#include "Player/PlayerActionFunc.hpp"

/**
 * Holds when the stick points back while walking.
 * @param pProperty the player's physical state
 * @param pInput the player's input
 * @param pDashChecker dash state
 * @param pConstParam the player's tuning values
 */
PlayerActionConditionGroundMoveToTurnJump::PlayerActionConditionGroundMoveToTurnJump(
    const PlayerProperty* pProperty, const IUsePlayerInput* pInput, const IUsePlayerDashChecker* pDashChecker,
    const PlayerConstParam* pConstParam)
    : mProperty(pProperty), mInput(pInput), mDashChecker(pDashChecker), mConstParam(pConstParam) {}

/**
 * @return whether the player is not dashing and the stick points to the opposite side of their front
 */
bool PlayerActionConditionGroundMoveToTurnJump::check() {
    sead::Vector3f moveDir = mInput->getMoveVec();
    al::verticalizeVec(&moveDir, mProperty->mGroundUp, moveDir);
    al::normalizeOrZero(&moveDir);
    if (mDashChecker->isDashing() || mDashChecker->isDashingFast()) {
        return false;
    }

    return PlayerActionFunc::isOppositeSide(moveDir, mProperty->mFront);
}

/**
 * Forgets the dash brake input.
 */
void PlayerActionConditionGroundMoveToTurnJump::setup() {
    mBrakeCommandFrame = 0;
}

/**
 * Starts the dash brake input window when the stick points back while running fast.
 * @param rStick stick direction
 */
void PlayerActionConditionGroundMoveToTurnJump::checkStickOn(const sead::Vector3f& rStick) {
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
 * Forgets the dash brake input once the player dashes again.
 */
void PlayerActionConditionGroundMoveToTurnJump::checkCancel() {
    if (mDashChecker->isDashing()) {
        mBrakeCommandFrame = 0;
    }
}
