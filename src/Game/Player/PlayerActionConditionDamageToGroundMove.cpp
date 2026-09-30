#include "Player/PlayerActionConditionDamageToGroundMove.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/Normal/PlayerConstParam.hpp"

/**
 * Holds when the player walks out of a damage reaction.
 * @param pCollision the player's collision
 * @param pInput the player's input
 * @param pDamageEnd damage action (unused)
 * @param pConstParam the player's tuning values
 */
PlayerActionConditionDamageToGroundMove::PlayerActionConditionDamageToGroundMove(
    const IUsePlayerCollision* pCollision, const IUsePlayerInput* pInput, const IUsePlayerIsDamageEnd* pDamageEnd,
    const PlayerConstParam* pConstParam)
    : mCollision(pCollision), mInput(pInput), mDamageEnd(pDamageEnd), mConstParam(pConstParam) {}

/**
 * Counts the frames since the damage started.
 * @return whether the player is on the ground, past the cancel frame and tilting the stick
 */
bool PlayerActionConditionDamageToGroundMove::check() {
    mFrame++;

    if (!mCollision->isOnFloor()) {
        return false;
    }

    if (mFrame <= static_cast<u32>(mConstParam->getDamageCancelFrame())) {
        return false;
    }

    return !(mInput->getMoveVec().length() < 0.1f);
}

/**
 * Restarts the frame count.
 */
void PlayerActionConditionDamageToGroundMove::setup() {
    mFrame = 0;
}
