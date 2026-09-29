#include "Player/PlayerActionConditionIsEquipped.hpp"
#include "Player/IUsePlayerEquipment.hpp"

/**
 * Holds while the player has the first equipment type equipped.
 * @param pEquipment the player's equipment
 */
PlayerActionConditionIsEquipped::PlayerActionConditionIsEquipped(const IUsePlayerEquipment* pEquipment) : mEquipment(pEquipment) {}

/**
 * @return whether that equipment is equipped
 */
bool PlayerActionConditionIsEquipped::check() {
    return mEquipment->isEquipped(static_cast<EPlayerEquipmentType>(0));
}
