#pragma once

class IUsePlayerAnimator;
class IUsePlayerCollision;
class IUsePlayerInput;
class IUsePlayerInputArranger;
class PlayerConstParam;
class IUsePlayerSubAction;
struct PlayerProperty;

/// The player systems every action gets handed.
struct PlayerActionArg {
    const IUsePlayerInput* getInput() const { return mInput; }

    PlayerProperty* mProperty;         // 0x0
    IUsePlayerAnimator* mAnimator;     // 0x8
    unsigned char _10[0x10];
    IUsePlayerCollision* mCollision;   // 0x20
    unsigned char _28[0x8];
    const IUsePlayerInput* mInput;     // 0x30
    IUsePlayerInputArranger* mInputArranger;  // 0x38
    IUsePlayerSubAction* mSubAction;   // 0x40
    unsigned char _48[0x10];
    const PlayerConstParam* mConstParam;  // 0x58
};
