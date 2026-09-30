#pragma once

#include <math/seadVector.h>

namespace al {

class JointDirectionInfo {
public:
    JointDirectionInfo();

    void setLocalBaseDir(const sead::Vector3f& rDir);
    void setLocalRotateAxis(const sead::Vector3f& rAxis);
    void setWorldTargetDir(const sead::Vector3f& rDir);
    void setPowerRate(f32 rate);
    void setLimitDegree(f32 degree);
    void addRate(f32 rate);
    void subRate(f32 rate);

    sead::Vector3f mLocalBaseDir = sead::Vector3f::ey;
    sead::Vector3f mLocalRotateAxis = sead::Vector3f::ex;
    sead::Vector3f mWorldTargetDir = sead::Vector3f::ey;
    f32 mPowerRate = 0.0f;
    f32 mLimitDegree = 0.0f;
};

static_assert(sizeof(JointDirectionInfo) == 0x2c);

}  // namespace al
