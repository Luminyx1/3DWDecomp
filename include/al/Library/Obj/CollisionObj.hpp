#pragma once

#include <math/seadMatrix.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class Resource;

class CollisionObj : public LiveActor {
public:
    CollisionObj(const ActorInitInfo& rInfo, Resource* pResource, const char* pCollisionFileName,
                 HitSensor* pHitSensor, const sead::Matrix34f* pJoinMtx, const char* pSuffix);
};
}  // namespace al
