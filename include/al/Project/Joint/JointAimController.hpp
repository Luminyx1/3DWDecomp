#pragma once

#include <math/seadQuat.h>

#include "Library/Joint/JointControllerBase.hpp"

namespace al {
class JointAimInfo;

class JointAimController : public JointControllerBase {
public:
    JointAimController(const JointAimInfo* pInfo);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

private:
    const JointAimInfo* mInfo;
    sead::Quatf mQuat;
};

static_assert(sizeof(JointAimController) == 0xc0);

}  // namespace al
