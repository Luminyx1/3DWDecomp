#pragma once

#include <basis/seadTypes.h>

namespace al {
class ScreenPointTarget;

class ScreenPointCheckGroup {
public:
    ScreenPointCheckGroup(s32);

    void setValid(ScreenPointTarget*);
    void setInvalid(ScreenPointTarget*);
    ScreenPointTarget* getTarget(s32) const;
    void setTarget(ScreenPointTarget*);

private:
    s32 mCapacity;                  // _0
    s32 mTargetNum = 0;             // _4
    s32 mValidTargetNum = 0;        // _8
    ScreenPointTarget** mTargets;   // _10
};
}  // namespace al
