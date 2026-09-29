#pragma once

#include "Player/PlayerDef.hpp"

/// The item the player has equipped (implemented by PlayerEquipmentDirector).
class IUsePlayerEquipment {
public:
    virtual bool isEquipped(EPlayerEquipmentType) const = 0;
    virtual bool isEquippedSomething() const = 0;
    virtual bool isEquipmentAction(EPlayerEquipmentAction) const = 0;
};
