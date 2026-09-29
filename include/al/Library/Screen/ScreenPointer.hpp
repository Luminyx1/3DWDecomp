#pragma once

#include <container/seadObjArray.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class LiveActor;
class ScreenPointDirector;
class ScreenPointTarget;

struct ScreenPointTargetHitInfo {
    ScreenPointTarget* mTarget;     // _0
    f32 mDistance;                  // _8
    sead::Vector3f mHitPos;         // _c
    sead::Vector3f mHitNormal;      // _18
};

class ScreenPointer {
public:
    ScreenPointer(const ActorInitInfo&, const LiveActor*, const sead::Vector3f*);

    bool hitCheckSegment(const sead::Vector3f&, const sead::Vector3f&);
    bool hitCheckScreenCircle(const sead::Vector2f&, f32);

    const LiveActor* mActor;                                 // _0
    sead::Vector3f mHitPos = sead::Vector3f::zero;           // _8
    sead::Vector3f mHitNormal = sead::Vector3f::zero;        // _14
    const sead::Vector3f* mPos;                              // _20
    ScreenPointDirector* mDirector = nullptr;                // _28
    sead::ObjArray<ScreenPointTargetHitInfo> mHitInfoArray;  // _30
};

static_assert(sizeof(ScreenPointer) == 0x50);
}  // namespace al
