#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {

class CameraPoserQuickTurn : public CameraPoser_RS {
public:
    CameraPoserQuickTurn(const char* pName);

    void init() override;
    void loadParam(const ByamlIter& rIter) override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void reset() override;
    void start(const CameraStartInfo& rInfo) override;

    void setFollow();
    void exeFollow();
    void exeRace();
    void calcTargetFrontLocal(sead::Vector3f* pFront, bool isUseTargetFrontIfStopped) const;

private:
    f32 mOffsetY = 400.0f;
    f32 mDistance = 2000.0f;
    f32 mAngleDegreeV = 20.0f;
    f32 mAngleDegreeH = 0.0f;
    bool mIsResetAngleIfSwitchTarget;
    f32 mDefaultDistance = 0.0f;
    const sead::Vector3f* mFrontDirPtr = nullptr;
    sead::Vector3f mFrontDir;
    f32 mRotateAngle;
    bool mIsTurnToVelocity;

public:
    bool mIsRotateFast = false;
};

static_assert(sizeof(CameraPoserQuickTurn) == 0x180);

}  // namespace al
