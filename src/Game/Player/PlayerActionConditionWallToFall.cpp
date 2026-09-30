#include "Player/PlayerActionConditionWallToFall.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/Normal/PlayerConstParam.hpp"

/**
 * Holds when the player lets go of the wall.
 * @param pInput the player's input
 * @param pCollision the player's collision
 * @param pConstParam the player's tuning values
 */
PlayerActionConditionWallToFall::PlayerActionConditionWallToFall(const IUsePlayerInput* pInput,
                                                                 const IUsePlayerCollision* pCollision,
                                                                 const PlayerConstParam* pConstParam)
    : mInput(pInput), mCollision(pCollision), mConstParam(pConstParam) {}

/**
 * @return whether the wall has been gone for more than three frames, or the stick has pushed away
 *         from it (within 45 degrees of its normal) for longer than WallApartFrame
 */
bool PlayerActionConditionWallToFall::check() {
    if (!mCollision->isOnBackWall()) {
        mNoWallFrame++;
        return mNoWallFrame > 3;
    }

    mNoWallFrame = 0;
    if (mInput->getMoveVec().length() > 0.7f) {
        sead::Vector3f moveDir = mInput->getMoveVec();
        al::normalize(&moveDir);
        IUsePlayerCollision::Info info = {};
        mCollision->getBackWallInfo(&info);
        if (info.mNormal.dot(moveDir) > 0.70710678f) {
            mApartFrame++;
            if (mApartFrame > mConstParam->getWallApartFrame()) {
                return true;
            }
        }

        return false;
    }

    mApartFrame = 0;
    return false;
}

/**
 * Restarts both counts.
 */
void PlayerActionConditionWallToFall::setup() {
    mApartFrame = 0;
    mNoWallFrame = 0;
}
