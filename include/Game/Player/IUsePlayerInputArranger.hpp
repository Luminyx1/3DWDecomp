#pragma once

#include <basis/seadTypes.h>

/// Adjusts how the player's controller input is read (implemented by PlayerInput).
class IUsePlayerInputArranger {
public:
    virtual void resetPrecedingJump() = 0;
    virtual void invalidateFrame(u32) = 0;
};
