#include "Player/PlayerActionConditionFloorCode.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/IUsePlayerDamageInvalidCheck.hpp"
#include "Player/PlayerActionFunc.hpp"

/**
 * Holds while the player touches ground with a map code.
 * @param pCollision the player's collision
 * @param pCode map code to look for
 * @param isOnFloorOnly only while standing on the floor
 * @param pDamageInvalidCheck damage invincibility, which blocks this, or nullptr
 */
PlayerActionConditionFloorCode::PlayerActionConditionFloorCode(const IUsePlayerCollision* pCollision, const char* pCode,
                                                               bool isOnFloorOnly,
                                                               const IUsePlayerDamageInvalidCheck* pDamageInvalidCheck)
    : mCollision(pCollision), mCode(pCode), mIsOnFloorOnly(isOnFloorOnly), mDamageInvalidCheck(pDamageInvalidCheck) {}

/**
 * @return whether the player touches that code and is not invincible
 */
bool PlayerActionConditionFloorCode::check() {
    if (mDamageInvalidCheck != nullptr && mDamageInvalidCheck->isInvalid()) {
        return false;
    }

    if (mIsOnFloorOnly && !mCollision->isOnFloor()) {
        return false;
    }

    return PlayerActionFunc::checkMapCode(mCollision, mCode);
}
