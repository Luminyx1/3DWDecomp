#pragma once

#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {

class JointAimInfo {
public:
    enum LimitType : s32 {
        LimitType_Circle,
        LimitType_Oval,
        LimitType_Rect,
    };

    JointAimInfo();

    void makeTurnQuat(sead::Quatf* pQuat, const sead::Vector3f& rDir) const;
    void makeTurnQuatCircle(sead::Quatf* pQuat, const sead::Vector3f& rDir) const;
    void makeTurnQuatOval(sead::Quatf* pQuat, const sead::Vector3f& rDir) const;
    void makeTurnQuatRect(sead::Quatf* pQuat, const sead::Vector3f& rDir) const;
    void setBaseAimLocalDir(const sead::Vector3f& rDir);
    void setBaseUpLocalDir(const sead::Vector3f& rDir);
    void setBaseSideLocalDir(const sead::Vector3f& rDir);
    void setBaseMtxPtr(const sead::Matrix34f* pMtx);
    void setTargetPos(const sead::Vector3f& rPos);
    void setPowerRate(f32 rate);
    void setLimitDegreeCircle(f32 degree);
    void setLimitDegreeOval(f32 sidePlus, f32 sideMinus, f32 upPlus, f32 upMinus);
    void setLimitDegreeRect(f32 sidePlus, f32 sideMinus, f32 upPlus, f32 upMinus);
    void setEnableBackAim(bool isEnable);
    void addPowerRate(f32 rate);
    void subPowerRate(f32 rate);
    void setInterpoleRate(f32 rate);

    sead::Vector3f mTargetPos = sead::Vector3f::zero;
    sead::Vector3f mBaseAimLocalDir = sead::Vector3f::ez;
    sead::Vector3f mBaseSideLocalDir = sead::Vector3f::ex;
    sead::Vector3f mBaseUpLocalDir = sead::Vector3f::ey;
    const sead::Matrix34f* mBaseMtxPtr = nullptr;
    f32 mPowerRate = 1.0f;
    f32 mInterpoleRate = 0.1f;
    f32 mLimitDegree[4] = {30.0f, 30.0f, 30.0f, 30.0f};
    LimitType mLimitType = LimitType_Circle;
    bool mIsEnableBackAim = false;
};

static_assert(sizeof(JointAimInfo) == 0x58);

}  // namespace al
