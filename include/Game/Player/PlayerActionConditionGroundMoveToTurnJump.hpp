#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerDashChecker;
class IUsePlayerInput;
class PlayerConstParam;
struct PlayerProperty;

/// Holds when the stick points back while walking (not dashing), for the side somersault.
/// The ground move action also feeds it the stick during a dash to time the dash brake.
class PlayerActionConditionGroundMoveToTurnJump : public PlayerActionCondition {
public:
    PlayerActionConditionGroundMoveToTurnJump(const PlayerProperty*, const IUsePlayerInput*,
                                              const IUsePlayerDashChecker*, const PlayerConstParam*);

    bool check() override;
    void setup() override;

    void checkStickOn(const sead::Vector3f&);
    void checkCancel();

private:
    const PlayerProperty* mProperty;          // 0x8
    const IUsePlayerInput* mInput;            // 0x10
    const IUsePlayerDashChecker* mDashChecker;  // 0x18
    const PlayerConstParam* mConstParam;      // 0x20
    s32 mBrakeCommandFrame = 0;               // 0x28
};
