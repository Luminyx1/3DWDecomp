#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;
struct PlayerProperty;

/// Holds while the stick points roughly where the player faces.
class PlayerActionConditionFrontInput : public PlayerActionCondition {
public:
    PlayerActionConditionFrontInput(const IUsePlayerInput*, const PlayerProperty*, bool);

    bool check() override;

private:
    const IUsePlayerInput* mInput;    // 0x8
    const PlayerProperty* mProperty;  // 0x10
    bool mIsWide;                     // 0x18
};
