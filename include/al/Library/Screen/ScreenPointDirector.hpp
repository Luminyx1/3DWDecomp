#pragma once

#include <container/seadObjArray.h>
#include <math/seadVector.h>

namespace al {
class ScreenPointCheckGroup;
class ScreenPointTarget;
struct ScreenPointTargetHitInfo;

class ScreenPointDirector {
public:
    ScreenPointDirector(s32 maxTargets);

    void registerTarget(ScreenPointTarget* pTarget);
    void setCheckGroup(ScreenPointTarget* pTarget);
    bool hitCheckSegment(sead::ObjArray<ScreenPointTargetHitInfo>* pHitInfos, s32 maxHits,
                         const sead::Vector3f& rStart, const sead::Vector3f& rEnd);
    bool hitCheckScreenCircle(sead::ObjArray<ScreenPointTargetHitInfo>* pHitInfos, s32 maxHits,
                              const sead::Vector2f& rPos, f32 radius);

private:
    ScreenPointCheckGroup* mCheckGroup;
};
}  // namespace al
