#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerWaterSurfaceInfo;
struct PlayerProperty;

/// Holds when the player swimming near the surface may jump out of the water.
class PlayerActionConditionSwimJump : public PlayerActionCondition {
public:
    PlayerActionConditionSwimJump(const IUsePlayerWaterSurfaceInfo*, const PlayerProperty*, bool);

    bool check() override;
    void setup() override;

private:
    const IUsePlayerWaterSurfaceInfo* mWaterSurfaceInfo;  // 0x8
    const PlayerProperty* mProperty;                      // 0x10
    bool mIsRising = false;                               // 0x18
    bool mIsAcceptStill;                                  // 0x19
};
