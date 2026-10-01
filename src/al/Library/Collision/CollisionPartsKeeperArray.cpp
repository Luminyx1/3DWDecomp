#include "Library/Collision/CollisionPartsKeeperArray.hpp"

#include "Library/Collision/CollisionCheckInfo.hpp"
#include <prim/seadMemUtil.h>

#include "Library/Math/MathUtil.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace al {
void updateCollisionParts(CollisionParts* pParts);
}  // namespace al

namespace {
inline bool isNearPartsInfo(const al::CollisionPartsKeeperArray::PartsInfo& rInfo,
                            const sead::Vector3f& rPos, f32 radius) {
    f32 range = rInfo.range + radius;
    f32 dx = rInfo.trans.x - rPos.x;

    if (dx < -range || range < dx) {
        return false;
    }

    f32 dy = rInfo.trans.y - rPos.y;

    if (dy < -range || range < dy) {
        return false;
    }

    f32 dz = rInfo.trans.z - rPos.z;

    if (dz < -range || range < dz) {
        return false;
    }

    return !(dx * dx + dy * dy + dz * dz > range * range);
}

inline void setPartsInfo(al::CollisionPartsKeeperArray::PartsInfo* pInfo,
                         al::CollisionParts* pParts) {
    const sead::Matrix34f& rMtx = pParts->getBaseMtx();
    pInfo->trans.x = rMtx(0, 3);
    pInfo->trans.y = rMtx(1, 3);
    pInfo->trans.z = rMtx(2, 3);
    pInfo->range = pParts->getBoundingSphereRange();
}
}  // namespace

namespace al {
/**
 * Creates a collision parts keeper with room for 12000 collision parts.
 */
CollisionPartsKeeperArray::CollisionPartsKeeperArray() {
    mPartsInfos = new PartsInfo[12000];
    sead::MemUtil::fillZero(mPartsInfos, sizeof(PartsInfo) * 12000);
    mPartsNum = 0;
    mPartsNumMax = 12000;
}

/**
 * Adds collision parts. Unused.
 * @param pParts collision parts
 */
void CollisionPartsKeeperArray::addCollisionParts(CollisionParts* pParts) {}

/**
 * Registers collision parts if they are valid and not registered yet.
 * @param pParts collision parts
 */
void CollisionPartsKeeperArray::connectToCollisionPartsList(CollisionParts* pParts) {
    if (findPartsIndex(pParts) >= 0) {
        return;
    }

    pParts->onJoinList();

    if (!pParts->isValidCollision()) {
        return;
    }

    PartsInfo* info = &mPartsInfos[mPartsNum++];
    info->parts = pParts;
    setPartsInfo(info, pParts);
}

/**
 * Unregisters collision parts.
 * @param pParts collision parts
 */
void CollisionPartsKeeperArray::disconnectToCollisionPartsList(CollisionParts* pParts) {
    s32 index = findPartsIndex(pParts);

    if (index < 0) {
        return;
    }

    s32 last = mPartsNum - 1;

    if (last != index) {
        mPartsInfos[index] = mPartsInfos[last];
        mPartsInfos[mPartsNum - 1].parts = nullptr;
    }

    mPartsNum--;
}

/**
 * Checks whether any collision parts contain a point.
 * @param pHitInfo output hit info
 * @param rCheckInfo check info
 * @return true if a collision parts was hit
 */
s32 CollisionPartsKeeperArray::checkStrikePoint(HitInfo* pHitInfo,
                                                const CollisionCheckInfoBase& rCheckInfo) const {
    CollisionParts* nearParts[256];
    s32 nearNum = 0;
    const PartsInfo* end = mPartsInfos + mPartsNum;

    for (const PartsInfo* info = mPartsInfos; info != end; info++) {
        if (!isNearPartsInfo(*info, rCheckInfo.getPos(), 0.0f)) {
            continue;
        }

        if (nearNum >= 256) {
            break;
        }

        nearParts[nearNum++] = info->parts;
    }

    for (s32 i = 0; i < nearNum; i++) {
        CollisionParts* parts = nearParts[i];

        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
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
 * Collects the collisions of a sphere.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @param isCheckNear whether to check near polygons
 * @param rMoveDir movement direction of the sphere
 * @return the number of hits
 */
s32 CollisionPartsKeeperArray::checkStrikeSphere(SphereHitResultBuffer* pBuffer,
                                                 const SphereCheckInfo& rCheckInfo,
                                                 bool isCheckNear,
                                                 const sead::Vector3f& rMoveDir) const {
    CollisionParts* nearParts[256];
    s32 nearNum = 0;
    const PartsInfo* end = mPartsInfos + mPartsNum;

    for (const PartsInfo* info = mPartsInfos; info != end; info++) {
        if (!isNearPartsInfo(*info, rCheckInfo.getPos(), rCheckInfo.getRadius())) {
            continue;
        }

        if (nearNum >= 256) {
            break;
        }

        nearParts[nearNum++] = info->parts;
    }

    s32 hitNum = 0;

    for (s32 i = 0; i < nearNum; i++) {
        CollisionParts* parts = nearParts[i];

        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
            continue;
        }

        hitNum += parts->checkStrikeSphere(pBuffer, rCheckInfo.getPos(), rCheckInfo.getRadius(),
                                           isCheckNear, rMoveDir, rCheckInfo.getTriangleFilter());

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
s32 CollisionPartsKeeperArray::checkStrikeArrow(ArrowHitResultBuffer* pBuffer,
                                                const ArrowCheckInfo& rCheckInfo) const {
    CollisionParts* nearParts[256];
    s32 nearNum = 0;
    const PartsInfo* end = mPartsInfos + mPartsNum;

    for (const PartsInfo* info = mPartsInfos; info != end; info++) {
        f32 range = info->range;
        const sead::BoundBox3f& rBox = rCheckInfo.getBoundBox();

        if (info->trans.x < rBox.getMin().x - range || range + rBox.getMax().x < info->trans.x) {
            continue;
        }

        if (info->trans.y < rBox.getMin().y - range || range + rBox.getMax().y < info->trans.y) {
            continue;
        }

        if (info->trans.z < rBox.getMin().z - range || range + rBox.getMax().z < info->trans.z) {
            continue;
        }

        if (!checkHitSegmentSphere(info->trans, rCheckInfo.getPos(), rCheckInfo.getEndPos(), range,
                                   nullptr, nullptr)) {
            continue;
        }

        if (nearNum >= 256) {
            break;
        }

        nearParts[nearNum++] = info->parts;
    }

    s32 hitNum = 0;

    for (s32 i = 0; i < nearNum; i++) {
        CollisionParts* parts = nearParts[i];

        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
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
 * Collects the collisions of a player sphere.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 CollisionPartsKeeperArray::checkStrikeSphereForPlayer(SphereHitResultBuffer* pBuffer,
                                                          const SphereCheckInfo& rCheckInfo) const {
    CollisionParts* nearParts[256];
    s32 nearNum = 0;
    const PartsInfo* end = mPartsInfos + mPartsNum;

    for (const PartsInfo* info = mPartsInfos; info != end; info++) {
        if (!isNearPartsInfo(*info, rCheckInfo.getPos(), rCheckInfo.getRadius())) {
            continue;
        }

        if (nearNum >= 256) {
            break;
        }

        nearParts[nearNum++] = info->parts;
    }

    s32 hitNum = 0;

    for (s32 i = 0; i < nearNum; i++) {
        CollisionParts* parts = nearParts[i];

        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
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
 * Collects the collisions of a disk. Unsupported.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 CollisionPartsKeeperArray::checkStrikeDisk(DiskHitResultBuffer* pBuffer,
                                               const DiskCheckInfo& rCheckInfo) const {
    return 0;
}

/**
 * Calls a delegate for every collision parts near a sphere.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rDelegate delegate to call
 */
void CollisionPartsKeeperArray::searchWithSphere(const sead::Vector3f& rPos, f32 radius,
                                                 CollisionPartsDelegate& rDelegate) const {
    SphereCheckInfo checkInfo(rPos, radius);
    CollisionParts* nearParts[256];
    s32 nearNum = 0;
    const PartsInfo* end = mPartsInfos + mPartsNum;

    for (const PartsInfo* info = mPartsInfos; info != end; info++) {
        if (!isNearPartsInfo(*info, rPos, radius)) {
            continue;
        }

        if (nearNum >= 256) {
            break;
        }

        nearParts[nearNum++] = info->parts;
    }

    for (s32 i = 0; i < nearNum; i++) {
        CollisionParts* parts = nearParts[i];

        if (alCollisionUtil::isInvalidParts(*parts, checkInfo)) {
            continue;
        }

        rDelegate.invoke(parts);
    }
}

/**
 * Calls a delegate for every collision parts near a sphere.
 * @param rCheckInfo check info
 * @param rDelegate delegate to call
 */
void CollisionPartsKeeperArray::searchWithSphere(const SphereCheckInfo& rCheckInfo,
                                                 CollisionPartsDelegate& rDelegate) const {
    CollisionParts* nearParts[400];
    s32 nearNum = 0;
    const PartsInfo* end = mPartsInfos + mPartsNum;

    for (const PartsInfo* info = mPartsInfos; info != end; info++) {
        if (!isNearPartsInfo(*info, rCheckInfo.getPos(), rCheckInfo.getRadius())) {
            continue;
        }

        if (nearNum >= 400) {
            break;
        }

        nearParts[nearNum++] = info->parts;
    }

    for (s32 i = 0; i < nearNum; i++) {
        CollisionParts* parts = nearParts[i];

        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
            continue;
        }

        rDelegate.invoke(parts);
    }
}

/**
 * Updates every collision parts and their cached bounding spheres.
 */
void CollisionPartsKeeperArray::movement() {
    PartsInfo* end = mPartsInfos + mPartsNum;

    for (PartsInfo* info = mPartsInfos; info != end; info++) {
        updateCollisionParts(info->parts);
        setPartsInfo(info, info->parts);
    }
}

/**
 * Finishes initialization.
 */
void CollisionPartsKeeperArray::endInit() {}
}  // namespace al
