#pragma once

#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.hpp"

namespace al {

class JointLookAtController : public JointControllerBase {
public:
    JointLookAtController(s32 maxJoints, const sead::Matrix34f* pBaseMtx);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;
    void appendJoint(s32 jointIndex, f32 rate, const sead::Vector2f& rYawRange,
                     const sead::Vector2f& rPitchRange, const sead::Vector3f& rLocalFront,
                     const sead::Vector3f& rLocalUp);
    void appendJointLocalJudge(s32 jointIndex, f32 rate, const sead::Vector2f& rYawRange,
                               const sead::Vector2f& rPitchRange,
                               const sead::Vector3f& rLocalFront, const sead::Vector3f& rLocalUp,
                               const sead::Matrix34f* pJudgeMtx,
                               const sead::Vector3f& rJudgeFront,
                               const sead::Vector3f& rJudgeUp, bool isJudge);
    void appendJointNoJudge(s32 jointIndex, f32 rate, const sead::Vector2f& rYawRange,
                            const sead::Vector2f& rPitchRange, const sead::Vector3f& rLocalFront,
                            const sead::Vector3f& rLocalUp);
    void requestJointLookAt(const sead::Vector3f& rTargetPos);
    void invalidJoint(s32 jointIndex);
    void validAllJoint();

private:
    u8 _a8[0x38];
};

static_assert(sizeof(JointLookAtController) == 0xe0);

}  // namespace al
