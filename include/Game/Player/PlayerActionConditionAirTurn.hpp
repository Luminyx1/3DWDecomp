#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerAirTurnCheck;

/// Holds while the player turns around in the air.
class PlayerActionConditionAirTurn : public PlayerActionCondition {
public:
    PlayerActionConditionAirTurn(const IUsePlayerAirTurnCheck*);

    bool check() override;

private:
    const IUsePlayerAirTurnCheck* mAirTurnCheck;  // 0x8
};
