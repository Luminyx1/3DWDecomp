#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
class ScreenPointTarget;

class ScreenPointKeeper {
public:
    ScreenPointKeeper(s32 maxTargets);

    ScreenPointTarget* addTarget(LiveActor* pHost, const char* pName, f32 radius,
                                 const sead::Vector3f* pFollowPos,
                                 const sead::Matrix34f* pFollowMtx,
                                 const sead::Vector3f& rOffset);
    void update();
    ScreenPointTarget* getTarget(s32 index) const;
    void validate();
    void invalidate();
    void validateBySystem();
    void invalidateBySystem();
    ScreenPointTarget* getTarget(const char* pName) const;

    s32 getTargetNum() const { return mTargetNum; }

private:
    s32 mMaxTargets;
    s32 mTargetNum = 0;
    ScreenPointTarget** mTargets;
};
}  // namespace al
