#pragma once

#include "Library/Joint/JointControllerBase.hpp"

namespace al {
class LiveActor;

class JointLocalTransController : public JointControllerBase {
public:
    JointLocalTransController(const LiveActor* pActor, const sead::Vector3f* pTrans);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

private:
    const LiveActor* mActor;
    const sead::Vector3f* mTrans;
};

}  // namespace al
