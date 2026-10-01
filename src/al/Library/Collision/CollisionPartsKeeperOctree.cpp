#include "Library/Collision/CollisionPartsKeeperOctree.hpp"

#include "Library/Collision/CollisionCheckInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace al {
s32 checkStrikePointCore(HitInfo* pHitInfo, const CollisionPartsList& rList,
                         const CollisionCheckInfoBase& rCheckInfo);
u32 checkStrikeSphereCore(SphereHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                          const SphereCheckInfo& rCheckInfo, bool isCheckNear,
                          const sead::Vector3f& rMoveDir);
s32 checkStrikeArrowCore(ArrowHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                         const ArrowCheckInfo& rCheckInfo);
u32 checkStrikeSphereForPlayerCore(SphereHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                                   const SphereCheckInfo& rCheckInfo, bool isCheckNear,
                                   const sead::Vector3f& rMoveDir);
s32 checkStrikeDiskCore(DiskHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                        const DiskCheckInfo& rCheckInfo);
void updateCollisionParts(CollisionParts* pParts);
void pushBackCollisionParts(CollisionPartsList* pList, CollisionParts* pParts);
}  // namespace al

namespace {
inline void getPartsTrans(sead::Vector3f* pTrans, const al::CollisionParts* pParts) {
    const sead::Matrix34f& rMtx = pParts->getBaseMtx();
    pTrans->x = rMtx(0, 3);
    pTrans->y = rMtx(1, 3);
    pTrans->z = rMtx(2, 3);
}

inline bool isOverlapRange(f32 minA, f32 maxA, f32 minB, f32 maxB) {
    return minA <= maxB && minB <= maxA;
}

inline bool isOverlapBox(const sead::BoundBox3f& rBoxA, const sead::BoundBox3f& rBoxB) {
    return isOverlapRange(rBoxA.getMin().x, rBoxA.getMax().x, rBoxB.getMin().x,
                          rBoxB.getMax().x) &&
           isOverlapRange(rBoxA.getMin().y, rBoxA.getMax().y, rBoxB.getMin().y,
                          rBoxB.getMax().y) &&
           isOverlapRange(rBoxA.getMin().z, rBoxA.getMax().z, rBoxB.getMin().z, rBoxB.getMax().z);
}
}  // namespace

namespace al {
/**
 * Creates an octree collision parts keeper.
 * @param nodeNumMax maximum number of octree nodes
 * @param depthMax maximum depth of the octree
 */
CollisionPartsKeeperOctree::CollisionPartsKeeperOctree(s32 nodeNumMax, s32 depthMax)
    : mDepthMax(depthMax) {
    mNodeList.allocBuffer(nodeNumMax, nullptr);
}

/**
 * Builds the octree from the bounds of every added collision parts.
 */
void CollisionPartsKeeperOctree::endInit() {
    f32 size = sead::Mathf::max(mPartsBox.getSizeX(),
                                sead::Mathf::max(mPartsBox.getSizeY(), mPartsBox.getSizeZ()));
    sead::Vector3f center;
    mPartsBox.getCenter(&center);
    mCubeSize = size;
    f32 halfSize = size * 0.5f;
    mRootNode.depth = 0;
    mRootNode.box.set(center - sead::Vector3f(halfSize, halfSize, halfSize),
                      center + sead::Vector3f(halfSize, halfSize, halfSize));
    insertMovingPartsListToOctNode();
    mIsEndInit = true;
}

/**
 * Moves the collision parts of the moving list into the octree.
 */
void CollisionPartsKeeperOctree::insertMovingPartsListToOctNode() {
    for (auto it = mPartsList.robustBegin(); it != mPartsList.robustEnd(); ++it) {
        CollisionParts* parts = it->mData;

        if (parts->isValidCollision()) {
            insertLooseOctree(&mRootNode, parts);
        } else {
            parts->mListNode.erase();
        }
    }
}

/**
 * Adds collision parts and grows the bounds of the octree.
 * @param pParts collision parts
 */
void CollisionPartsKeeperOctree::addCollisionParts(CollisionParts* pParts) {
    sead::Vector3f trans;
    getPartsTrans(&trans, pParts);
    f32 range = pParts->getBoundingSphereRange() * 2.0f * 0.5f;

    sead::Vector3f halfSize(range, range, range);
    sead::Vector3f min = trans - halfSize;
    sead::Vector3f max = trans + halfSize;

    if (getPartsMin().x > min.x) {
        getPartsMin().x = min.x;
    }

    if (getPartsMin().y > min.y) {
        getPartsMin().y = min.y;
    }

    if (getPartsMin().z > min.z) {
        getPartsMin().z = min.z;
    }

    if (getPartsMax().x < max.x) {
        getPartsMax().x = max.x;
    }

    if (getPartsMax().y < max.y) {
        getPartsMax().y = max.y;
    }

    if (getPartsMax().z < max.z) {
        getPartsMax().z = max.z;
    }

    pushBackCollisionParts(&mPartsList, pParts);
}

/**
 * Inserts collision parts into the octree once initialization has finished.
 * @param pParts collision parts
 */
void CollisionPartsKeeperOctree::connectToCollisionPartsList(CollisionParts* pParts) {
    if (mIsEndInit) {
        insertLooseOctree(&mRootNode, pParts);
    }
}

/**
 * Inserts collision parts into the deepest loose octree node that contains them.
 * @param pNode node to start from
 * @param pParts collision parts
 * @return the node the collision parts were added to
 */
CollisionPartsKeeperOctree::OctNode*
CollisionPartsKeeperOctree::insertLooseOctree(OctNode* pNode, CollisionParts* pParts) {
    while (pNode->depth + 1 < mDepthMax) {
        f32 childHalfSize = calcNodeCubeSizeHalf(pNode->depth + 1);
        sead::Vector3f center;
        pNode->box.getCenter(&center);
        sead::Vector3f trans;
        getPartsTrans(&trans, pParts);

        bool isLowerX = trans.x <= center.x;
        bool isLowerY = trans.y <= center.y;
        bool isLowerZ = trans.z <= center.z;
        s32 index = (isLowerX ? 0 : 1) | (isLowerY ? 0 : 2) | (isLowerZ ? 0 : 4);
        OctNode** childSlot = &pNode->children[index];
        OctNode* child = *childSlot;
        sead::Vector3f childCenter;

        if (child != nullptr) {
            child->box.getCenter(&childCenter);
        } else {
            f32 offset = mCubeSize / (2 << pNode->depth);
            childCenter.x = center.x + (isLowerX ? offset * -0.5f : offset * 0.5f);
            childCenter.y = center.y + (isLowerY ? offset * -0.5f : offset * 0.5f);
            childCenter.z = center.z + (isLowerZ ? offset * -0.5f : offset * 0.5f);
        }

        f32 looseHalfSize = childHalfSize * 2.0f * 0.5f;
        sead::Vector3f looseHalf(looseHalfSize, looseHalfSize, looseHalfSize);
        sead::Vector3f boxMin = childCenter - looseHalf;
        sead::Vector3f boxMax = looseHalf + childCenter;
        sead::BoundBox3f box(boxMin, boxMax);

        if (!isSphereFitsInBox(trans, pParts->getBoundingSphereRange(), box)) {
            break;
        }

        if (child == nullptr) {
            child = mNodeList.emplaceBack();

            if (child == nullptr) {
                break;
            }

            child->parent = pNode;
            child->box.set(boxMin, boxMax);
            child->depth = pNode->depth + 1;
            *childSlot = child;
        }

        pNode = child;
    }

    pushBackCollisionParts(&pNode->partsList, pParts);
    return pNode;
}

/**
 * Removes collision parts from their octree node.
 * @param pParts collision parts
 */
void CollisionPartsKeeperOctree::disconnectToCollisionPartsList(CollisionParts* pParts) {
    if (mIsEndInit) {
        pParts->mListNode.erase();
    }
}

/**
 * Checks whether the collision parts of the root node contain a point.
 * @param pHitInfo output hit info
 * @param rCheckInfo check info
 * @return true if a collision parts was hit
 */
s32 CollisionPartsKeeperOctree::checkStrikePoint(HitInfo* pHitInfo,
                                                 const CollisionCheckInfoBase& rCheckInfo) const {
    return checkStrikePointCore(pHitInfo, mRootNode.partsList, rCheckInfo);
}

/**
 * Collects the collisions of a sphere.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @param isCheckNear whether to check near polygons
 * @param rMoveDir movement direction of the sphere
 * @return the number of hits
 */
s32 CollisionPartsKeeperOctree::checkStrikeSphere(SphereHitResultBuffer* pBuffer,
                                                  const SphereCheckInfo& rCheckInfo,
                                                  bool isCheckNear,
                                                  const sead::Vector3f& rMoveDir) const {
    return checkStrikeSphereRecursive(pBuffer, rCheckInfo, &mRootNode, isCheckNear, rMoveDir,
                                      checkStrikeSphereCore);
}

/**
 * Collects the collisions of a sphere in a node and its children.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @param pNode octree node
 * @param isCheckNear whether to check near polygons
 * @param rMoveDir movement direction of the sphere
 * @param checkFunc function checking a collision parts list
 * @return the number of hits
 */
s32 CollisionPartsKeeperOctree::checkStrikeSphereRecursive(
    SphereHitResultBuffer* pBuffer, const SphereCheckInfo& rCheckInfo, const OctNode* pNode,
    bool isCheckNear, const sead::Vector3f& rMoveDir, SphereCheckFunc checkFunc) const {
    s32 hitNum = checkFunc(pBuffer, pNode->partsList, rCheckInfo, isCheckNear, rMoveDir);

    for (s32 i = 0; i < 8; i++) {
        const OctNode* child = pNode->children[i];

        if (child != nullptr &&
            isNearCollideSphereAabb(rCheckInfo.getPos(), rCheckInfo.getRadius(), child->box)) {
            hitNum += checkStrikeSphereRecursive(pBuffer, rCheckInfo, child, isCheckNear,
                                                 rMoveDir, checkFunc);
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
s32 CollisionPartsKeeperOctree::checkStrikeSphereForPlayer(SphereHitResultBuffer* pBuffer,
                                                           const SphereCheckInfo& rCheckInfo) const {
    return checkStrikeSphereRecursive(pBuffer, rCheckInfo, &mRootNode, false,
                                      sead::Vector3f::zero, checkStrikeSphereForPlayerCore);
}

/**
 * Collects the collisions of a disk.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 CollisionPartsKeeperOctree::checkStrikeDisk(DiskHitResultBuffer* pBuffer,
                                                const DiskCheckInfo& rCheckInfo) const {
    return checkStrikeDiskRecursive(pBuffer, rCheckInfo, &mRootNode);
}

/**
 * Collects the collisions of a disk in a node and its children.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @param pNode octree node
 * @return the number of hits
 */
s32 CollisionPartsKeeperOctree::checkStrikeDiskRecursive(DiskHitResultBuffer* pBuffer,
                                                         const DiskCheckInfo& rCheckInfo,
                                                         const OctNode* pNode) const {
    s32 hitNum = checkStrikeDiskCore(pBuffer, pNode->partsList, rCheckInfo);

    for (s32 i = 0; i < 8; i++) {
        const OctNode* child = pNode->children[i];

        if (child != nullptr && isNearCollideSphereAabb(rCheckInfo.getPos(),
                                                        rCheckInfo.getBoundingRadius(),
                                                        child->box)) {
            hitNum += checkStrikeDiskRecursive(pBuffer, rCheckInfo, child);
        }
    }

    return hitNum;
}

/**
 * Calls a delegate for every collision parts near a sphere.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rDelegate delegate to call
 */
void CollisionPartsKeeperOctree::searchWithSphere(const sead::Vector3f& rPos, f32 radius,
                                                  CollisionPartsDelegate& rDelegate) const {
    searchWithSphereRecursive(rPos, radius, rDelegate, &mRootNode);
}

/**
 * Calls a delegate for every collision parts near a sphere in a node and its children.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rDelegate delegate to call
 * @param pNode octree node
 */
void CollisionPartsKeeperOctree::searchWithSphereRecursive(const sead::Vector3f& rPos, f32 radius,
                                                           CollisionPartsDelegate& rDelegate,
                                                           const OctNode* pNode) const {
    auto it = pNode->partsList.begin();
    SphereCheckInfo checkInfo(rPos, radius);

    for (; it != pNode->partsList.end(); ++it) {
        CollisionParts* parts = *it;

        if (alCollisionUtil::isInvalidParts(*parts, checkInfo)) {
            continue;
        }

        if (alCollisionUtil::isFarAway(*parts, checkInfo.getPos(), checkInfo.getRadius())) {
            continue;
        }

        rDelegate.invoke(*it);
    }

    for (s32 i = 0; i < 8; i++) {
        const OctNode* child = pNode->children[i];

        if (child != nullptr && isNearCollideSphereAabb(rPos, radius, child->box)) {
            searchWithSphereRecursive(rPos, radius, rDelegate, child);
        }
    }
}

/**
 * Calls a delegate for every collision parts near a sphere.
 * @param rCheckInfo check info
 * @param rDelegate delegate to call
 */
void CollisionPartsKeeperOctree::searchWithSphere(const SphereCheckInfo& rCheckInfo,
                                                  CollisionPartsDelegate& rDelegate) const {
    searchWithSphereRecursive(rCheckInfo, rDelegate, &mRootNode);
}

/**
 * Calls a delegate for every collision parts near a sphere in a node and its children.
 * @param rCheckInfo check info
 * @param rDelegate delegate to call
 * @param pNode octree node
 */
void CollisionPartsKeeperOctree::searchWithSphereRecursive(const SphereCheckInfo& rCheckInfo,
                                                           CollisionPartsDelegate& rDelegate,
                                                           const OctNode* pNode) const {
    for (auto it = pNode->partsList.begin(); it != pNode->partsList.end(); ++it) {
        CollisionParts* parts = *it;

        if (alCollisionUtil::isInvalidParts(*parts, rCheckInfo)) {
            continue;
        }

        if (alCollisionUtil::isFarAway(*parts, rCheckInfo.getPos(), rCheckInfo.getRadius())) {
            continue;
        }

        rDelegate.invoke(*it);
    }

    for (s32 i = 0; i < 8; i++) {
        const OctNode* child = pNode->children[i];

        if (child != nullptr &&
            isNearCollideSphereAabb(rCheckInfo.getPos(), rCheckInfo.getRadius(), child->box)) {
            searchWithSphereRecursive(rCheckInfo, rDelegate, child);
        }
    }
}

/**
 * Collects the collisions of an arrow.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @return the number of hits
 */
s32 CollisionPartsKeeperOctree::checkStrikeArrow(ArrowHitResultBuffer* pBuffer,
                                                 const ArrowCheckInfo& rCheckInfo) const {
    return checkStrikeArrowRecursive(pBuffer, rCheckInfo, &mRootNode);
}

/**
 * Collects the collisions of an arrow in a node and its children.
 * @param pBuffer output hit buffer
 * @param rCheckInfo check info
 * @param pNode octree node
 * @return the number of hits
 */
s32 CollisionPartsKeeperOctree::checkStrikeArrowRecursive(ArrowHitResultBuffer* pBuffer,
                                                          const ArrowCheckInfo& rCheckInfo,
                                                          const OctNode* pNode) const {
    s32 hitNum = checkStrikeArrowCore(pBuffer, pNode->partsList, rCheckInfo);

    for (s32 i = 0; i < 8; i++) {
        const OctNode* child = pNode->children[i];

        if (child != nullptr && isOverlapBox(rCheckInfo.getBoundBox(), child->box)) {
            hitNum += checkStrikeArrowRecursive(pBuffer, rCheckInfo, child);
        }
    }

    return hitNum;
}

/**
 * Updates every collision parts and reinserts the ones that left their node.
 */
void CollisionPartsKeeperOctree::movement() {
    updateOctNodeCollisionPartsRecursive(&mRootNode);
    insertMovingPartsListToOctNode();
}

/**
 * Updates the collision parts of a node and its children.
 * @param pNode octree node
 */
void CollisionPartsKeeperOctree::updateOctNodeCollisionPartsRecursive(OctNode* pNode) {
    for (auto it = pNode->partsList.robustBegin(); it != pNode->partsList.robustEnd(); ++it) {
        CollisionParts* parts = it->mData;
        updateCollisionParts(parts);

        if (parts->_154 == 0 && !isFitsInBox(parts, pNode->box)) {
            pushBackCollisionParts(&mPartsList, parts);
        }
    }

    for (s32 i = 0; i < 8; i++) {
        if (pNode->children[i] != nullptr) {
            updateOctNodeCollisionPartsRecursive(pNode->children[i]);
        }
    }
}

/**
 * Checks whether the bounding sphere of collision parts fits in a box.
 * @param pParts collision parts
 * @param rBox box
 * @return true if the bounding sphere fits
 */
bool CollisionPartsKeeperOctree::isFitsInBox(const CollisionParts* pParts,
                                             const sead::BoundBox3f& rBox) const {
    f32 range = pParts->getBoundingSphereRange();
    sead::Vector3f trans;
    getPartsTrans(&trans, pParts);

    if (trans.x - range < rBox.getMin().x) {
        return false;
    }

    if (rBox.getMax().x < range + trans.x) {
        return false;
    }

    if (trans.y - range < rBox.getMin().y) {
        return false;
    }

    if (rBox.getMax().y < range + trans.y) {
        return false;
    }

    if (trans.z - range < rBox.getMin().z) {
        return false;
    }

    if (rBox.getMax().z < range + trans.z) {
        return false;
    }

    return true;
}

/**
 * Calculates half the cube size of a node at a depth.
 * @param depth node depth
 * @return half the cube size
 */
f32 CollisionPartsKeeperOctree::calcNodeCubeSizeHalf(s32 depth) const {
    return mCubeSize * 2.0f / (2 << depth);
}

/**
 * Checks whether a sphere fits in a box.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rBox box
 * @return true if the sphere fits
 */
bool CollisionPartsKeeperOctree::isSphereFitsInBox(const sead::Vector3f& rPos, f32 radius,
                                                   const sead::BoundBox3f& rBox) const {
    if (rPos.x - radius < rBox.getMin().x) {
        return false;
    }

    if (rBox.getMax().x < rPos.x + radius) {
        return false;
    }

    if (rPos.y - radius < rBox.getMin().y) {
        return false;
    }

    if (rBox.getMax().y < rPos.y + radius) {
        return false;
    }

    if (rPos.z - radius < rBox.getMin().z) {
        return false;
    }

    if (rBox.getMax().z < rPos.z + radius) {
        return false;
    }

    return true;
}
}  // namespace al
