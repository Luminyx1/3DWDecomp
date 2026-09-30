#pragma once

#include "Library/Joint/JointControllerBase.hpp"

namespace al {
class LiveActor;

class JointMtxController : public JointControllerBase {
public:
    JointMtxController(const LiveActor* pActor, const sead::Matrix34f* pMtx, bool isMulMtx);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

private:
    const LiveActor* mActor;
    const sead::Matrix34f* mMtx;
    bool mIsMulMtx;
};

static_assert(sizeof(JointMtxController) == 0xc0);

}  // namespace al
