#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerWaterSurfaceInfo;
class PlayerConstParam;
class PlayerFigureDirector;

/// Holds when the player is under water close enough below the surface to swim on it.
class PlayerActionConditionInWaterSurface : public PlayerActionCondition {
public:
    PlayerActionConditionInWaterSurface(const IUsePlayerWaterSurfaceInfo*, const PlayerFigureDirector*,
                                        const PlayerConstParam*);

    bool check() override;

private:
    const IUsePlayerWaterSurfaceInfo* mWaterSurfaceInfo;  // 0x8
    const PlayerFigureDirector* mFigureDirector;          // 0x10
    const PlayerConstParam* mConstParam;                  // 0x18
};
