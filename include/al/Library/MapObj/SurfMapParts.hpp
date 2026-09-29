#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class CollisionPartsFilterActor;

class SurfMapParts : public LiveActor {
public:
    SurfMapParts(const char*);

    void init(const ActorInitInfo&) override;

    void exeWait();

    CollisionPartsFilterActor* mCollisionPartsFilter = nullptr;  // _148
    f32 mCheckOffset = 1000.0f;                                  // _150
    sead::Quatf mStartQuat = sead::Quatf::unit;                  // _154
    sead::Vector3f mStartTrans = sead::Vector3f::zero;           // _164
    sead::Vector3f mUpDir = sead::Vector3f::ey;                  // _170
    bool mIsEnableSlope = true;                                  // _17c
};
}  // namespace al
