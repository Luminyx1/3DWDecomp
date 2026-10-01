#include "Player/PlayerActionConditionOutInWater.hpp"
#include "Player/IUsePlayerCheckArea.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when the player gets back into water.
 * @param pProperty the player's physical state
 * @param pCheckArea area checker
 */
PlayerActionConditionOutInWater::PlayerActionConditionOutInWater(const PlayerProperty* pProperty,
                                                                 const IUsePlayerCheckArea* pCheckArea)
    : mProperty(pProperty), mCheckArea(pCheckArea) {}

/**
 * @return whether the player is in water after having left it
 */
bool PlayerActionConditionOutInWater::check() {
    bool isOutOfWater = mIsOutOfWater;
    bool isInWater = mCheckArea->isInWater(mProperty->getTrans());

    if (isOutOfWater) {
        return isInWater;
    }

    mIsOutOfWater = !isInWater;
    return false;
}

/**
 * Forgets that the player left the water.
 */
void PlayerActionConditionOutInWater::setup() {
    mIsOutOfWater = false;
}
