#pragma once

#include "Player/IUsePlayerEquipment.hpp"
#include "Player/PlayerDef.hpp"

namespace al {
class HitSensor;
}

namespace sead {
class IDelegate;
}

/// Where the player's equipment goes when it is released.
enum PlayerReleaseEquipmentGoalType : unsigned long {};

/// The items the player has equipped (propeller box, cannon box, crown, ...).
class PlayerEquipmentDirector : public IUsePlayerEquipment {
public:
    bool isEquipped(EPlayerEquipmentType type) const override;
    bool isEquippedSomething() const override;
    bool isEquipmentAction(EPlayerEquipmentAction action) const override;

    bool tryEquip(EPlayerEquipmentType type, EPlayerEquipmentAction action, al::HitSensor* pSensor,
                  sead::IDelegate* pDelegate);
    void releaseEquipment(EPlayerEquipmentType type, PlayerReleaseEquipmentGoalType goalType);
    void releaseEquipmentForce(PlayerReleaseEquipmentGoalType goalType);
    void pauseHeadgear();
    void resumeHeadgear();
};
