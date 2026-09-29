#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerWaterFlowField;

/// Holds while a water current pushes the player.
class PlayerActionConditionInWaterFlowField : public PlayerActionCondition {
public:
    PlayerActionConditionInWaterFlowField(const IUsePlayerWaterFlowField*);

    bool check() override;

private:
    const IUsePlayerWaterFlowField* mWaterFlowField;  // 0x8
};
