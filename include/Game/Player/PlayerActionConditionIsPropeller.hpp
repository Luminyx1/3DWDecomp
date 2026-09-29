#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerEquipment;

/// Holds while the equipped item provides the propeller action.
class PlayerActionConditionIsPropeller : public PlayerActionCondition {
public:
    PlayerActionConditionIsPropeller(const IUsePlayerEquipment*);

    bool check() override;

private:
    const IUsePlayerEquipment* mEquipment;  // 0x8
};
