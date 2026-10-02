#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {

class CameraPoserKinopioBrigade_RS : public CameraPoser_RS {
public:
    struct V3bool {
        bool x;
        bool y;
        bool z;
    };

    CameraPoserKinopioBrigade_RS(const char* pName);

    void start(const CameraStartInfo& rInfo) override;
    void resetValues(f32 angleH, f32 angleV);
    void update() override;
    void calcDrcAngle();
    void calcBaseAngle();
    void convergeBaseAngle();
    void commitCamera();
    void followLookAt(f32 rate, const sead::Vector3f& rTargetTrans, const sead::Vector3f& rOffset,
                      const V3bool& rIsFollow);
    void loadParam(const ByamlIter& rIter) override;
    void transferParam(const CameraPoserKinopioBrigade_RS* pOther);

    bool isEnableRotateByPad() const override { return true; }

public:
    f32 mStartAngleV = 30.0f;
    f32 mStartAngleH = 45.0f;
    f32 mAngleV = 30.0f;
    f32 mAngleH = 45.0f;
    f32 mAngleVMin = 0.0f;
    f32 mAngleVMax = 89.0f;
    f32 mBaseAngleV = 30.0f;
    f32 mBaseAngleH = 45.0f;
    f32 mCurrentBaseAngleV = 30.0f;
    f32 mCurrentBaseAngleH = 45.0f;
    f32 mDrcAngleV = 0.0f;
    f32 mDrcAngleH = 0.0f;
    f32 mSmoothDrcAngleV = 0.0f;
    f32 mSmoothDrcAngleH = 0.0f;
    sead::Vector3f mPadAxisX = sead::Vector3f::ex;
    sead::Vector3f mPadAxisY = sead::Vector3f::ey;
    sead::Vector3f mPadAxisZ = sead::Vector3f::ez;
    f32 mPrevPadAngleV = 0.0f;
    f32 mPrevPadAngleH = 0.0f;
    f32 mDistance = 5500.0f;
    bool mIsSetDistanceDetail = false;
    f32 mDistanceX = 5500.0f;
    f32 mDistanceY = 5500.0f;
    f32 mDistanceZ = 5500.0f;
    V3bool mIsFollowLookAtPlayer = {false, false, false};
    bool mIsResetForce = false;
    bool mIsTransferAlways = false;
    sead::Vector3f mFollowLookAtPlayerOffset = sead::Vector3f::zero;
    V3bool mIsUseMoveLimitMin = {false, false, false};
    V3bool mIsUseMoveLimitMax = {false, false, false};
    sead::Vector3f mMoveLimitMin = sead::Vector3f::zero;
    sead::Vector3f mMoveLimitMax = sead::Vector3f::zero;
    s32 mResetFrame = 0;
    f32 mResetSpeedH = 0.0f;
    f32 mResetSpeedV = 0.0f;
    bool mIsSetStartAngle = true;
};

static_assert(sizeof(CameraPoserKinopioBrigade_RS) == 0x200);

}  // namespace al
