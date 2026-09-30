#pragma once

#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.hpp"

namespace al {
class LiveActor;

class JointRumbler : public JointControllerBase {
public:
    enum EAxis : u32 {
        EAxis_X,
        EAxis_Y,
        EAxis_Z,
    };

    struct Details {
        s32 delay;
        f32 rate;
    };

    JointRumbler(const LiveActor* pActor, const char* pJointName, f32 cycle, f32 power,
                 s32 duration, s32 startStep);

    void initDetails(EAxis axis, s32 delay, f32 rate);
    void start();
    void update();
    void updateEach(f32* pOut, EAxis axis);
    void reset();
    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

    bool isActive() const { return mDuration + mStartStep > mStep - mMaxDelay; }

private:
    const LiveActor* mActor;
    s32 mJointIndex = 0;
    f32 mCycle;
    f32 mPower;
    s32 mDuration;
    s32 mStartStep;
    s32 mMaxDelay = 0;
    Details mDetails[3];
    s32 mStep;
    sead::Vector3f mScale = {1.0f, 1.0f, 1.0f};
};

static_assert(sizeof(JointRumbler) == 0xf0);

}  // namespace al
