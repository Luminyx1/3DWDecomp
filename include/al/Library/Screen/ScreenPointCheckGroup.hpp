#pragma once

#include <basis/seadTypes.h>

namespace al {
class ScreenPointTarget;

class ScreenPointCheckGroup {
public:
    ScreenPointCheckGroup(s32 maxTargets);

    void setValid(ScreenPointTarget* pTarget);
    void setInvalid(ScreenPointTarget* pTarget);
    ScreenPointTarget* getTarget(s32 index) const;
    void setTarget(ScreenPointTarget* pTarget);

    s32 getTargetNum() const { return mTargetNum; }
    s32 getValidTargetNum() const { return mValidTargetNum; }

private:
    s32 mMaxTargets;
    s32 mTargetNum = 0;
    s32 mValidTargetNum = 0;
    ScreenPointTarget** mTargets;
};
}  // namespace al
