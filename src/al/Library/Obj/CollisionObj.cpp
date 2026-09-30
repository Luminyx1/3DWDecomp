#include "Library/Obj/CollisionObj.hpp"

#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"

namespace al {
/**
 * Constructs an actor holding a collision.
 * @param rInfo actor init info
 * @param pResource resource containing the collision
 * @param pCollisionFileName collision file name
 * @param pHitSensor sensor of the collision
 * @param pJoinMtx matrix the collision follows
 * @param pSuffix collision suffix
 */
CollisionObj::CollisionObj(const ActorInitInfo& rInfo, Resource* pResource,
                           const char* pCollisionFileName, HitSensor* pHitSensor,
                           const sead::Matrix34f* pJoinMtx, const char* pSuffix)
    : LiveActor("CollisionObj") {
    initActorSceneInfo(this, rInfo);
    initActorPoseTRSV(this);
    initActorCollisionWithResource(this, pResource, pCollisionFileName, pHitSensor, pJoinMtx,
                                   pSuffix);
    initExecutorCollisionMapObjDecorationMovement(this, rInfo);
}
}  // namespace al
