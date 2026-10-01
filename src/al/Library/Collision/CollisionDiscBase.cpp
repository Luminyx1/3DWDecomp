#include "Library/Collision/CollisionDiscBase.hpp"

#include <prim/seadDelegate.h>

#include "Library/Collision/ICollisionPartsKeeper.hpp"
#include "Library/Collision/KCollisionServer.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace al {
/**
 * Collects the polygons hit by the disc.
 * @return true if any polygon was hit
 */
bool CollisionDiscBase::check() {
    getResultRingBuffer()->clear();
    sead::Delegate1<CollisionDiscBase, CollisionParts*> delegate(
        this, &CollisionDiscBase::callbackFromParts);
    alCollisionUtil::getCollisionPartsKeeper(mActor)->searchWithSphere(mPos, mRadius, delegate);
    return getResultRingBuffer()->size() > 0;
}

/**
 * Collects the polygons of collision parts hit by the disc.
 * @param pParts collision parts
 */
void CollisionDiscBase::callbackFromParts(CollisionParts* pParts) {
    mLocalPos.setMul(pParts->getBaseInvMtx(), mPos);
    mLocalDir.setRotated(pParts->getBaseInvMtx(), mDir);
    mLocalRadius = mRadius * pParts->mMtxScale;
    mKCollisionServer = pParts->getKCollisionServer();
    sead::Delegate2<CollisionDiscBase, KCPrismData*, const KCPrismHeader*> delegate(
        this, &CollisionDiscBase::callbackFromServer);
    KCFxyz pos;
    pos.set(mLocalPos.x, mLocalPos.y, mLocalPos.z);
    mKCollisionServer->searchPrism(&pos, mLocalRadius, delegate);

    for (s32 i = mUpdatedHitNum; i < getResultRingBuffer()->size(); i++) {
        Result& result = (*getResultRingBuffer())[i];
        result.mHitInfo.mHitPos.setRotated(pParts->getBaseMtx(), result.mHitInfo.mHitPos);
        result.mHitInfo.mTriangle.fillData(*pParts, result.mPrismData, result.mPrismHeader);
    }

    mUpdatedHitNum = getResultRingBuffer()->size();
}

/**
 * Gets the number of polygons hit by the disc.
 * @return the number of hits
 */
u32 CollisionDiscBase::getHitNum() const {
    return getResultRingBuffer()->size();
}

/**
 * Gets a polygon hit by the disc.
 * @param index index of the hit
 * @return the hit info
 */
const CollisionDiscBase::DiscHitInfo* CollisionDiscBase::getHitInfo(u32 index) const {
    return &(*getResultRingBuffer())[index].mHitInfo;
}

/**
 * Checks whether the disc hits a polygon.
 * @param pData prism data of the polygon
 * @param pHeader prism header of the polygon
 */
void CollisionDiscBase::callbackFromServer(KCPrismData* pData, const KCPrismHeader* pHeader) {
    if (isResultFull()) {
        return;
    }

    Result result;

    if (!mKCollisionServer->KCHitDisc(pData, pHeader, mLocalPos, mLocalDir, mLocalRadius,
                                      mHeight, &result.mHitInfo.mHitPos,
                                      &result.mHitInfo.mDistance)) {
        return;
    }

    result.mPrismData = pData;
    result.mPrismHeader = pHeader;
    getResultRingBuffer()->pushBack(result);
}
}  // namespace al
