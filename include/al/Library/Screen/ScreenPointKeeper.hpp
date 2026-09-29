#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
class ScreenPointTarget;

class ScreenPointKeeper {
public:
    ScreenPointKeeper(s32);

    ScreenPointTarget* addTarget(LiveActor*, const char*, f32, const sead::Vector3f*,
                                 const sead::Matrix34f*, const sead::Vector3f&);
    void update();
    ScreenPointTarget* getTarget(s32) const;
    void validate();
    void invalidate();
    void validateBySystem();
    void invalidateBySystem();
    ScreenPointTarget* getTarget(const char*) const;

    s32 mMaxNumTargets;             // _0
    s32 mCurNumTargets = 0;         // _4
    ScreenPointTarget** mTargets;   // _8
};
}  // namespace al
