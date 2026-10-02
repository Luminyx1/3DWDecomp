#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {

class CameraPoserCart : public CameraPoser_RS {
public:
    CameraPoserCart(const char* pName);

    void start(const CameraStartInfo& rInfo) override;

    void stop();
    void restart();
    void exeFollow();
    void exeStop();
    void setUseDestinationAngle(f32 angleDegree, s32 step);

private:
    f32 mLookAtOffsetY = 120.0f;
    f32 mDistance = 900.0f;
    f32 mAngleDegreeV = 20.0f;
    f32 mDestinationAngleDegree = 0.0f;
    s32 mDestinationStep = -1;
    sead::Vector3f mDirH = {0.0f, 0.0f, 0.0f};
};

static_assert(sizeof(CameraPoserCart) == 0x168);

}  // namespace al
