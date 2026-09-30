#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserRace : public CameraPoser_RS {
public:
    CameraPoserRace(const char* pName);

    void init() override;
    void loadParam(const ByamlIter& rIter) override;
    void start(const CameraStartInfo& rInfo) override;
    void update() override;

    void calcTargetFrontLocal(sead::Vector3f* pFront, bool isUnused) const;

public:
    const sead::Vector3f* mFrontDirPtr = nullptr;
    sead::Vector3f mFrontDir = sead::Vector3f::ez;
    f32 mRotateAngle = 0.0f;
    f32 mOffsetY = 120.0f;
    f32 mDistance = 1600.0f;
    f32 mAngleDegreeV = 20.0f;
    f32 mRotateRate1 = 0.1f;
    f32 mRotateRate2 = 0.3f;
    bool mIsTurnToVelocity = true;
};

static_assert(sizeof(CameraPoserRace) == 0x178);

}  // namespace al
