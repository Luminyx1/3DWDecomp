#include "Library/LiveActor/ActorAreaFunction.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjMtxConnecter.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
/**
 * Registers a matrix that areas linked to a placement follow.
 * @param pAreaUser The area user.
 * @param pMtx The host matrix.
 * @param rInfo The actor init info.
 */
void registerAreaHostMtx(const IUseAreaObj* pAreaUser, const sead::Matrix34f* pMtx,
                         const ActorInitInfo& rInfo) {
    pAreaUser->getAreaObjDirector()->getMtxConnecterHolder()->registerParentMtx(
        pMtx, *rInfo.mPlacementInfo);
}

/**
 * Registers an actor's base matrix as the matrix its linked areas follow.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void registerAreaHostMtx(const LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerAreaHostMtx(pActor, pActor->getBaseMtx(), rInfo);
}

/**
 * Registers a matrix that areas linked to a placement follow in sync.
 * @param pAreaUser The area user.
 * @param pMtx The host matrix.
 * @param rInfo The actor init info.
 */
void registerAreaSyncHostMtx(const IUseAreaObj* pAreaUser, const sead::Matrix34f* pMtx,
                             const ActorInitInfo& rInfo) {
    pAreaUser->getAreaObjDirector()->getMtxConnecterHolder()->registerSyncParentMtx(
        pMtx, *rInfo.mPlacementInfo);
}

/**
 * Registers an actor's base matrix as the matrix its linked areas follow in sync.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void registerAreaSyncHostMtx(const LiveActor* pActor, const ActorInitInfo& rInfo) {
    registerAreaSyncHostMtx(pActor, pActor->getBaseMtx(), rInfo);
}

/**
 * Keeps an actor inside an area group by moving it back or limiting its velocity.
 * @param pPos The output edge position.
 * @param pActor The actor.
 * @param pGroup The area group.
 * @param pArea The area whose edge is used.
 * @return Whether the actor was revised.
 */
bool tryReviseVelocityInsideAreaObj(sead::Vector3f* pPos, LiveActor* pActor, AreaObjGroup* pGroup,
                                    const AreaObj* pArea) {
    if (!pGroup || !pArea) {
        return false;
    }

    if (!pGroup->getInVolumeAreaObj(getTrans(pActor))) {
        calcNearestAreaObjEdgePos(pPos, pArea, getTrans(pActor));
        setTrans(pActor, *pPos);
        setVelocityZero(pActor);
        return true;
    }

    f32 speed = getVelocity(pActor).length();
    sead::Vector3f nextPos = getTrans(pActor) + getVelocity(pActor);

    if (pGroup->getInVolumeAreaObj(nextPos)) {
        return false;
    }

    calcNearestAreaObjEdgePos(pPos, pArea, nextPos);
    sead::Vector3f velocity = *pPos - getTrans(pActor);

    if (velocity.length() > speed) {
        setLength(&velocity, speed);
    }

    setVelocity(pActor, velocity);
    return true;
}
}  // namespace al
