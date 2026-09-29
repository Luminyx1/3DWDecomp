#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerWaterSurfaceInfo;

/// Holds when there is no water surface near the player.
class PlayerActionConditionNoWaterSurface : public PlayerActionCondition {
public:
    PlayerActionConditionNoWaterSurface(const IUsePlayerWaterSurfaceInfo*);

    bool check() override;

private:
    const IUsePlayerWaterSurfaceInfo* mWaterSurfaceInfo;  // 0x8
};
