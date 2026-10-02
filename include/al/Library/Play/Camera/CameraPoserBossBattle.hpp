#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {

class CameraPoserBossBattle : public CameraPoser_RS {
public:
    CameraPoserBossBattle(const char* pName, const sead::Vector3f* pPos);

    void init() override;
    void loadParam(const ByamlIter& rIter) override;
    void start(const CameraStartInfo& rInfo) override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void update() override;
    bool isEnableRotateByPad() const override;

    void setPosPtr(const sead::Vector3f* pPos);
    bool tryChangeFollowCamera();
    bool isCameraTargetOutOfRangeY() const;
    bool tryChangeTowerCamera();
    f32 calcOutOfRangeDistance() const;

    void exeTower();
    void exeFollow();
    void endFollow();
    void exeFollowNear();

public:
    const sead::Vector3f* mBossPos;
    f32 mOffsetY = 950.0f;
    f32 mOffsetYAdd = 0.0f;
    f32 mOffsetYNear = 1350.0f;
    f32 mDistance = 3500.0f;
    f32 mDistanceAdd = 0.0f;
    f32 mCameraDistanceNear = 2000.0f;
    f32 mCameraDistanceFar = 3500.0f;
    f32 mAngleDegreeV = 30.0f;
    f32 mAngleDegreeVNear = 10.0f;
    f32 mAngleDegreeVFar = 30.0f;
    f32 mToFollowDistance = 1000.0f;
    f32 mToFollowCylinderHeight = 500.0f;
    f32 mOutOfRangeDistance = 1800.0f;
    f32 mOutOfRangeDistanceMax = 2300.0f;
    sead::Vector3f mTowerEye;
    sead::Vector3f mStartEye;
    bool mIsValidFollowNear = false;
    sead::Vector3f mTowerDir = sead::Vector3f::zero;
    f32 mPrevAngleDegreeV = 0.0f;
    bool mIsInterpoleAngleV = false;
    f32 mPrevOffsetY = 0.0f;
    f32 mPrevDistanceAdd = 0.0f;
};

static_assert(sizeof(CameraPoserBossBattle) == 0x1c0);

}  // namespace al
