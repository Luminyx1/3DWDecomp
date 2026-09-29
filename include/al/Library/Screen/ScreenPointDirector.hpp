#pragma once

#include <container/seadObjArray.h>
#include <math/seadVector.h>

namespace al {
class ScreenPointCheckGroup;
class ScreenPointTarget;
struct ScreenPointTargetHitInfo;

class ScreenPointDirector {
public:
    ScreenPointDirector(s32);

    void registerTarget(ScreenPointTarget*);
    void setCheckGroup(ScreenPointTarget*);
    bool hitCheckSegment(sead::ObjArray<ScreenPointTargetHitInfo>*, s32, const sead::Vector3f&,
                         const sead::Vector3f&);
    bool hitCheckScreenCircle(sead::ObjArray<ScreenPointTargetHitInfo>*, s32,
                              const sead::Vector2f&, f32);

    ScreenPointCheckGroup* mCheckGroup = nullptr;  // _0
};
}  // namespace al
