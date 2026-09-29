#include "Player/PlayerActionConditionSquatEnergyFull.hpp"
#include "Player/IUsePlayerSquatEnergy.hpp"

/**
 * Holds once a squat is fully charged.
 * @param pSquatEnergy squat charge
 */
PlayerActionConditionSquatEnergyFull::PlayerActionConditionSquatEnergyFull(const IUsePlayerSquatEnergy* pSquatEnergy) : mSquatEnergy(pSquatEnergy) {}

/**
 * @return whether the charge is full
 */
bool PlayerActionConditionSquatEnergyFull::check() {
    return mSquatEnergy->getEnergy() == 1.0f;
}
