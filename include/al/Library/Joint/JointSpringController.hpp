#pragma once

#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.hpp"

namespace al {

class JointSpringController : public JointControllerBase {
public:
    JointSpringController();


    void setChildLocalPos(const sead::Vector3f& rPos);
    void setChildLocalMtxPtr(const sead::Matrix34f* pMtx);
    void setStability(f32 stability);
    void setFriction(f32 friction);
    void setLimitDegree(f32 degree);
    void setControlRate(f32 rate);
    void addControlRate(f32 rate);
    void subControlRate(f32 rate);
    void reset();
    void calcChildPos(sead::Vector3f* pPos, const sead::Matrix34f* pMtx) const;
    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

private:
    const sead::Matrix34f* mChildLocalMtxPtr = nullptr;
    sead::Vector3f mChildLocalPos;
    sead::Vector3f mChildPos = sead::Vector3f::zero;
    sead::Vector3f mVelocity = sead::Vector3f::zero;
    f32 mStability = 0.025f;
    f32 mFriction = 0.9f;
    f32 mLimitDegree = 30.0f;
    f32 mControlRate = 1.0f;
    bool mIsInitialized = false;
};

static_assert(sizeof(JointSpringController) == 0xe8);

}  // namespace al
