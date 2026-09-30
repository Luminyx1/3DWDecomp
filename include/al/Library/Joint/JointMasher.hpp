#pragma once

#include "Library/Joint/JointControllerBase.hpp"

namespace al {
class LiveActor;

class JointMasher : public JointControllerBase {
public:
    struct MashInfo {
        s32 jointIndex;
        f32 rate;
    };

    JointMasher(const LiveActor* pActor, const bool* pIsValid, s32 maxJoints);

    void append(const char* pJointName, f32 rate);
    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

private:
    const LiveActor* mActor;
    const bool* mIsValid;
    sead::ObjArray<MashInfo> mMashInfos;
};

static_assert(sizeof(JointMasher) == 0xd8);

}  // namespace al
