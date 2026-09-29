#pragma once

#include <basis/seadTypes.h>

/// Cat-suit wall climbing time (implemented by PlayerWallClimbFrameControl).
class IUsePlayerWallClimbInfo {
public:
    virtual s32 getWallClimbFrame() const = 0;
    virtual s32 getWallClimbDashFrame() const = 0;
    virtual bool isWallClimbCountOver() const = 0;
};
