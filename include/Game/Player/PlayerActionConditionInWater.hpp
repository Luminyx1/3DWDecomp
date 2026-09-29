#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCheckArea;
struct PlayerProperty;

/// Holds while the player is in a water area.
class PlayerActionConditionInWater : public PlayerActionCondition {
public:
    PlayerActionConditionInWater(const PlayerProperty*, const IUsePlayerCheckArea*);

    bool check() override;

private:
    const PlayerProperty* mProperty;  // 0x8
    const IUsePlayerCheckArea* mCheckArea;  // 0x10
};
