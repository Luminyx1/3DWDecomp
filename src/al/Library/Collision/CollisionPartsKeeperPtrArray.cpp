#include "Library/Collision/CollisionPartsKeeperPtrArray.hpp"

#include "Library/Collision/CollisionCheckInfo.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace al {
/**
 * Creates a collision parts keeper without collision parts.
 */
CollisionPartsKeeperPtrArray::CollisionPartsKeeperPtrArray() = default;

/**
 * Checks whether any collision parts contain a point.
 * @param pHitInfo output hit info
 * @param rCheckInfo check info
 * @return true if a collision parts was hit
 */
s32 CollisionPartsKeeperPtrArray::checkStrikePoint(HitInfo* pHitInfo,
                                                   const CollisionCheckInfoBase& rCheckInfo) const {
    s32 num = mPartsArray->size();
    for (s32 i = 0; i < num; i++) {
        if (mPartsArray->at(i)->checkStrikePoint(pHitInfo, rCheckInfo.getPos(),
                                                 rCheckInfo.getTriangleFilter())) {
            return true;
        }
    }
    return false;
}

/**
 * Collects the collisions of a sphere.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @param isCheckNear whether to check near polygons
 * @param rMoveDir movement direction of the sphere
 * @return the number of hits
 */
s32 CollisionPartsKeeperPtrArray::checkStrikeSphere(SphereHitResultBuffer* pBuffer,
                                                    const SphereCheckInfo& rCheckInfo,
                                                    bool isCheckNear,
                                                    const sead::Vector3f& rMoveDir) const {
    s32 num = mPartsArray->size();
    s32 hitNum = 0;
    for (s32 i = 0; i < num; i++) {
        hitNum += mPartsArray->at(i)->checkStrikeSphere(pBuffer, rCheckInfo.getPos(),
                                                        rCheckInfo.mRadius, isCheckNear, rMoveDir,
                                                        rCheckInfo.getTriangleFilter());
        if (pBuffer->isFull()) {
            break;
        }
    }
    return hitNum;
}

/**
 * Collects the collisions of an arrow.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 CollisionPartsKeeperPtrArray::checkStrikeArrow(ArrowHitResultBuffer* pBuffer,
                                                   const ArrowCheckInfo& rCheckInfo) const {
    s32 num = mPartsArray->size();
    s32 hitNum = 0;
    for (s32 i = 0; i < num; i++) {
        hitNum += mPartsArray->at(i)->checkStrikeArrow(pBuffer, rCheckInfo.getPos(),
                                                       rCheckInfo.getDir(),
                                                       rCheckInfo.getTriangleFilter());
        if (pBuffer->isFull()) {
            break;
        }
    }
    return hitNum;
}

/**
 * Collects the collisions of a player sphere.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 CollisionPartsKeeperPtrArray::checkStrikeSphereForPlayer(
    SphereHitResultBuffer* pBuffer, const SphereCheckInfo& rCheckInfo) const {
    s32 num = mPartsArray->size();
    s32 hitNum = 0;
    for (s32 i = 0; i < num; i++) {
        hitNum += mPartsArray->at(i)->checkStrikeSphere(pBuffer, rCheckInfo.getPos(),
                                                        rCheckInfo.mRadius, false,
                                                        sead::Vector3f::zero,
                                                        rCheckInfo.getTriangleFilter());
        if (pBuffer->isFull()) {
            break;
        }
    }
    return hitNum;
}

/**
 * Collects the collisions of a disk. Unsupported.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 CollisionPartsKeeperPtrArray::checkStrikeDisk(DiskHitResultBuffer* pBuffer,
                                                  const DiskCheckInfo& rCheckInfo) const {
    return 0;
}

/**
 * Calls a delegate for every collision parts near a sphere.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rDelegate delegate to call
 */
void CollisionPartsKeeperPtrArray::searchWithSphere(
    const sead::Vector3f& rPos, f32 radius, sead::IDelegate1<CollisionParts*>& rDelegate) const {
    s32 num = mPartsArray->size();
    for (s32 i = 0; i < num; i++) {
        if (alCollisionUtil::isFarAway(*mPartsArray->at(i), rPos, radius)) {
            continue;
        }
        rDelegate.invoke(mPartsArray->at(i));
    }
}

/**
 * Calls a delegate for every collision parts near a sphere.
 * @param rCheckInfo check info
 * @param rDelegate delegate to call
 */
void CollisionPartsKeeperPtrArray::searchWithSphere(
    const SphereCheckInfo& rCheckInfo, sead::IDelegate1<CollisionParts*>& rDelegate) const {
    s32 num = mPartsArray->size();
    for (s32 i = 0; i < num; i++) {
        if (alCollisionUtil::isFarAway(*mPartsArray->at(i), rCheckInfo.getPos(),
                                       rCheckInfo.mRadius)) {
            continue;
        }
        rDelegate.invoke(mPartsArray->at(i));
    }
}

/**
 * Finishes initialization.
 */
void CollisionPartsKeeperPtrArray::endInit() {}

/**
 * Adds collision parts. Unused.
 * @param pParts collision parts
 */
void CollisionPartsKeeperPtrArray::addCollisionParts(CollisionParts* pParts) {}

/**
 * Connects collision parts. Unused.
 * @param pParts collision parts
 */
void CollisionPartsKeeperPtrArray::connectToCollisionPartsList(CollisionParts* pParts) {}

/**
 * Disconnects collision parts. Unused.
 * @param pParts collision parts
 */
void CollisionPartsKeeperPtrArray::disconnectToCollisionPartsList(CollisionParts* pParts) {}

/**
 * Updates the collision parts. Unused.
 */
void CollisionPartsKeeperPtrArray::movement() {}
}  // namespace al
