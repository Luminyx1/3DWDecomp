#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerDashChecker;
class IUsePlayerInput;
class PlayerConstParam;
struct PlayerProperty;

/// Holds for a few frames after the stick is pulled back (or let go) while running fast, to skid to a stop.
class PlayerActionConditionGroundMoveToBrake : public PlayerActionCondition {
public:
    PlayerActionConditionGroundMoveToBrake(const PlayerProperty*, const IUsePlayerInput*, const IUsePlayerDashChecker*,
                                           const PlayerConstParam*);

    bool check() override;
    void checkStickOn(const sead::Vector3f&);
    void checkCancel();
    void setup() override;

private:
    const PlayerProperty* mProperty;            // 0x8
    const IUsePlayerInput* mInput;              // 0x10
    const IUsePlayerDashChecker* mDashChecker;  // 0x18
    const PlayerConstParam* mConstParam;        // 0x20
    s32 mBrakeCommandFrame = 0;                 // 0x28
};
