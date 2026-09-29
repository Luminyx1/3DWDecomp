#include "Player/PlayerActionConditionIsPropeller.hpp"
#include "Player/IUsePlayerEquipment.hpp"

/**
 * Holds while the equipped item provides the propeller action.
 * @param pEquipment the player's equipment
 */
PlayerActionConditionIsPropeller::PlayerActionConditionIsPropeller(const IUsePlayerEquipment* pEquipment) : mEquipment(pEquipment) {}

/**
 * @return whether the propeller action is available
 */
bool PlayerActionConditionIsPropeller::check() {
    return mEquipment->isEquipmentAction(static_cast<EPlayerEquipmentAction>(1));
}
