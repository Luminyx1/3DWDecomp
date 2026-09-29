#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCheckArea;
struct PlayerProperty;

/// Holds when the player gets back into water after having been out of it.
class PlayerActionConditionOutInWater : public PlayerActionCondition {
public:
    PlayerActionConditionOutInWater(const PlayerProperty*, const IUsePlayerCheckArea*);

    bool check() override;
    void setup() override;

private:
    const PlayerProperty* mProperty;         // 0x8
    const IUsePlayerCheckArea* mCheckArea;  // 0x10
    bool mIsOutOfWater = false;             // 0x18
};
