#pragma once

#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.hpp"

namespace al {

class JointLocalAxisRotator : public JointControllerBase {
public:
    JointLocalAxisRotator(f32* pDegree, const sead::Vector3f& rAxis, bool isLocal);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

private:
    sead::Vector3f mAxis;
    f32* mDegree;
    bool mIsLocal;
};

static_assert(sizeof(JointLocalAxisRotator) == 0xc8);

}  // namespace al
