#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class CameraOffsetPreset;

class CameraPoserTower_RS : public CameraPoser_RS {
public:
    struct FixLookAtDistanceInfo {
        void setFix(bool isFixDistance) { isFix = isFixDistance; }

        bool isFix = false;
    };

    CameraPoserTower_RS(const char* pName, const sead::Vector3f* pAxisPos);

    void init() override;
    void initByPlacementObj(const PlacementInfo& rInfo) override;
    void loadParam(const ByamlIter& rIter) override;
    void start(const CameraStartInfo& rInfo) override;
    sead::Vector3f calcOffset();
    f32 calcDistance();
    void setParams(f32 marginAngleH, f32 angleV);
    void movement() override;
    void update() override;
    void resetInputRotate(f32 speed, s32 minStep);
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;

    void exeTower();
    void exeTowerInput();
    void exeFollow();

    void setFixToSwitchFollowDistance(bool isFix);
    void setZoomIn(f32 distance, f32 distanceNear, f32 offsetY, s32 frame, bool isUnused);

private:
    void calcAxisPos(sead::Vector3f* pAxisPos) const {
        if (mAxisPosPtr != nullptr) {
            pAxisPos->set(*mAxisPosPtr);
        } else if (mIsSetAxisPos) {
            pAxisPos->set(mAxisPos);
        } else {
            pAxisPos->setMul(getViewMtx(), mLocalAxisPos);
        }
    }

    bool isResetting() const { return mResetStep >= 0 && mResetStep < mResetStepNum; }

    void startInterpRotate(f32 startAngleH, f32 endAngleH, f32 speed, s32 minStep);
    f32 calcAxisDistanceH() const;
    bool isNearAxis(f32 distance) const;

public:
    const sead::Vector3f* mAxisPosPtr;
    sead::Vector3f mAxisPos = {0.0f, 0.0f, 0.0f};
    bool mIsSetAxisPos = false;
    sead::Vector3f mLocalAxisPos = {0.0f, 0.0f, 0.0f};
    s32 _16c;
    s32 mResetStep = -1;
    s32 mResetStepNum = -1;
    f32 mResetStartAngleH = 0.0f;
    f32 mResetEndAngleH = 0.0f;
    f32 mResetSpeed = 1.1f;
    FixLookAtDistanceInfo* mFixLookAtDistanceInfo = nullptr;
    CameraOffsetPreset* mOffsetPreset = nullptr;
    f32 mDistance = 1800.0f;
    f32 mDistanceNear = 1800.0f;
    f32 mZoomDistance = 1800.0f;
    f32 mZoomDistanceNear = 1800.0f;
    f32 mZoomOffsetY = 0.0f;
    s32 mZoomFrame = 0;
    s32 mZoomStep = 0;
    f32 mSwitchToFollowDistance = 1000.0f;
    f32 mAngleH = 0.0f;
    f32 mTargetAngleH = 0.0f;
    f32 mMarginAngleH = 60.0f;
    f32 mUserMarginAngleH = -1.0f;
    f32 mCurrentMarginAngleH = 60.0f;
    f32 mFollowSpeedH = 0.0f;
    f32 mInputSpeedH = 0.0f;
    f32 mInterpAngleH = 0.0f;
    sead::Vector3f mTowerEye = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mPrevTargetTrans = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mPrevAxisPos = {0.0f, 0.0f, 0.0f};
    f32 mInputOffsetAngleH = 0.0f;
    f32 mInterpRotateSpeedByFrame = 1.1f;
    bool mIsClampInMarginInputOffsetAngleH = false;
    bool mIsValidVelocityOffset = false;
    bool mIsZoomIn = false;
    f32 mVelocityOffset = 0.0f;
    f32 mVelocityOffsetTarget = 0.0f;
    f32 mVelocityOffsetMax = 500.0f;
};

static_assert(sizeof(CameraPoserTower_RS) == 0x218);

}  // namespace al
