#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerEquipment;

/// Holds while the player has the first equipment type equipped.
class PlayerActionConditionIsEquipped : public PlayerActionCondition {
public:
    PlayerActionConditionIsEquipped(const IUsePlayerEquipment*);

    bool check() override;

private:
    const IUsePlayerEquipment* mEquipment;  // 0x8
};
