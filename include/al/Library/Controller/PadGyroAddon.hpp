#pragma once

#include <controller/seadControllerAddon.h>
#include <math/seadVector.h>

namespace al {
class PadGyroAddon : public sead::ControllerAddon {
    SEAD_RTTI_OVERRIDE(PadGyroAddon, sead::ControllerAddon)

public:
    PadGyroAddon(sead::Controller* pController, s32 index);

    bool calc() override;

    bool tryUpdateGyroStatus();
    void getPose(sead::Vector3f* pSide, sead::Vector3f* pUp, sead::Vector3f* pFront) const;
    void getSDKPose(sead::Vector3f* pSide, sead::Vector3f* pUp, sead::Vector3f* pFront) const;

    bool isStatusOk() const { return mIsStatusOk; }
    const sead::Vector3f& getAngularVelocity() const { return mAngularVelocity; }
    const sead::Vector3f& getAngle() const { return mAngle; }
    s32 getSampleCount() const { return mSampleCount; }

private:
    bool mIsStatusOk = false;
    sead::Vector3f mSide = sead::Vector3f::ex;
    sead::Vector3f mUp = sead::Vector3f::ey;
    sead::Vector3f mFront = sead::Vector3f::ez;
    sead::Vector3f mSDKSide = sead::Vector3f::ex;
    sead::Vector3f mSDKUp = sead::Vector3f::ey;
    sead::Vector3f mSDKFront = sead::Vector3f::ez;
    sead::Vector3f mAngularVelocity = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mAngle = {0.0f, 0.0f, 0.0f};
    s64 mPrevSamplingNumber = 0;
    s32 mSampleCount = 0;
    s32 mIndex;
};
}  // namespace al
