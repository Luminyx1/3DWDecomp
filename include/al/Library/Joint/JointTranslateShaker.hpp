#pragma once

#include <container/seadRingBuffer.h>

#include "Library/Joint/JointControllerBase.hpp"

enum JointTranslateAxis : s32 {
    JointTranslateAxis_X,
    JointTranslateAxis_Y,
    JointTranslateAxis_Z,
    JointTranslateAxis_None,
};

namespace al {
class LiveActor;

class JointTranslateShaker : public JointControllerBase {
public:
    struct ShakeInfo {
        ShakeInfo() = default;

        ShakeInfo(s32 index, JointTranslateAxis translateAxis)
            : jointIndex(index), axis(translateAxis) {}

        s32 jointIndex;
        JointTranslateAxis axis = JointTranslateAxis_X;
    };

    JointTranslateShaker(const LiveActor* pActor, s32 maxJoints);

    void append(s32 jointIndex, JointTranslateAxis axis);
    void append(const char* pJointName, JointTranslateAxis axis);
    s32 getJointIndexActor(const char* pJointName);
    void setShake(f32 amplitude, s32 duration, f32 cycle, f32 attenuation);
    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

private:
    const LiveActor* mActor;
    sead::RingBuffer<ShakeInfo> mShakeInfos;
    f32 mAmplitude = 0.0f;
    s32 mStep = -1;
    s32 mDuration = -1;
    f32 mCycle = 0.0f;
    f32 mAttenuation = 0.0f;
};

static_assert(sizeof(JointTranslateShaker) == 0xe0);

}  // namespace al
