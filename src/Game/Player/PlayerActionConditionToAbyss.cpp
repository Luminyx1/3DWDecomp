#include "Player/PlayerActionConditionToAbyss.hpp"
#include "Player/IUsePlayerCheckArea.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds when the player fell into an abyss area.
 * @param pProperty the player's physical state
 * @param pCheckArea area checker
 */
PlayerActionConditionToAbyss::PlayerActionConditionToAbyss(const PlayerProperty* pProperty, const IUsePlayerCheckArea* pCheckArea) : mProperty(pProperty), mCheckArea(pCheckArea) {}

/**
 * @return whether the player is in an abyss
 */
bool PlayerActionConditionToAbyss::check() {
    return mCheckArea->isInAbyss(mProperty->getTrans());
}
