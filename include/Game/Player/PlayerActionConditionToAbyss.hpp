#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCheckArea;
struct PlayerProperty;

/// Holds when the player fell into an abyss area.
class PlayerActionConditionToAbyss : public PlayerActionCondition {
public:
    PlayerActionConditionToAbyss(const PlayerProperty*, const IUsePlayerCheckArea*);

    bool check() override;

private:
    const PlayerProperty* mProperty;  // 0x8
    const IUsePlayerCheckArea* mCheckArea;  // 0x10
};
