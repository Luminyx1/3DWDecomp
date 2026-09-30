#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserLookDown : public CameraPoser_RS {
public:
    CameraPoserLookDown(const char* pName);

    void loadParam(const ByamlIter& rIter) override;
    void start(const CameraStartInfo& rInfo) override;
    void update() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;

public:
    sead::Vector3f mStartAt;
    f32 mOffsetY = -200.0f;
    f32 mDistance = 2200.0f;
    f32 mAngle = 60.0f;
    bool mIsRotateH = true;
    bool mIsRotateV = true;
    bool mIsResetAngleIfSwitchTarget = false;
    bool mIsForceFollow = false;
    bool mIsPosYLocked = false;
};

static_assert(sizeof(CameraPoserLookDown) == 0x168);

}  // namespace al
