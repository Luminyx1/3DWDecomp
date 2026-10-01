#include "Player/PlayerActionConditionInWater.hpp"
#include "Player/IUsePlayerCheckArea.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds while the player is in a water area.
 * @param pProperty the player's physical state
 * @param pCheckArea area checker
 */
PlayerActionConditionInWater::PlayerActionConditionInWater(const PlayerProperty* pProperty, const IUsePlayerCheckArea* pCheckArea) : mProperty(pProperty), mCheckArea(pCheckArea) {}

/**
 * @return whether the player is in water
 */
bool PlayerActionConditionInWater::check() {
    return mCheckArea->isInWater(mProperty->getTrans());
}
