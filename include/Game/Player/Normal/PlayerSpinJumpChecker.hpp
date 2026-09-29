#pragma once

#include <basis/seadTypes.h>

class IUsePlayerInput;

/// Detects the spin input (shaking the controller) for a spin jump.
class PlayerSpinJumpChecker {
public:
    PlayerSpinJumpChecker(const IUsePlayerInput*);

    void update();

    bool isSpin() const { return mSpinFrame != 0; }

private:
    const IUsePlayerInput* mInput;  // 0x0
    unsigned char _8[0x608];
    s32 mSpinFrame;  // 0x610
};
