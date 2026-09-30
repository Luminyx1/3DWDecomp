#pragma once

#include "Library/Joint/JointControllerBase.hpp"

namespace al {
class JointDirectionInfo;

class JointLocalDirController : public JointControllerBase {
public:
    JointLocalDirController(const JointDirectionInfo* pInfo);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) override;

private:
    const JointDirectionInfo* mInfo;
};

}  // namespace al
