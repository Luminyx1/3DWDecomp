#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjArray.h>
#include <math/seadMatrix.h>

class IJointController {
public:
    virtual void calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) = 0;
    virtual void appendJointId(s32 jointId) = 0;
};

namespace al {

class JointControllerBase : public IJointController {
public:
    JointControllerBase();

    void appendJointId(s32 jointId) override;

    bool findNextId(s32* pId, s32 current) const;
    bool isExistId(s32 jointId) const;

private:
    sead::FixedObjArray<s32, 8> mJointIds;
};

static_assert(sizeof(JointControllerBase) == 0xa8);

}  // namespace al
