#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerSquatEnergy;

/// Holds once a squat is fully charged.
class PlayerActionConditionSquatEnergyFull : public PlayerActionCondition {
public:
    PlayerActionConditionSquatEnergyFull(const IUsePlayerSquatEnergy*);

    bool check() override;

private:
    const IUsePlayerSquatEnergy* mSquatEnergy;  // 0x8
};
