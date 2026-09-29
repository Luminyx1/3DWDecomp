#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace sead {
class PerspectiveProjection;
}

namespace al {
/// Shakes the camera by moving the projection offset.
class CameraShaker {
public:
    CameraShaker(sead::PerspectiveProjection* pProjection);

    void update();
    void startShake(s32 type);
    void startShakeByString(const char* pName);

    sead::PerspectiveProjection* mProjection;  // _0
    sead::Vector2f mOffset = {0.0f, 0.0f};     // _8
    s32 mFrame = -1;                           // _10
    s32 mType = 0;                             // _14
};
}  // namespace al
