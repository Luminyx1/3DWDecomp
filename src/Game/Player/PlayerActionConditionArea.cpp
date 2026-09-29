#include "Player/PlayerActionConditionArea.hpp"
#include "Player/IUsePlayerCheckArea.hpp"
#include "Player/Normal/PlayerProperty.hpp"

/**
 * Holds while the player is in a kind of area.
 * @param pProperty the player's physical state
 * @param pCheckArea area checker
 * @param checkFunc area query to run, e.g. &IUsePlayerCheckArea::isInWater
 */
PlayerActionConditionArea::PlayerActionConditionArea(const PlayerProperty* pProperty,
                                                     const IUsePlayerCheckArea* pCheckArea, CheckFunc checkFunc)
    : mProperty(pProperty), mCheckArea(pCheckArea), mCheckFunc(checkFunc) {}

/**
 * @return whether the player is in the area
 */
bool PlayerActionConditionArea::check() {
    return (mCheckArea->*mCheckFunc)(mProperty->mTrans);
}
