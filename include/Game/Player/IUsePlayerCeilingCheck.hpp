#pragma once

#include <basis/seadTypes.h>

/// Whether there is room above the player to stand up (implemented by PlayerCeilingCheck).
class IUsePlayerCeilingCheck {
public:
    virtual bool hasSpaceToStandUp() const = 0;
    virtual bool hasSpaceToStandUpForBig() const = 0;
    virtual s32 getSpaceLevel() const = 0;
};
