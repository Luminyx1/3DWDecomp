#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCheckArea;
class IUsePlayerWaterSurfaceInfo;
class PlayerConstParam;
class PlayerFigureDirector;
struct PlayerProperty;

/// Holds when the player should stop swimming on the surface and sink.
class PlayerActionConditionSinkWater : public PlayerActionCondition {
public:
    PlayerActionConditionSinkWater(const IUsePlayerWaterSurfaceInfo*, const PlayerFigureDirector*,
                                   const PlayerConstParam*, const IUsePlayerCheckArea*, const PlayerProperty*);

    bool check() override;

private:
    const IUsePlayerWaterSurfaceInfo* mWaterSurfaceInfo;  // 0x8
    const PlayerFigureDirector* mFigureDirector;          // 0x10
    const PlayerConstParam* mConstParam;                  // 0x18
    const IUsePlayerCheckArea* mCheckArea;                // 0x20
    const PlayerProperty* mProperty;                      // 0x28
};
