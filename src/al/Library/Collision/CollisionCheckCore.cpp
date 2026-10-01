#include "Library/Collision/CollisionCheckCore.hpp"

#include "Library/Collision/CollisionCheckInfo.hpp"
#include "Library/Collision/ICollisionPartsKeeper.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace {
using namespace al;

inline void searchWithSphereCore(const CollisionPartsList& rList, const SphereCheckInfo& rCheckInfo,
                                 CollisionPartsDelegate& rDelegate) {
    for (auto it = rList.begin(); it != rList.end(); ++it) {
        const CollisionParts* parts = *it;

        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
            continue;
        }

        if (alCollisionUtil::isFarAway(*parts, rCheckInfo.getPos(), rCheckInfo.getRadius())) {
            continue;
        }

        rDelegate.invoke(*it);
    }
}

class SimpleCollisionPartsKeeper : public ICollisionPartsKeeper {
public:
    SimpleCollisionPartsKeeper() = default;

    void endInit() override;
    void addCollisionParts(CollisionParts* pParts) override;
    void connectToCollisionPartsList(CollisionParts* pParts) override;
    void disconnectToCollisionPartsList(CollisionParts* pParts) override;
    s32 checkStrikePoint(HitInfo* pHitInfo,
                         const CollisionCheckInfoBase& rCheckInfo) const override;
    s32 checkStrikeSphere(SphereHitResultBuffer* pBuffer, const SphereCheckInfo& rCheckInfo,
                          bool isCheckNear, const sead::Vector3f& rMoveDir) const override;
    s32 checkStrikeArrow(ArrowHitResultBuffer* pBuffer,
                         const ArrowCheckInfo& rCheckInfo) const override;
    s32 checkStrikeSphereForPlayer(SphereHitResultBuffer* pBuffer,
                                   const SphereCheckInfo& rCheckInfo) const override;
    s32 checkStrikeDisk(DiskHitResultBuffer* pBuffer,
                        const DiskCheckInfo& rCheckInfo) const override;
    void searchWithSphere(const sead::Vector3f& rPos, f32 radius,
                          CollisionPartsDelegate& rDelegate) const override;
    void searchWithSphere(const SphereCheckInfo& rCheckInfo,
                          CollisionPartsDelegate& rDelegate) const override;
    void movement() override;

private:
    CollisionPartsList mPartsList;
};
}  // namespace

namespace al {
/**
 * Checks whether a point is inside any collision parts of a list.
 * @param pHitInfo output hit info
 * @param rList collision parts list
 * @param rCheckInfo check info
 * @return true if a collision parts was hit
 */
bool checkStrikePointCore(HitInfo* pHitInfo, const CollisionPartsList& rList,
                          const CollisionCheckInfoBase& rCheckInfo) {
    for (CollisionParts* parts : rList) {
        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
            continue;
        }

        if (alCollisionUtil::isFarAway(*parts, rCheckInfo.getPos(), 0.0f)) {
            continue;
        }

        if (parts->checkStrikePoint(pHitInfo, rCheckInfo.getPos(),
                                    rCheckInfo.getTriangleFilter())) {
            return true;
        }
    }

    return false;
}

/**
 * Collects the collisions of a sphere with a list of collision parts.
 * @param pBuffer output hit buffer
 * @param rList collision parts list
 * @param rCheckInfo check info
 * @param isCheckNear whether to interpolate the movement of the collision parts
 * @param rMoveDir movement direction of the sphere
 * @return the number of hits
 */
s32 checkStrikeSphereCore(SphereHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                          const SphereCheckInfo& rCheckInfo, bool isCheckNear,
                          const sead::Vector3f& rMoveDir) {
    s32 hitNum = 0;

    for (CollisionParts* parts : rList) {
        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
            continue;
        }

        if (alCollisionUtil::isFarAway(*parts, rCheckInfo.getPos(), rCheckInfo.getRadius())) {
            continue;
        }

        hitNum += parts->checkStrikeSphere(pBuffer, rCheckInfo.getPos(), rCheckInfo.getRadius(),
                                           isCheckNear, rMoveDir,
                                           rCheckInfo.getTriangleFilter());

        if (pBuffer->isFull()) {
            break;
        }
    }

    return hitNum;
}

/**
 * Collects the collisions of an arrow with a list of collision parts.
 * @param pBuffer output hit buffer
 * @param rList collision parts list
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 checkStrikeArrowCore(ArrowHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                         const ArrowCheckInfo& rCheckInfo) {
    s32 hitNum = 0;

    for (CollisionParts* parts : rList) {
        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
            continue;
        }

        sead::Vector3f trans;
        parts->getBaseMtx().getTranslation(trans);
        f32 range = parts->getBoundingSphereRange();

        if (!isNearCollideSphereAabb(trans, range, rCheckInfo.getBoundBox())) {
            continue;
        }

        if (!checkHitSegmentSphere(trans, rCheckInfo.getPos(), rCheckInfo.getEndPos(), range,
                                   nullptr, nullptr)) {
            continue;
        }

        hitNum += parts->checkStrikeArrow(pBuffer, rCheckInfo.getPos(), rCheckInfo.getDir(),
                                          rCheckInfo.getTriangleFilter());

        if (pBuffer->isFull()) {
            break;
        }
    }

    return hitNum;
}

/**
 * Collects the collisions of a player sphere with a list of collision parts.
 * @param pBuffer output hit buffer
 * @param rList collision parts list
 * @param rCheckInfo check info
 * @param isCheckNear unused
 * @param rMoveDir unused
 * @return the number of hits
 */
s32 checkStrikeSphereForPlayerCore(SphereHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                                   const SphereCheckInfo& rCheckInfo, bool isCheckNear,
                                   const sead::Vector3f& rMoveDir) {
    s32 hitNum = 0;

    for (CollisionParts* parts : rList) {
        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
            continue;
        }

        if (alCollisionUtil::isFarAway(*parts, rCheckInfo.getPos(), rCheckInfo.getRadius())) {
            continue;
        }

        hitNum += parts->checkStrikeSphereForPlayer(pBuffer, rCheckInfo.getPos(),
                                                    rCheckInfo.getRadius(),
                                                    rCheckInfo.getTriangleFilter());

        if (pBuffer->isFull()) {
            break;
        }
    }

    return hitNum;
}

/**
 * Collects the collisions of a disk with a list of collision parts.
 * @param pBuffer output hit buffer
 * @param rList collision parts list
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 checkStrikeDiskCore(DiskHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                        const DiskCheckInfo& rCheckInfo) {
    s32 hitNum = 0;

    for (CollisionParts* parts : rList) {
        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
            continue;
        }

        if (alCollisionUtil::isFarAway(*parts, rCheckInfo.getPos(),
                                       rCheckInfo.getBoundingRadius())) {
            continue;
        }

        hitNum += parts->checkStrikeDisk(pBuffer, rCheckInfo.getPos(), rCheckInfo.mRadius,
                                         rCheckInfo.mHeight, *rCheckInfo.mDir,
                                         rCheckInfo.getTriangleFilter());

        if (pBuffer->isFull()) {
            break;
        }
    }

    return hitNum;
}

/**
 * Updates the matrices of valid collision parts.
 * @param pParts collision parts
 */
void updateCollisionParts(CollisionParts* pParts) {
    if (!pParts->isValidCollision()) {
        return;
    }

    pParts->updateMtx();

    if (pParts->isJustValidated()) {
        pParts->resetJustValidated();
    }
}

/**
 * Updates the matrices of every valid collision parts in a list.
 * @param pList collision parts list
 */
void updateCollisionPartsList(CollisionPartsList* pList) {
    for (CollisionParts* parts : *pList) {
        updateCollisionParts(parts);
    }
}

/**
 * Adds collision parts to a list if they are valid.
 * @param pList collision parts list
 * @param pParts collision parts
 */
void pushBackCollisionParts(CollisionPartsList* pList, CollisionParts* pParts) {
    pList->pushBack(pParts->getListNode());
    pParts->onJoinList();

    if (!pParts->isValidCollision()) {
        pList->erase(pParts->getListNode());
    }
}

/**
 * Creates a collision parts keeper that checks every collision parts.
 * @return the keeper
 */
ICollisionPartsKeeper* createSimpleCollisionPartsKeeper() {
    return new SimpleCollisionPartsKeeper;
}

/**
 * Creates the check info of a disk.
 * @param rPos center of the disk
 * @param radius radius of the disk
 * @param height height of the disk
 * @param rDir normal of the disk
 */
DiskCheckInfo::DiskCheckInfo(const sead::Vector3f& rPos, f32 radius, f32 height,
                             const sead::Vector3f& rDir) {
    mPos = &rPos;
    mPartsFilter = nullptr;
    mTriangleFilter = nullptr;
    mRadius = radius;
    mHeight = height;
    mDir = &rDir;
    mBoundingRadius = 0.0f;

    if (radius > height) {
        mBoundingRadius = sead::Mathf::sqrt(2.0f) * radius;
    } else {
        mBoundingRadius = sead::Mathf::sqrt(height * height + radius * radius);
    }
}
}  // namespace al

namespace {
/**
 * Finishes initialization.
 */
void SimpleCollisionPartsKeeper::endInit() {}

/**
 * Adds collision parts.
 * @param pParts collision parts
 */
void SimpleCollisionPartsKeeper::addCollisionParts(CollisionParts* pParts) {
    pushBackCollisionParts(&mPartsList, pParts);
}

/**
 * Connects collision parts.
 * @param pParts collision parts
 */
void SimpleCollisionPartsKeeper::connectToCollisionPartsList(CollisionParts* pParts) {
    pushBackCollisionParts(&mPartsList, pParts);
}

/**
 * Disconnects collision parts.
 * @param pParts collision parts
 */
void SimpleCollisionPartsKeeper::disconnectToCollisionPartsList(CollisionParts* pParts) {
    pParts->getListNode()->erase();
}

/**
 * Checks whether a point is inside any collision parts.
 * @param pHitInfo output hit info
 * @param rCheckInfo check info
 * @return true if a collision parts was hit
 */
s32 SimpleCollisionPartsKeeper::checkStrikePoint(HitInfo* pHitInfo,
                                                 const CollisionCheckInfoBase& rCheckInfo) const {
    return checkStrikePointCore(pHitInfo, mPartsList, rCheckInfo);
}

/**
 * Collects the collisions of a sphere.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @param isCheckNear whether to interpolate the movement of the collision parts
 * @param rMoveDir movement direction of the sphere
 * @return the number of hits
 */
s32 SimpleCollisionPartsKeeper::checkStrikeSphere(SphereHitResultBuffer* pBuffer,
                                                  const SphereCheckInfo& rCheckInfo,
                                                  bool isCheckNear,
                                                  const sead::Vector3f& rMoveDir) const {
    return checkStrikeSphereCore(pBuffer, mPartsList, rCheckInfo, isCheckNear, rMoveDir);
}

/**
 * Collects the collisions of an arrow.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 SimpleCollisionPartsKeeper::checkStrikeArrow(ArrowHitResultBuffer* pBuffer,
                                                 const ArrowCheckInfo& rCheckInfo) const {
    return checkStrikeArrowCore(pBuffer, mPartsList, rCheckInfo);
}

/**
 * Collects the collisions of a player sphere.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 SimpleCollisionPartsKeeper::checkStrikeSphereForPlayer(
    SphereHitResultBuffer* pBuffer, const SphereCheckInfo& rCheckInfo) const {
    return checkStrikeSphereForPlayerCore(pBuffer, mPartsList, rCheckInfo, false,
                                          sead::Vector3f::zero);
}

/**
 * Collects the collisions of a disk.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 SimpleCollisionPartsKeeper::checkStrikeDisk(DiskHitResultBuffer* pBuffer,
                                                const DiskCheckInfo& rCheckInfo) const {
    return checkStrikeDiskCore(pBuffer, mPartsList, rCheckInfo);
}

/**
 * Calls a delegate for every collision parts near a sphere.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rDelegate delegate to call
 */
void SimpleCollisionPartsKeeper::searchWithSphere(const sead::Vector3f& rPos, f32 radius,
                                                  CollisionPartsDelegate& rDelegate) const {
    SphereCheckInfo checkInfo(rPos, radius);
    searchWithSphereCore(mPartsList, checkInfo, rDelegate);
}

/**
 * Calls a delegate for every collision parts near a sphere.
 * @param rCheckInfo check info
 * @param rDelegate delegate to call
 */
void SimpleCollisionPartsKeeper::searchWithSphere(const SphereCheckInfo& rCheckInfo,
                                                  CollisionPartsDelegate& rDelegate) const {
    searchWithSphereCore(mPartsList, rCheckInfo, rDelegate);
}

/**
 * Updates the matrices of every collision parts.
 */
void SimpleCollisionPartsKeeper::movement() {
    updateCollisionPartsList(&mPartsList);
}
}  // namespace
