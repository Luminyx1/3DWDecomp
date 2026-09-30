#pragma once

#include <basis/seadTypes.h>

namespace al {
struct LiveActorFlag {
    LiveActorFlag();

    bool isDead = true;
    bool isClipped = false;
    bool isInvalidClipping = true;
    bool isDrawClipping = false;
    bool isClippedByLOD = false;
    bool isOffCalcAnim = false;
    bool isHideModel = false;
    bool isNoCollide = true;
    bool _8 = false;
    bool isValidMatCode = false;
    bool isValidCeilWallFloorMatCode = false;
    bool isAreaTarget = true;
    bool isUpdMovementEffectAudioCol = true;
    bool _d;
    bool _e;
    bool _f;
    u32 _10;
    u32 _14;
    u32 _18;
    bool _1c = true;
    bool isDeadAlive = false;
};
}  // namespace al
