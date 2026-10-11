#pragma once

class IUsePlayerAnimator;
class IUsePlayerCollision;
class IUsePlayerInput;
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
    unsigned char _38[0x8];
    IUsePlayerSubAction* mSubAction;   // 0x40
};
