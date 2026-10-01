#include "Library/Collision/CollisionMultiSphereBase.hpp"

#include <prim/seadDelegate.h>

#include "Library/Collision/ICollisionPartsKeeper.hpp"
#include "Library/Collision/KCollisionFunc.hpp"
#include "Library/Collision/KCollisionServer.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace al {
/**
 * Collects the polygons hit by the spheres.
 * @return true if any polygon was hit
 */
bool CollisionMultiSphereBase::check() {
    getResultRingBuffer()->clear();
    mUpdatedHitNum = 0;

    if (mSpheres == nullptr) {
        return false;
    }

    if (mSphereNum < 2) {
        return false;
    }

    const Sphere* sphere = mSpheres;

    if (mBaseMtx != nullptr) {
        for (s32 i = 0; i < mSphereNum; i++) {
            mSphereWorks[i].mPos.setMul(*mBaseMtx, sphere->mPos);
            sphere = getNextSphere(sphere);
        }
    } else if (mTrans != nullptr) {
        for (s32 i = 0; i < mSphereNum; i++) {
            mSphereWorks[i].mPos.setAdd(sphere->mPos, *mTrans);
            sphere = getNextSphere(sphere);
        }
    } else {
        for (s32 i = 0; i < mSphereNum; i++) {
            mSphereWorks[i].mPos.set(sphere->mPos);
            sphere = getNextSphere(sphere);
        }
    }

    sead::Delegate1<CollisionMultiSphereBase, CollisionParts*> delegate(
        this, &CollisionMultiSphereBase::callbackFromParts);
    alCollisionUtil::getCollisionPartsKeeper(mActor)->searchWithSphere(
        mSphereWorks[0].mPos, mSpheres->mRadius, delegate);
    return getResultRingBuffer()->size() > 0;
}

/**
 * Collects the polygons of collision parts hit by the spheres.
 * @param pParts collision parts
 */
void CollisionMultiSphereBase::callbackFromParts(CollisionParts* pParts) {
    for (s32 i = 0; i < mSphereNum; i++) {
        mSphereWorks[i].mLocalPos.setMul(pParts->getBaseInvMtx(), mSphereWorks[i].mPos);
    }

    f32 scale = pParts->mMtxScale;
    mLocalScale = scale;
    mKCollisionServer = pParts->getKCollisionServer();
    sead::Delegate2<CollisionMultiSphereBase, KCPrismData*, const KCPrismHeader*> delegate(
        this, &CollisionMultiSphereBase::callbackFromServer);
    KCFxyz pos;
    pos.x = mSphereWorks[0].mLocalPos.x;
    pos.y = mSphereWorks[0].mLocalPos.y;
    pos.z = mSphereWorks[0].mLocalPos.z;
    mKCollisionServer->searchPrism(&pos, scale * mSpheres->mRadius, delegate);

    for (s32 i = mUpdatedHitNum; i < getResultRingBuffer()->size(); i++) {
        Result& result = (*getResultRingBuffer())[i];
        sead::Vector3f hitPos;
        alKCollisionFunc::calcSphereHitPos(&hitPos, mKCollisionServer,
                                           mSphereWorks[result.mSphereIndex].mLocalPos,
                                           *result.mPrismData, result.mPrismHeader,
                                           static_cast<u8>(result.mHitInfo.mCollisionLocation));
        result.mHitInfo.mPos.setMul(pParts->getBaseMtx(), hitPos);
        result.mHitInfo.mTriangle.fillData(*pParts, result.mPrismData, result.mPrismHeader);
    }

    mUpdatedHitNum = getResultRingBuffer()->size();
}

/**
 * Gets the number of polygons hit by the spheres.
 * @return the number of hits
 */
u32 CollisionMultiSphereBase::getHitNum() const {
    return getResultRingBuffer()->size();
}

/**
 * Gets a polygon hit by the spheres.
 * @param index index of the hit
 * @return the hit info
 */
const HitInfo* CollisionMultiSphereBase::getHitInfo(u32 index) const {
    return &(*getResultRingBuffer())[index].mHitInfo;
}

/**
 * Gets the index of the sphere that hit a polygon.
 * @param index index of the hit
 * @return the sphere index
 */
s32 CollisionMultiSphereBase::getHitSphereIndex(u32 index) const {
    return (*getResultRingBuffer())[index].mSphereIndex;
}

/**
 * Checks whether the spheres hit a polygon.
 * @param pData prism data of the polygon
 * @param pHeader prism header of the polygon
 */
void CollisionMultiSphereBase::callbackFromServer(KCPrismData* pData,
                                                  const KCPrismHeader* pHeader) {
    if (isResultFull()) {
        return;
    }

    KCFxyz pos;
    Result result;
    const Sphere* sphere = getNextSphere(mSpheres);

    for (s32 i = 1; i < mSphereNum; sphere = getNextSphere(sphere), i++) {
        pos.x = mSphereWorks[i].mLocalPos.x;
        pos.y = mSphereWorks[i].mLocalPos.y;
        pos.z = mSphereWorks[i].mLocalPos.z;
        f32 dist;
        u8 location;

        if (!mKCollisionServer->KCHitSphereForPlayer(pData, pHeader, &pos,
                                                     sphere->mRadius * mLocalScale, &dist,
                                                     &location)) {
            continue;
        }

        result.mPrismData = pData;
        result.mPrismHeader = pHeader;
        result.mSphereIndex = i;
        result.mHitInfo._70 = dist;
        result.mHitInfo.mCollisionLocation = static_cast<CollisionLocation>(location);
        getResultRingBuffer()->pushBack(result);

        if (isResultFull()) {
            return;
        }
    }
}
}  // namespace al
