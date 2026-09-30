#pragma once

#include "Library/Joint/JointControllerBase.hpp"
#include <math/seadQuat.h>

namespace al {
class LiveActor;

class JointQuatController : public JointControllerBase {
public:
    JointQuatController(const LiveActor* pActor, const sead::Quatf* pQuat);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

private:
    const LiveActor* mActor;
    const sead::Quatf* mQuat;
};

}  // namespace al
