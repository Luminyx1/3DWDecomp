#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerInput;
class PlayerConstParam;
struct PlayerProperty;

/// Holds when the stick points far enough away from where the standing player faces to turn on the spot.
class PlayerActionConditionWaitToPivot : public PlayerActionCondition {
public:
    PlayerActionConditionWaitToPivot(const IUsePlayerInput*, const PlayerProperty*, const PlayerConstParam*);

    bool check() override;

private:
    const IUsePlayerInput* mInput;        // 0x8
    const PlayerProperty* mProperty;      // 0x10
    const PlayerConstParam* mConstParam;  // 0x18
};
