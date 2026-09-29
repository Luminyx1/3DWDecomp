#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollisionCheckArrow;
class IUsePlayerInput;
class PlayerConstParam;
struct PlayerProperty;

/// Holds when a ground pound is input high enough above the ground.
class PlayerActionConditionAirMoveToHipDrop : public PlayerActionCondition {
public:
    PlayerActionConditionAirMoveToHipDrop(const IUsePlayerInput*, const PlayerProperty*,
                                          IUsePlayerCollisionCheckArrow*, const PlayerConstParam*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;                 // 0x8
    const PlayerProperty* mProperty;               // 0x10
    IUsePlayerCollisionCheckArrow* mCheckArrow;    // 0x18
    const PlayerConstParam* mConstParam;           // 0x20
};
