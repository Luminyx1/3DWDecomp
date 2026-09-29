#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCollision;
struct PlayerProperty;

/// Holds while the slope of the floor compares to an angle in a given way.
class PlayerActionConditionFloorAngle : public PlayerActionCondition {
public:
    /// How the floor's angle compares to the threshold for the condition to hold.
    enum EOperationType {
        cGreater = 0,
        cGreaterEqual = 1,
        cLessEqual = 2,
        cLess = 3,
    };

    PlayerActionConditionFloorAngle(const PlayerProperty*, const IUsePlayerCollision*, EOperationType, f32);

    bool check() override;

private:
    const PlayerProperty* mProperty;        // 0x8
    const IUsePlayerCollision* mCollision;  // 0x10
    EOperationType mOperationType;               // 0x18
    f32 mAngle;                             // 0x1c
};
