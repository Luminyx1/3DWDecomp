#pragma once

#include <math/seadQuat.h>

#include "Library/Joint/JointControllerBase.hpp"

namespace al {
class LiveActor;

class JointLocalQuatRotator : public JointControllerBase {
public:
    JointLocalQuatRotator(const LiveActor* pActor, const char* pJointName, const sead::Quatf* pQuat);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

private:
    const LiveActor* mActor;
    s32 mJointIndex = 0;
    const sead::Quatf* mQuat;
};

}  // namespace al
