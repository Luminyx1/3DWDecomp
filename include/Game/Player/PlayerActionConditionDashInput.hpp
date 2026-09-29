#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;
class PlayerConstParam;
struct PlayerProperty;

/// Holds while the dash button was held with the stick forward within the last few frames.
class PlayerActionConditionDashInput : public PlayerActionCondition {
public:
    PlayerActionConditionDashInput(const IUsePlayerInput*, const PlayerProperty*, const PlayerConstParam*);

    bool check() override;
    void setup() override;

private:
    const IUsePlayerInput* mInput;        // 0x8
    const PlayerProperty* mProperty;      // 0x10
    u32 mFrame = 0;                       // 0x18
    const PlayerConstParam* mConstParam;  // 0x20
};
