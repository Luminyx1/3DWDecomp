#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace sead {
class PerspectiveProjection;
}  // namespace sead

namespace al {

class CameraShaker {
public:
    CameraShaker(sead::PerspectiveProjection* pProjection);

    void update();
    void startShake(s32 index);
    void startShakeByString(const char* pName);

private:
    sead::PerspectiveProjection* mProjection;
    sead::Vector2f mOffset = {0.0f, 0.0f};
    s32 mStep = -1;
    s32 mIndex = 0;
};

static_assert(sizeof(CameraShaker) == 0x18);

}  // namespace al
