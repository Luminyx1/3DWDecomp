#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class GateMapParts : public LiveActor {
public:
    GateMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;

    void start();
    void exeWait();
    void exeOpen();
    void updatePose(f32 rate);
    void exeBound();
    void exeEnd();

    sead::Quatf mQuat = sead::Quatf::unit;
    sead::Vector3f mTrans = sead::Vector3f::zero;
    sead::Quatf mMoveNextQuat = sead::Quatf::unit;
    sead::Vector3f mMoveNextTrans = sead::Vector3f::zero;
    s32 mMaxHitReactions = 10;
    s32 mOpenTime = 120;
    f32 mBoundRate = 0.1f;
    s32 mCurrentBoundSteps = 120;
    s32 mHitReactionCurrent = 0;
    f32 mCurrentBoundRate = 0.1f;
    s32 mHitReactionCount = 0;
};
}  // namespace al
