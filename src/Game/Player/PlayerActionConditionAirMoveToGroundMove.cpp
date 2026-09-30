#include "Player/PlayerActionConditionAirMoveToGroundMove.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/PlayerActionFunc.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when the player lands while not moving upwards.
 * @param pCollision the player's collision
 * @param pProperty the player's physical state
 */
PlayerActionConditionAirMoveToGroundMove::PlayerActionConditionAirMoveToGroundMove(const IUsePlayerCollision* pCollision, const PlayerProperty* pProperty)
    : mCollision(pCollision), mProperty(pProperty) {}

/**
 * @return whether the player landed
 */
bool PlayerActionConditionAirMoveToGroundMove::check() {
    if (PlayerActionFunc::isUpperVelocity(mProperty)) {
        return false;
    }

    return mCollision->isOnFloor();
}
