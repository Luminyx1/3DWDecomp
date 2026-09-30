#pragma once

#include <math/seadQuat.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class CollisionPartsFilterActor;

class SurfMapParts : public LiveActor {
public:
    SurfMapParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;

    void exeWait();

    CollisionPartsFilterActor* mCollisionPartsFilter = nullptr;
    f32 mCheckOffset = 1000.0f;
    sead::Quatf mStartQuat = sead::Quatf::unit;
    sead::Vector3f mStartTrans = sead::Vector3f::zero;
    sead::Vector3f mUpDir = sead::Vector3f::ey;
    bool mIsEnableSlope = true;
};
}  // namespace al
