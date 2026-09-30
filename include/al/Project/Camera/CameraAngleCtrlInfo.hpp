#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;
struct CameraObjectRequestInfo;

class CameraAngleCtrlInfo {
public:
    CameraAngleCtrlInfo();

    static CameraAngleCtrlInfo* createWithRelativeH();
    void load(const ByamlIter& rIter);
    void start(f32 angleV);
    void update(const sead::Vector2f& rStick, f32 sensitivityScale, bool isTriggerReset);
    bool receiveRequestFromObject(const CameraObjectRequestInfo& rInfo);
    void setAngleV(f32 angleV);
    bool isFixByRangeHV() const;
    bool isResetStartTiming() const;
    s32 getMaxResetStep() const;

    void setDefaultAngleV(f32 min, f32 max) {
        mDefaultMinAngleV = min;
        mDefaultMaxAngleV = max;
    }

    void setStartAngleV(f32 angle) { mStartAngleV = angle; }

    f32 getAngleH() const { return mAngleH; }

    f32 getAngleV() const { return mAngleV; }

private:
    struct ResetInfo {
        s32 step = -1;
        s32 maxStep = 0;
        f32 startAngleH = 0.0f;
        f32 startAngleV = 0.0f;
        f32 targetAngleV = 0.0f;
    };

    struct RequestInfo {
        s32 step = -1;
        s32 maxStep = 0;
        f32 startAngleV = 0.0f;
        f32 targetAngleV = 0.0f;
    };

    ResetInfo* mResetInfo;
    RequestInfo* mRequestInfo;
    bool mIsValidRotateH = false;
    s32 mRotateHType = 0;
    f32 mAngleH = 0.0f;
    f32 mTargetAngleH = 0.0f;
    f32 mMinAngleH = -45.0f;
    f32 mMaxAngleH = 45.0f;
    f32 mAngleV = 20.0f;
    f32 mStartAngleV = 20.0f;
    f32 mTargetAngleV = 20.0f;
    f32 mDefaultMinAngleV = -85.0f;
    f32 mDefaultMaxAngleV = 85.0f;
    f32 mSpeedV = 1.8f;
    f32 mTargetLerpRateV = 0.7f;
    f32 mLerpRateV = 0.1f;
    f32 mStickThreshold = 0.3f;
    f32 mSensitivityScale = 1.0f;
    bool mIsKeepPreAngleV = false;
    bool mIsSetResetAngleV = false;
    f32 mResetAngleV = 20.0f;
    bool mIsInvalidReceiveRequest = false;
};

static_assert(sizeof(CameraAngleCtrlInfo) == 0x60);

}  // namespace al
